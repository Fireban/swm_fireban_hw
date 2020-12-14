# TIC-thermal-imaging-camera

SDK2.9

Linux


Install required packages

sudo apt-get update               (ensures that apt-get pulls the latest packages)

sudo apt-get install libusb-1.0   (installs libusb drivers)

sudo apt-get install libusb-dev   (installs libusb development libraries)

sudo apt-get install libsdl2-dev   (installs SDL2 development libraries)


Execute the installation script
For ARM v7 or greater targets:
./install-arm-linux-gnueabihf-2.9.sh

For i686 targets:
./install-i686-linux-gnu-2.9.sh

For x86_64 targets:
./install-x86_64-linux-gnu-2.9.sh

For High-Silicon Cortex-A7 targets:
./install-arm-hisiv300-linux-uclibcgnueabi-2.9.sh

For Rockchip RV1108 targets:
./install-arm-rkcvr-linux-uclibcgnueabihf-2.9.sh


NOTE: If the install script does not run due to lack of permissions, type ‘sudo chmod +x <script>’.
  
This will set the execute bits on the script file.

Build the sample apps

In each sample directory, type:

“make clean”, then “make”
