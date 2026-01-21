cmake_minimum_required(VERSION 3.16)
project(WatchDog C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_executable(watchdogd daemon/watchdogd.c)
add_executable(watchdogctl ctl/watchdogctl.cpp)

#install(TARGETS watchdogd DESTINATION /usr/sbin)
#install(TARGETS watchdogctl DESTINATION /usr/bin)

install(TARGETS watchdogd
        RUNTIME DESTINATION bin)

install(TARGETS watchdogctl
        RUNTIME DESTINATION bin)

install(FILES assets/Judge.txt
        DESTINATION share/watchdog)

#Manual for now
#add_custom_target(install_watchdog
#COMMAND sudo systemctl stop watchdog.service
#COMMAND sudo cp watchdogd /usr/local/bin/watchdogd
#COMMAND sudo systemctl start watchdog.service
#)

