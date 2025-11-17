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

## drawing to the matrix

Every instance of `fw_led_matrix::LedMatrix` has an internal matrix where you can make changes using
`fw_led_matrix::LedMatrix::blit()`.
There are two ways of drawing the internal matrix to the actual matrix:

* `fw_led_matrix::LedMatrix::draw_matrix_black_white()` will draw the internal matrix to the actual matrix
    it interprets the values in the internal matrix as booleans (0 = OFF, 1 to 255 = ON)
* `fw_led_matrix::LedMatrix::draw_matrix_greyscale()` will draw the internal matrix to the actual matrix
    it interprets the values in the internal matrix as brightness values

### coordinates

the top-left corner is (0, 0), the bottom-right corner is (8, 33)

when using `fw_led_matrix::LedMatrix::blit()` the top left corner of the "image" is used as the anchor
so if you where trying to blit:
```
01010 | ░█░█░ 
01010 | ░█░█░
00000 | ░░░░░
10001 | █░░░█
01110 | ░███░
```

to the matrix at (2, 1)

the resulting matrix (assuming it was empty before) will look like this

```
000000000 | ░░░░░░░░░
000101000 | ░░░█░█░░░
000101000 | ░░░█░█░░░
000000000 | ░░░░░░░░░
001000100 | ░░█░░░█░░
000111000 | ░░░███░░░
000000000 | ░░░░░░░░░
000000000 | ░░░░░░░░░
000000000 | ░░░░░░░░░
... + 14 more empty rows ...
000000000 | ░░░░░░░░░
```

### using blit

`fw_led_matrix:LedMatrix::blit()` can be used to blit a 2D "image" to the internal matrix

`fw_led_matrix::LedMatrix::blit()` takes 3 arguments:
1. `std::vector<std::vector<uint8_t>> data` this is the data to blit in column-major order meaning `data` is a
    `std::vector`of columns, each column is a `std::vector` of bytes which are the values to blit
2. `unsigned int x` the x coordinate of the position to blit to, accepted values: 0 to (8 - the amount of columns),
    if x is greater than (8 - amount of columns) `blit` will return `fw_led_matrix::X_OUT_OF_BOUNDS` 
3. `unsigned int y` the y coordinate of the position to blit to, accepted values:
    0 to (33 - the height of the highest column),
    if x is greater than (33 - height of highest column) `blit` will return `fw_led_matrix::Y_OUT_OF_BOUNDS`

### using set pixel

`fw_led_matrix::LedMatrix::set_pixel` can be used to set a single pixel value on the internal matrix

`fw_led_matrix::LedMatrix::set_pixel` takes 3 arguments
1. `uint8_t value` the value to to write to the pixel
2. `unsigned int x` the x coordinate of the position to blit to, accepted values: 0 to 8,
   if x is greater than 8 `blit` will return `fw_led_matrix::X_OUT_OF_BOUNDS`
3. `unsigned int y` the y coordinate of the position to blit to, accepted values: 0 to 33,
   if x is greater than 33 `blit` will return `fw_led_matrix::Y_OUT_OF_BOUNDS

## Raw communication with the matrix
based on [https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md](https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md)

ho to communicate with the matrix using `fw_led_matrix::LedMatrix::send_command()`

### Commands
these commands are defined in the enum fw_led_matrix::Command

if a command appears twice in this table it means it has multiple variants that differ in parameters and/or response 

| Command      |  id  | Parameters | Response | Description                                                                       | Shorthand (Member function of `fw_led_matrix::LedMatrix`)     |
|--------------|:----:|:----------:|:--------:|-----------------------------------------------------------------------------------|---------------------------------------------------------------|
| BRIGHTNESS   | 0x00 |   1 byte   |    -     | Sets the brightness                                                               | `set_brightness()`                                            |
| BRIGHTNESS   | 0x00 |     -      |  1 byte  | Gets the brightness                                                               | `get_brightness()`                                            |
| PATTERN      | 0x01 |  1 byte*   |    -     | Displays a pre-programmed pattern                                                 | `display_pattern()`                                           |
| BOOTLOADER   | 0x02 |     -      |    -     | Jumps to the bootloader**                                                         | -                                                             |
| SLEEP        | 0x03 |   1 byte   |    -     | Sets if the matrix is asleep                                                      | `set_sleep()`                                                 |
| SLEEP        | 0x03 |     -      |  1 byte  | Gets if the matrix is asleep                                                      | `get_sleep()`                                                 |
| ANIMATE      | 0x04 |   1 byte   |    -     | Sets if the current pattern should scroll                                         | `set_animate()`                                               |
| ANIMATE      | 0x04 |     -      |  1 byte  | Gets if the current pattern is animating                                          | `get_animate()`                                               |
| PANIC        | 0x05 |     -      |    -     | Causes the Firmware to crash**                                                    | -                                                             |
| DRAWBW       | 0x06 |  39 bytes  |    -     | Draw a black & white image                                                        | `blit()` + `draw_matrix_black_white()`                        |
| STAGE_COL    | 0x07 |  35 bytes  |    -     | Sends a column of brightnesses                                                    | `blit()` + `draw_matrix_greyscale()`                          |
| COMMIT_COL   | 0x08 |     -      |    -     | Draw all columns sent by STAGE_COL                                                | `blit()` + `draw_matrix_greyscale()`                          |
| START_GAME   | 0x09 | 1 byte***  |    -     | Start a pre-programmed game on the matrix****                                     | `game_start()`                                                |
| GAME_CONTROL | 0x11 |   1 byte   |    -     | Control the currently running game                                                | `game_control()` / `game_quit()` (for quitting the game only) |
| GAME_STATUS  | 0x12 |     -      |   ???    | I'm not quite sure what this does and don't know enough rust to read the firmware | -                                                             |
| VERSION      | 0x20 |     -      | 3 bytes  | Gets the version info of the matrix                                               | `get_version()`                                               |

+ \*: The pattern `PERCENTAGE` is special it needs an extra parameter.
+ \*\*: The LED matrix will stop responding after this.
+ \*\*\*: The game `GAME_OF_LIFE` is special it needs an extra parameter.
+ \*\*\*\*: The LED matrix will not respond normally to commands until the game is quit.

### Parameter types

parameter types can be converted to bytes for usage with `fw_led_matrix::LedMatrix::send_command()` with `LedMatrix::enum_to_value()`
