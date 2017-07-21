cmake_minimum_required( VERSION 3.2 FATAL_ERROR )
project( mbp-pidcool VERSION 0.1 LANGUAGES CXX )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${mbp-pidcool_SOURCE_DIR}/../bin/Debug )
set( CMAKE_MODULE_PATH ${mbp-pidcool_DIR}/cmake/modules )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

#find_package( X11 REQUIRED )
#find_package( Cairo REQUIRED )

add_subdirectory( ./mbp-pidcoold )
