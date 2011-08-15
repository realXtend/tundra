#!/bin/bash
cd /opt/realxtend-tundra
export LD_LIBRARY_PATH=.:./modules/core
./server "$@"
