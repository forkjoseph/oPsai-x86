#!/bin/bash

su
mount -o rw,remount /system
cp /sdcard/opsaivncserver /system/lib/
cp /system/lib/opsaivncserver /system/lib/opsaivncserver.so
chmod 777 /system/lib/opsaivncserver.so
mount -o ro,remount /system