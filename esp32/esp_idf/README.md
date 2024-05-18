```sh
. $HOME/esp/esp-idf/export.sh
sudo chmod a+rw /dev/ttyUSB0
```

### Configure the Project

```sh
idf.py menuconfig
```

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```sh
idf.py -p /dev/ttyUSB0 flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.
