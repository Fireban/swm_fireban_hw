#!/bin/bash
sudo apt-get update
echo Y | sudo apt-get upgrade
echo Y | sudo apt-get install libusb-1.0 libusb-dev libsdl2-dev
echo Y | sudo apt-get install libusb-1.0-0-dev libusb-1.0-0
echo Y | sudo apt-get install v4l2loopback-utils
echo Y | sudo apt-get install raspberrypi-kernel-headers
echo Y | sudo apt-get install libcurl4-openssl-dev

sudo cp ./thermal-imaging-camera/example/10-seekthermal.rules /etc/udev/rules.d/
sudo udevadm control --reload

sudo cp ./thermal-imaging-camera/example/lib/libseekware.so* /usr/lib
sudo mkdir /usr/include/seekware
sudo cp ./thermal-imaging-camera/example/include/seekware/seekware.h /usr/include/seekware

sudo cp ./thermal-imaging-camera/example/src/seekware-filter/loop_exe.sh /home/pi

curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | sudo bash
sudo apt install git-lfs
#cd fireban-hw && git lfs pull



