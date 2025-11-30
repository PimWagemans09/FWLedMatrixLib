# A simple C++ library for the framework 16 LED matrix input module

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
    fwlm::LedMatrix led_matrix("COM3");
    
    // set to half brightness
    led_matrix.set_brightness(128);
    
    // display the zig zag pattern
    led_matrix.display_pattern(fwlm::Pattern::ZIG_ZAG);
}
```

## General commands

All of the functions below are methods of `fwlm::LedMatrix`

* `set_brightness(uint8t value)` - sets the brightness of the matrix to `value`
* `get_brightness(uint8t *brightness_out)` - gets the brightness and store it in `brightness_out`
* `display_pattern(fwlm::Pattern pattern)` - displays a pre-programmed pattern on the matrix
* `set_sleep(bool sleep)` - sets if the matrix is asleep or not to `sleep`
* `get_sleep(bool *sleep_out)` - gets is the matrix is asleep or not and stores it in `sleep_out`
* `set_animate(bool animate)` - sets if the matrix is animating (scrolling what's on the matrix) or not to `animate`
* `get_animate(bool *animate_out)` - gets if the matrix is animating and stores it in `animate_out`
* `get_version(fwlm::Version *version_out)` - gets the firmware version of the matrix
    and stores it in `version_out`

## Drawing to the matrix

Every instance of `fwlm::LedMatrix` has an internal matrix where you can make changes using
`fwlm::LedMatrix::blit()`.
There are two ways of drawing the internal matrix to the actual matrix:

* `fwlm::LedMatrix::draw_matrix_black_white()` will draw the internal matrix to the actual matrix
    it interprets the values in the internal matrix as booleans (0 = OFF, 1 to 255 = ON)
* `fwlm::LedMatrix::draw_matrix_greyscale()` will draw the internal matrix to the actual matrix
    it interprets the values in the internal matrix as brightness values

### coordinates

the top-left corner is (0, 0), the bottom-right corner is (8, 33)

when using `fwlm::LedMatrix::blit()` the top left corner of the "image" is used as the anchor
so if you where trying to blit:
```
01010 | ░█░█░ 
01010 | ░█░█░
00000 | ░░░░░
10001 | █░░░█
01110 | ░███░
```

to the internal matrix at (2, 1)

the resulting internal matrix (assuming it was empty before) will look like this

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

### Using `blit`

`fwlm:LedMatrix::blit()` can be used to blit a 2D "image" to the internal matrix

`blit` takes 3 arguments:
1. `std::vector<std::vector<uint8_t>> data` this is the data to blit in column-major order meaning `data` is a
    `std::vector`of columns, each column is a `std::vector` of bytes which are the values to blit
2. `unsigned int x` the x coordinate of the position to blit to, accepted values: 0 to (8 - the amount of columns),
    if x is greater than (8 - amount of columns) `blit` will return `fwlm::X_OUT_OF_BOUNDS` 
3. `unsigned int y` the y coordinate of the position to blit to, accepted values:
    0 to (33 - the height of the highest column),
    if x is greater than (33 - height of highest column) `blit` will return `fwlm::Y_OUT_OF_BOUNDS`

### Using `set_pixel`

`fwlm::LedMatrix::set_pixel()` can be used to set a single pixel value on the internal matrix

`set_pixel` takes 3 arguments
1. `uint8_t value` the value to write to the pixel
2. `unsigned int x` the x coordinate of the position to blit to, accepted values: 0 to 8,
   if x is greater than 8 `blit` will return `fwlm::X_OUT_OF_BOUNDS`
3. `unsigned int y` the y coordinate of the position to blit to, accepted values: 0 to 33,
   if x is greater than 33 `blit` will return `fwlm::Y_OUT_OF_BOUNDS

## starting, playing, and quitting games

When playing a game most other commands will stop working correctly.

Unfortunately I can't get Tetris to work so it won't be explained here.

### starting games

To start a game use `fwlm::LedMatrix::game_start()`, it's usage depends on what game you're starting.

#### Starting Conway's Game of Life

To start Conway's Game of Life you need to pass two parameters to `game_start` like so:

```c++
led_matrix.game_start(fwlm::GameID::GAME_OF_LIFE, fwlm::GameOfLifeStartParam::<PATTERN_NAME>);
```

Just replace `<PATTERN_NAME>` with any of the following:

* `CURRENT_MATRIX`
* `PATTERN_1` 
* `BLINKER`
* `TOAD`
* `BEACON`
* `GLIDER`
* `BLINKER_TOAD_BEACON`

#### Starting Snake and Pong

You can start Snake and Pong like so:

```c++
led_matrix.game_start(fwlm::GameID::<GAME_NAME>);
```

Just replace `<GAME_NAME>` with any of the following:

* `SNAKE`
* `PONG`

### Controlling a game

Conway's game of life doesn't have any controls.

Games can be controlled using `fwlm::LedMatrix::game_control()`.

What param to pass into `game_control` depends on the game that's being played.

#### Controlling Snake

* `fwlm::GameControl::UP` - the snake's head will move up
* `fwlm::GameControl::DOWN` - the snake's head will move down
* `fwlm::GameControl::LEFT` - the snake's head will move left
* `fwlm::GameControl::RIGHT` - the snake's head will move right

#### Controlling Pong

* `fwlm::GameControl::LEFT` - Player 1's paddle will move left.
* `fwlm::GameControl::RIGHT` - Player 1's paddle will move right.
* `fwlm::GameControl::LEFT2` - Player 2's paddle will move left.
* `fwlm::GameControl::RIGHT2` - Player 2's paddle will move right.

### Quitting a game

Any game can be quit in 2 ways:

1. using `fwlm::LedMatrix::game_quit()`
2. using `led_matrix.game_control(fwlm::GameControl::QUIT);`

## Errors

almost all functions in this library return an error code where:
`0` is success and non-`0` (including negatives) is failure.

To get an error message from the code use `fwlm::error_to_string(int error)`.

The error message will be prefixed with the source of the error, the prefix will be:

* `fwlm:` if the source is this library
* `windows_getlasterror:` if the source is the Windows api
* `linux_errno:` if the source is the linux "api" (functions like `open`, `read`, and `write`)

## Raw communication with the matrix
Based on [this document](https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md).

How to communicate with the matrix using `fwlm::LedMatrix::send_command()`.

### Commands
These commands are defined in the enum `fwlm::Command`.

If a command appears twice in this table it means it has multiple variants that differ in parameters and/or response 

| Command      |  id  | Parameters | Response | Description                                                                       | Shorthand (Member function of `fwlm::LedMatrix`)              |
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
| START_GAME   | 0x10 | 1 byte***  |    -     | Start a pre-programmed game on the matrix****                                     | `game_start()`                                                |
| GAME_CONTROL | 0x11 |   1 byte   |    -     | Control the currently running game                                                | `game_control()` / `game_quit()` (for quitting the game only) |
| GAME_STATUS  | 0x12 |     -      |   ???    | I'm not quite sure what this does and don't know enough rust to read the firmware | -                                                             |
| VERSION      | 0x20 |     -      | 3 bytes  | Gets the version info of the matrix                                               | `get_version()`                                               |

+ \*: The pattern `PERCENTAGE` is special it needs an extra parameter.
+ \*\*: The LED matrix will stop responding after this.
+ \*\*\*: The game `GAME_OF_LIFE` is special it needs an extra parameter.
+ \*\*\*\*: The LED matrix will not respond normally to commands until the game is quit.

### Getting a response
NOTE: the response is always 32 bytes long even if the table above says it only responds with 1 byte, the rest of the bytes will just be 0x00.

```c++
#include "FWLedMatrixLib/fw_led_matrix.h"

void main(){
    // replace "COM3" with the path to your device
    // so something like COM<number> on Windows
    // and something like /dev/ttyACM0 on Linux
    fwlm::LedMatrix led_matrix("COM3")
    
    // to tell send_command that you expect a response call it with `with_response = true`
    int r = led_matrix.send_command(fwlm::Command::BRIGHTNESS, {}, true);
    
    // get what the last response was, may be empty if the command failed or no commands have been sent
    const std::vector<uint8_t> buffer = led_matrix.get_last_response();
    
    // print the response (not necessary)
    for (const unsigned char i : buffer) {
        printf("%02x ", i);
    }
    printf("\n");
}
```
