#!/bin/bash

 while [ 1 ]
     do
         Cnt1=`ps -aux|grep "seekware-sdl"|grep -v grep|wc -l`
         PROCESS1=`ps -aux|grep "seekware-sdl"|grep -v grep|awk '{print $1}'`
     if [ $Cnt1 -ne 0 ]
         then
             echo "seekware-sdl (PID : $PROCESS1) alredy runing"
     else
         ./seekware-sdl &
         echo "seekware-sdl run"
     fi
     sleep 3
 done

