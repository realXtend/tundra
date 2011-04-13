#!/bin/bash

# load arg script to server and connect with viewer

bindir=$(dirname $(readlink -f $0))/../bin
testfile=`readlink -f $1`
cd $bindir

echo 'var d = frame.DelayedExecute(15.0); d.Triggered.connect(function (time) { print("exiting!"); framework.Exit(); });' > exitdelay.js
echo 'var d = frame.DelayedExecute(0.5); d.Triggered.connect(function (time) { client.Login("localhost",2345,"foo"," ","tcp");});' > dologin.js

case $testfile in 
    *.js)
	echo "Setting up args for a js file ($testfile)"
	cat exitdelay.js $testfile > s.js
	servercmd="./server --headless --run s.js"
	viewercmd="./viewer --run v.js"
	;;
    *.txml)
	echo "Setting up args for a txml file ($testfile)"
	servercmd="./server --file $testfile --run exitdelay.js"
	viewercmd="./viewer --storage `dirname $testfile`/ --run v.js"
	;;
esac

ulimit -t 60 # cpu time limit
ulimit -c unlimited # i'd like core dumps with that
cat exitdelay.js dologin.js > v.js
xterm -e bash -c "$servercmd"' 2>&1 | tee s.out; echo $? > exitstatus.s' &
xterm -e bash -c "$viewercmd"' 2>&1 | tee v.out; echo $? > exitstatus.v' 
wait
grep . exitstatus.?
grep Error: [sv].out
test -f core && exit 1
if test `cat exitstatus.s` = 0 && test `cat exitstatus.v` = 0; then
    echo 'test outcome: success'
    exit 0
else
    echo 'test outcome: failure'
    exit 1
fi

