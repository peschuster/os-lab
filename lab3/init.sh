#!/bin/sh

sudo insmod fifo.ko

major=$(awk "\$2 == \"fifo\" {print \$1}" /proc/devices)

sudo mknod /dev/fifo0 c $major 0
sudo mknod /dev/fifo1 c $major 1
sudo mknod /dev/fifo2 c $major 2
sudo mknod /dev/fifo3 c $major 3

sudo chmod a+rwx /dev/fifo0
sudo chmod a+rwx /dev/fifo1
sudo chmod a+rwx /dev/fifo2
sudo chmod a+rwx /dev/fifo3
