#!/usr/bin/env bash

set -euo pipefail

if [ -f /etc/os-release ] && grep -qi '^ID=ubuntu' /etc/os-release; then
	if [ -f /etc/apt/sources.list ]; then
		sudo sed -i 's|http://security.ubuntu.com/ubuntu|http://archive.ubuntu.com/ubuntu|g' /etc/apt/sources.list
	fi
	if [ -f /etc/apt/sources.list.d/ubuntu.sources ]; then
		sudo sed -i 's|http://security.ubuntu.com/ubuntu|http://archive.ubuntu.com/ubuntu|g' /etc/apt/sources.list.d/ubuntu.sources
	fi
fi

sudo DEBIAN_FRONTEND=noninteractive apt-get -o Acquire::Retries=5 update
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y --fix-missing \
	cmake libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libdrm-dev libgbm-dev \
	ttf-mscorefonts-installer fontconfig libsystemd-dev libinput-dev libudev-dev libxkbcommon-dev
mkdir build && cd build
cmake ..
make -j`nproc`
