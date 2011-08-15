#!/bin/bash
cd /opt/realxtend-naali
export LD_LIBRARY_PATH=.:./modules/core
./viewer "$@"
