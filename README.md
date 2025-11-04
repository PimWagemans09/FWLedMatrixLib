# A simple C++ library for the framework 16 led matrix inputmodule
Supports Windows and Linux.
Tested on Windows 11 and Linux Mint.

## Simple example
```c++
#include "FWLedMatrixLib/fw_led_matrix.h"

void main(){
    // replace "COM3" with the path to your device
    // so something like COM<number> on Windows
    // and something like /dev/ttyACM0 on Linux
    fw_led_matrix::LedMatrix led_matrix("COM3")
    
    // set to half brightness
    led_matrix.set_brightness(128);
    
    // display the zig zag pattern
    led_matrix.send_command(fw_led_matrix::Command::PATTERN, {fw_led_matrix::params.pattern.ZIG_ZAG});
}
```