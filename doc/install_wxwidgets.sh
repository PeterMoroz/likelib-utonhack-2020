#!/bin/bash

if apt-get update > /dev/null; then
  SUDO_PREF=""
else
  SUDO_PREF=sudo
fi

sudo apt-get install libgtk-3-dev build-essential checkinstall

DIR_PATH=$PWD

${SUDO_PREF} mkdir /opt/wx-widgets && ${SUDO_PREF} chown $UID /opt/wx-widgets
cd /opt/wx-widgets

wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.3/wxWidgets-3.1.3.tar.bz2
tar xvjf wxWidgets-3.1.3.tar.bz2

cd wxWidgets-3.1.3
./configure --prefix=/usr/local --disable-shared --enable-unicode 
make
${SUDO_PREF} make install

cd $DIR_PATH
rm -rf /opt/wx-widgets
