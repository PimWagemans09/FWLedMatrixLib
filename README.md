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

## Getting a response
```c++
#include "FWLedMatrixLib/fw_led_matrix.h"

void main(){
    // replace "COM3" with the path to your device
    // so something like COM<number> on Windows
    // and something like /dev/ttyACM0 on Linux
    fw_led_matrix::LedMatrix led_matrix("COM3")
    
    // to tell send_command that you expect a response call it with `with_response = true`
    int r = led_matrix.send_command(fw_led_matrix::Command::VERSION, {}, true);
    
    // get what the last response was, may be empty if the command failed or no commands have been sent
    const std::vector<uint8_t> buffer = led_matrix.get_last_response();
    
    // print the response (not necessary)
    for (const unsigned char i : buffer) {
        printf("%02x ", i);
    }
    printf("\n");
}
```

### Commands that have a response
based on [https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md](https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md)
* BRIGHTNESS - when called with no parameters it responds with the brightness
* SLEEP - when called with no parameters it responds with a bool that indicates if it is sleeping
* ANIMATE - when called with no parameters it responds with a bool that indicates if it is animating
* GAME_STATUS - I don't know how this responds.
* VERSION - responds with the version ( see above link for format )