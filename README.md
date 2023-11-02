Contributors:
- Kurucsai András (WWEI3B)
- Kardos Martin (FRAV6D)

# Task description
In this task we implement a software using Qt, linux kernel module and socket to control a LED matrix circuit using Maxim Integrated MAX7219 chips.
The circuit consists of 4 8x8 matrices controlled by 4 cascaded integrated circuits, communicating via SPI interface.

The software will have the following functions:
* A graphical interface implemented in Qt, with switchable, customizable functions.
* Socket communication between two linux systems.
* Frame by frame animation through the graphical interface.
* Drawing in the graphical interface, inverting drawings.
* Saving drawings to file.
* Display microphone on.
* Configurable process display (e.g. is microsoft teams running, is discord running, etc.)
# Implemented program
Compared to our original task description, only the configurable process display was not implemented.

Start server:
* compile the kernel module with make
* "dtc spidev_disabler.dts -O dtb >spidev_disabler.dtbo" to compile the dts file needed to use SPI bus 0
* load the translated dts with "sudo dtoverlay -d . spidev_disabler
* load the kernel module with "sudo insmod max7219x4.ko
* compile server using gcc e.g. gcc server.c -o server
* start the server with ./server {portnumber}

On the client side, the connection is made by ip address, the connection is indicated by the gui in the bottom left corner.

**Show video URL:**
https://youtu.be/Xmn5PWXf6pk

# Experiences

It is not worth starting the implementation without proper study of the documentation. Mastering QT's own type system required a lot of re-reading.
In QT, generating hardware-related bytes was easier than imagined, but implementing some features required a more thorough study of the documentation (e.g. microphone polling).
When the kernel module was first loaded, it was found that SPI bus 0 was already occupied by another module on the system, so a solution had to be found which was time consuming.

We gained experience in building and debugging Qt applications, kernel modules (e.g. dmesg), interfacing a real physical hardware and designing a user-friendly graphical interface for it.
