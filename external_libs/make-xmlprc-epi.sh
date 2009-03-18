#!/bin/bash

EXPATDIR=$(pwd)'/expat'
XMLRPCDIR=$(pwd)'/xmlrpc-epi'

pushd ${EXPATDIR}
chmod u+x configure
./configure --prefix=
make
make install DESTDIR=${EXPATDIR}
popd

pushd ${XMLRPCDIR}
chmod u+x configure
./configure --with-expat=${EXPATDIR}
make
popd
