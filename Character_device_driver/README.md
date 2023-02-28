# Lunix character device driver

This character device driver takes raw data from a base station that collects measurements (battery, light, temperature) from wireless sensors.
The goal is to format the raw data into human readable from and present each measurement as a special file in the /dev directory.

## To deploy:

1. Compile the code: `make`
2. Load the Lunix kernel module: `insmod lunix.ko`
3. Create the USB TTYs at which the base station will be connexted as well as the measurements devices: `bash mk-lunix-devs.sh`
4. Attach the driver to the input TTY. (on one of the 4 `ttyS_`s): `./lunix-attach /dev/$TTY`
5. Verify that measurements are being received by using `cat` on the first lunix device: `cat /dev/lunix0-temp`
