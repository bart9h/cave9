#!/bin/sh /etc/rc.common

START=99
STOP=40

start()
{
	mkdir -p /tmp/c9
	cd /tmp/c9 || { echo cd /tmp/c9 failed; exit 1; }
	rm -f in out
	mkfifo in out
	touch hi
	
	while true
	do
		head -q -n 1 in hi | sort -nr | head -1 | tee hi > out
	done &
	
	
	while true
	do 
		(netcat -u -l -p 31559 > in < out) &
		sleep 1
		killall netcat
	done &
}

stop()
{
	killall cave9
}

restart()
{
	stop
	start
}
