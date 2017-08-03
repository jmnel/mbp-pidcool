#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>

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

#define DAEMON_NAME "mbp-pidcoold"

const double rpmMinLeft = 2000.0;
const double rpmMinRight = 2000.0;
const double rpmMaxLeft = 7000.0;
const double rpmMaxRight = 7000.0;

const double kP = 0.01;
const double kI = 0.001;
const double kD = 0.004;

const double tStep = 1.0;

const double tempGoal = 60.0;

// -- entry point --
int main(int argc, char *argv[]) {
    setlogmask(LOG_UPTO(LOG_NOTICE));
    openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID,
            LOG_USER);
    cout << "gid = " << getuid << endl;
    if (getuid != 0) {
        cout << "mbp-pidcoold not started with root privileges. ";
        cout << "mpd-pidcoold as root. Exiting." << endl;
        // return EXIT_FAILURE;
    }

    pid_t pid, sid;
    pid = fork();
    if (pid < 0) {
        cout << "Error: Daemon failed to fork. Exiting." << endl;
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    double integ = 0.0;

    double tempOld = getCpuTemp();

    double eOld = 0.0;

    double tempAvg[10] = {tempOld};

    runCmd("tee /sys/devices/platform/applesmc.768/fan1_manual <<< 1");
    runCmd("tee /sys/devices/platform/applesmc.768/fan2_manual <<< 1");

    bool limit = false;
    while (true) {
        double tDelta = tStep;

        assert(tDelta > 0.0);
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
        double e = temp - tempGoal;


        if (!limit) {
            integ = integ + tDelta * e;
        }



        auto deriv = (e - eOld) / tDelta;

        auto u = kP * e + kI * integ + kD * deriv;

        limit = (u <= 0.0 || u >= 1.0);

        // cout << "int=" << integ << endl;
        // cout << "u_pre=" << u << endl;
        u = clamp(u, 0.0, 1.0);

        // cout << "t=" << temp << "°C" << endl;

        double l = u * (rpmMaxLeft - rpmMinLeft) + rpmMinLeft;
        double r = u * (rpmMaxRight - rpmMinRight) + rpmMinRight;

        int left = (int)(l + 0.5);
        int right = (int)(r + 0.5);

        setRpm(left, right);

        tempOld = temp;
        eOld = e;

        timespec sleepTime;
        sleepTime.tv_sec = tStep;
        nanosleep(&sleepTime, nullptr);
    }

    syslog(LOG_NOTICE, "Exiting daemon.");
    closelog();

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
