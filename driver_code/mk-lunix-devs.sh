#!/bin/bash

# Ensure the nodes for the first four serial ports are there.
mknod /dev/ttyS0 c 4 64
mknod /dev/ttyS1 c 4 65
mknod /dev/ttyS2 c 4 66
mknod /dev/ttyS3 c 4 67

# Lunix:TNG nodes: 16 sensors, each has 3 nodes.
for sensor in $(seq 0 1 15); do
	mknod /dev/lunix$sensor-batt c 60 $[$sensor * 8 + 0]
	mknod /dev/lunix$sensor-temp c 60 $[$sensor * 8 + 1]
	mknod /dev/lunix$sensor-light c 60 $[$sensor * 8 + 2]
done
