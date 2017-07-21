#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <time.h>
#include <unistd.h>

using std::array;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;

string runCmd(string const &cmd);
double getCpuTemp();
void setRpm(int left, int right);

template <class T>
T clamp(T const &x, T const &low, T const &up) {
    auto v = (x < low ? low : x);
    v = (v > up ? up : v);
    return v;
}

const double rpmMinLeft = 2000.0;
const double rpmMinRight = 2000.0;
const double rpmMaxLeft = 7000.0;
const double rpmMaxRight = 7000.0;

const double kP = 10.0;
const double kI = 10.0;
const double kD = 2.0;

const double tStep = 1.0;

const double tempGoal = 48.0;

// -- entry point --
int main(int argc, char *argv[]) {
    if (getuid != 0) {
        // cout << "mbp-pidcoold not started with root privileges. ";
        // cout << "mpd-pidcoold as root. Exiting." << endl;
        // return EXIT_FAILURE;
    }

    double integ = 0.0;

    double tempOld = getCpuTemp();

    double eOld = 0.0;

    double tempAvg[10] = {0.0};

    runCmd("tee /sys/devices/platform/applesmc.768/fan1_manual <<< 1");
    runCmd("tee /sys/devices/platform/applesmc.768/fan2_manual <<< 1");

    while (true) {
        auto tDelta = tStep;

        double tempSample = getCpuTemp();

        for (size_t i = 1; i < 10; i++) {
            tempAvg[i] = tempAvg[i - 1];
        }
        tempAvg[0] = tempSample;

        double sum = 0.0;
        for (size_t i = 0; i < 10; i++) {
            sum += tempAvg[i];
        }

        auto temp = sum / 10.0;

        // calculate present error value
        auto e = temp - tempGoal;

        integ += tDelta * e;

        auto deriv = (e - eOld) / tDelta;

        auto u = kP * e + kI * integ + kD * deriv;

        cout << "t=" << temp << "°C" << endl;
        cout << "u=" << u << endl;

        double l = clamp(2000.0 + u * 20.0, rpmMinLeft, rpmMaxLeft);
        double r = clamp(2000.0 + u * 20.0, rpmMinRight, rpmMaxRight);

        int left = (int)(l + 0.5);
        int right = (int)(r + 0.5);

        cout << "l=" << l << ", r=" << r << endl;

        setRpm(left, right);

        tempOld = temp;
        eOld = e;

        timespec sleepTime;
        sleepTime.tv_sec = tStep;
        nanosleep(&sleepTime, nullptr);
    }

    // auto temp = getCpuTemp();
    // cout << "temp=" << temp << "°C" << endl;

    return EXIT_SUCCESS;
}

// -- runCmd function --
string runCmd(string const &cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
    }
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

// -- getCpuTemp function --
double getCpuTemp() {
    string cmd = "sensors | grep Package | ";
    cmd += "sed '/.*/s/Package id 0:[ ]*[+-]//' | ";
    cmd += "sed '/.*/s/°C.*//'";
    auto str = runCmd(cmd);
    auto temp = std::strtof(str.c_str(), nullptr);
    return temp;
}

// -- setRpm function --
void setRpm(int left, int right) {
    string cmd = "tee /sys/devices/platform/applesmc.768/fan1_output <<< ";
    cmd += std::to_string(left);
    runCmd(cmd);
    cmd = "tee /sys/devices/platform/applesmc.768/fan2_output <<< ";
    cmd += std::to_string(right);
    runCmd(cmd);
}
