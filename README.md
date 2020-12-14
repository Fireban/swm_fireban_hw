# SWM Fireban
SWM 11th Fireban team project HW part

## Server Setup

```
sudo su
./init_sh.sh

```

## Driver Setup 

```
sudo depmod -a
sudo modprobe v4l2loopback video_nr=4
```

## Hardware Setup
```
기기당 한번만 설정하면 됩니다. 
sudo raspi-config -> 4.Interfacing Options -> P1 Camera Enable
sudo raspi-config -> 4.Interfacing Options -> P6 Serial Enable
stty -F /dev/serial0 9600
```

## Run
```
sudo ./SWM_Fireban-HW/thermal-imaging-camera/example/src/seekware_stream_v2/seekware-sdl
sudo ./SWM_Fireban-HW/project/autorun
```

## Auto Run

자동실행을 위해서 각각의 스크립트 최하단에 아래의 코드를 추가시켜줍니다.

sudo vim etc/profile.d/bash_completion.sh
> sudo modprobe v4l2loopback video_nr=4

sudo vim /etc/xdg/lxsession/LXDE-pi/autostart
> @lxterminal -e ./SWM_Fireban-HW/thermal-imaging-camera/example/src/seekware_stream_v2/loop_exe.sh
> @lxterminal -e ./SWM_Fireban-HW/project/autorun

