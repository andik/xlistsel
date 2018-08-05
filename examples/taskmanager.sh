#!/bin/ksh

while true
do
	PID=$(
		ps -o pid,ppid,nice,time,comm | 
		sed -E 's/ +/|/g' | 
		xlistsel -s '|' -L -b Kill -b Nice -b Exit -o 0
	)
	case $? in
		0 | 3) exit;;
		1) kill $PID;;
		2) renice 10 $PID;;
	esac
done
