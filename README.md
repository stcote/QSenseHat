# QSenseHat
Qt Interface to the SenseHat board for the Raspberry Pi

The SenseHat board https://www.raspberrypi.org/products/sense-hat/ contains a variety of hardware to augment a Raspberry Pi. These include:
- An 8x8 LED matrix with 16 bit color.
- A miniature 5-button joystick.
- A suite of sensors
  - Gyroscope
  - Accelerometer
  - Magnetometer
  - Temperature
  - Barometric pressure
  - Humidity

There is a python library available, but C++ was left out in the cold. The QSenseHat library hides the ugly details needed to utilize the SenseHat board and makes all functionality available in an object oriented interface.

For access to the sensor data, the excellent RTIMULib library is needed - https://github.com/richards-tech/RTIMULib

The easiest way to get started is to install the sense-hat package.
'sudo apt-get install sense-hat'
or more detailed instructions here: https://www.raspberrypi.org/documentation/hardware/sense-hat/
This will install the RTIMULib library and header files.

As this is a Qt library, Qt needs to be installed. Instructions can be found at: <br>
http://famedsolutions.com/index.php/2015/05/02/installing-qt5-on-debian-jessie/<br>
These instructions are valid for Jessie, and I suggest you should be running it also.

To build and use the QSenseHat library, retrieve the source code, and on a command line in that directory, enter the following commands:<br>
>qmake<br>
>make<br>
>sudo make install<br>

The library (libQSenseHat.so.*) will be installed in /usr/lib and the include files at /usr/include

Using Qt has definite benefits in utilizing the SenseHat. Signals are emitted for joystick use as well as the sensor readings. And the library allows a QImage to be copied to the LED matrix. The importance of this is that a QPainter can be used to draw anything into the QImage. This can then be copied to the matrix. A method is supplied to create a QImage of the required type and size. This was used to implement scrolling text for the display. The QImage can also be much larger, and an x and y offset can allow the LED display to display a small part of a much larger image.

An example application should be added shortly


