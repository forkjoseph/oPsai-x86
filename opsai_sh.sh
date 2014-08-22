#!/bin/bash

shopt -s expand_aliases
alias ndk-build="$HOME/Development/android-ndk-r9d/./ndk-build"

ndk-build -j256
adb push libs/x86/opsaivncserver /sdcard/
adb push libs/x86/opsaitcpserver /sdcard/
adb push libs/x86/opsaizmqserver /sdcard/

