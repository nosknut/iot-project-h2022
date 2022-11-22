# Based on the following PlatformIO espressif32 example code
https://github.com/platformio/platform-espressif32/tree/develop/examples/espidf-aws-iot

Uncheck ```Terminal › Integrated: Windows Enable Conpty``` if menuconfig is not responding to arrowkeys

## Navigation issues with menuconfig
https://github.com/platformio/platform-espressif32/issues/423

How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-espressif32/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell

# Change directory to example
$ cd platform-espressif32/examples/espidf-aws-iot

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Clean build files
$ pio run --target clean
```