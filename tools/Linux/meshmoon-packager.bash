#!/bin/bash

set -e

START_DIR=$(dirname $(readlink -f $0))
$START_DIR/../../src/admino-plugins/tools/Linux/Ubuntu/meshmoon-packager.bash $@
