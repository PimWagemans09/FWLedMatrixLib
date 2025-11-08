# A simple C++ library for the framework 16 LED matrix inputmodule

This library has functions to easily send commands to a LED matrix, get the response, and definitions for all parameter types.


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
NOTE: the response is always 32 bytes long even if the table below says it only responds with 1 byte, the rest ov the bytes will just be 0x00.

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

## Functions to control the matrix



## Raw communication with the matrix
based on [https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md](https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md)


### Commands
these commands are defined in the enum fw_led_matrix::Command

if a command appears twice in this table it means it has multiple variants that differ in parameters and/or response 

| Command      |  id  | Parameters | Response | Description                                                                       | Shorthand (Member function of `fw_led_matrix::LedMatrix`)     |
|--------------|:----:|:----------:|:--------:|-----------------------------------------------------------------------------------|---------------------------------------------------------------|
| BRIGHTNESS   | 0x00 |   1 byte   |    -     | Sets the brightness                                                               | `set_brightness()`                                            |
| BRIGHTNESS   | 0x00 |     -      |  1 byte  | Gets the brightness                                                               | -                                                             |
| PATTERN      | 0x01 |  1 byte*   |    -     | Displays a pre-programmed pattern                                                 | -                                                             |
| BOOTLOADER   | 0x02 |     -      |    -     | Jumps to the bootloader**                                                         | -                                                             |
| SLEEP        | 0x03 |   1 byte   |    -     | Sets if the matrix is asleep                                                      | -                                                             |
| SLEEP        | 0x03 |     -      |  1 byte  | Gets if the matrix is asleep                                                      | -                                                             |
| ANIMATE      | 0x04 |   1 byte   |    -     | Sets if the current pattern should scroll                                         | -                                                             |
| ANIMATE      | 0x04 |     -      |  1 byte  | Gets if the current pattern is animating                                          | -                                                             |
| PANIC        | 0x05 |     -      |    -     | Causes the Firmware to crash**                                                    | -                                                             |
| DRAWBW       | 0x06 |  39 bytes  |    -     | Draw a black & white image                                                        | `blit()` + `draw_matrix_black_white()`                        |
| STAGE_COL    | 0x07 |  35 bytes  |    -     | Sends a column of brightnesses                                                    | `blit()` + `draw_matrix_greyscale()`                          |
| COMMIT_COL   | 0x08 |     -      |    -     | Draw all columns sent by STAGE_COL                                                | `blit()` + `draw_matrix_greyscale()`                          |
| START_GAME   | 0x09 | 1 byte***  |    -     | Start a pre-programmed game on the matrix****                                     | `game_start()`                                                |
| GAME_CONTROL | 0x11 |   1 byte   |    -     | Control the currently running game                                                | `game_control()` / `game_quit()` (for quitting the game only) |
| GAME_STATUS  | 0x12 |     -      |   ???    | I'm not quite sure what this does and don't know enough rust to read the firmware | -                                                             |
| VERSION      | 0x20 |     -      | 3 bytes  | Gets the version info of the matrix                                               | -                                                             |

+ *: The pattern `PERCENTAGE` is special it needs an extra parameter.
+ **: The LED matrix will stop responding after this.
+ ***: The game `GAME_OF_LIFE` is special it needs an extra parameter.
+ ***: The LED matrix will not respond normally to commands until the game is quit.