#!/bin/bash

 while [ 1 ]
     do
         Cnt1=`ps -aux|grep "seekware-sdl"|grep -v grep|wc -l`
         Cnt2=`ps -aux|grep "autorun"|grep -v grep|wc -l`
         PROCESS1=`ps -aux|grep "seekware-sdl"|grep -v grep|awk '{print $1}'`
         PROCESS2=`ps -aux|grep "autorun"|grep -v grep|awk '{print $1}'`
     if [ $Cnt1 -ne 0 ]
         then
             echo "seekware-sdl (PID : $PROCESS1) already runing"
     else
         ./seekware-sdl &
         echo "seekware-sdl run"
     fi

     if [ $Cnt2 -ne 0 ]
         then
             echo "autorun (PID : $PROCESS2) already runing"
     else
	 ./autorun &
	 echo "autorun run"
     fi
     sleep 3
 done

