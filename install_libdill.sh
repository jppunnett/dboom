#!/bin/sh
set -ex
wget http://libdill.org/libdill-2.14.tar.gz
tar -xzf libdill-2.14.tar.gz
cd libdill-2.14 && ./configure --prefix=/usr && make && sudo make install
