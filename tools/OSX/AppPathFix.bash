#!/bin/bash

curdir=$(pwd)
cd ../../build/Tundra.app
bundledir=$(pwd)
cd $curdir
./MacAppFix $bundledir
