#!/usr/bin/sh

cd /root/HELICS
git pull

cd /root/HELICS/build_matlab
make -j2 mfile_overwrite

cd /root/HELICS/build_interface
make -j2 pyfile_overwrite
make -j2 javafile_overwrite

cd /root/HELICS