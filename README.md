WatchDog is a lightweight Blue Team utility to plant canary files within a chose directory. These are designed to look valuable and like they have useful information. When these are altered (opened, edited, deleted) a notification alerts the user.

## Disclaimer

WatchDog is an educational Blue Team / CTF tool.
It has no affiliation with any brands or other creations.  
It intentionally prioritizes simplicity and visibility over stealth or completeness and requires much more development.

## Requirements

Linux system with `systemd`
CMake
A C++ compiler (gcc / clang)
Root privileges (for installation and service control)

## Downloading
git clone https://github.com/Byt3R3ad3er/WatchDog.git
cd watchdog
mkdir build
cd build
cmake ..
make

## After downloading repo
sudo cp build/watchdogd /usr/local/bin/
sudo cp build/watchdogctl /usr/local/bin/
sudo cp systemd/watchdog.service /etc/systemd/system/

sudo systemctl daemon-reload


## Commands after downloading the repo and creating a build folder

sudo ./watchdogctl

After setting configuration with watchdogctl:
sudo systemctl start watchdog.service

To stop the utility:
sudo systemctl stop watchdog.service

To restart the utility after making further configuration changes:
sudo systemctl restart watchdog.service

To see logs in regards to the utility:
journalctl -u watchdog.service -n 5 --no-pager
