#!/bin/bash
CurrentDir=$PWD
apt update
apt upgrade
mkdir ~/OCR
cd ~/OCR
apt install git
apt install cmake
apt install tesseract-ocr
apt install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
apt install libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev

git clone https://github.com/opencv/opencv_contrib.git 
git clone https://github.com/opencv/opencv.git
cd opencv
mkdir release
cd release
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local -DINSTALL_C_EXAMPLES=ON -DINSTALL_PYTHON_EXAMPLES=ON -DOPENCV_EXTRA_MODULES_PATH=~/OCR/opencv_contrib/modules -DBUILD_EXAMPLES=ON ..
make
sudo make install
cd CurrentDir
mkdir build
cd build
cmake ../
cd ../
cp build/ocr .
./run.sh
