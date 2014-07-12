#!/bin/bash

su
cp /sdcard/opsaizmqserver /data/
chmod 777 /data/opsaizmqserver
/data/./opsaizmqserver &
