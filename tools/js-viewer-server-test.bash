#!/bin/bash

# load arg script to server and connect with viewer

bindir=$(dirname $(readlink -f $0))/../bin
jsfile=`readlink -f $1`
cd $bindir
ulimit -t 60 # cpu time limit
ulimit -c unlimited # i'd like core dumps with that
echo 'var d = frame.DelayedExecute(5.0); d.Triggered.connect(function (time) { print("exiting!"); framework.Exit(); });' > exitdelay.js
echo 'var d = frame.DelayedExecute(0.5); d.Triggered.connect(function (time) { client.Login("localhost",2345,"foo"," ","tcp");});' > dologin.js
cat $jsfile exitdelay.js > s.js
cat dologin.js exitdelay.js > v.js
xterm -e bash -c './server --headless --run s.js 2>&1 | tee s.out; echo $? > exitstatus.s' &
xterm -e bash -c './viewer --run v.js 2>&1 | tee v.out; echo $? > exitstatus.v' 
wait
grep . exitstatus.?
test -f core && exit 1
