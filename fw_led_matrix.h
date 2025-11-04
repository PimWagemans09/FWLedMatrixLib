#ifndef FW_LED_MATRIX_H
#define FW_LED_MATRIX_H
#include <string>
#include <cstdint>
#include <span>
#include <vector>

#include "internal/magic.h"

namespace fw_led_matrix {

    enum error {
        SUCCESS = 0,
        ERROR = 1,
        X_OUT_OF_BOUNDS = 2,
        Y_OUT_OF_BOUNDS = 3,
    };

    enum class Command : uint8_t {
        BRIGHTNESS = 0x00,
        PATTERN = 0x01,
        BOOTLOADER_RESET = 0x02,
        SLEEP = 0x03,
        ANIMATE = 0x04,
        PANIC = 0x05,
        DRAW = 0x06,
        STAGE_COL = 0x07,
        COMMIT_COL = 0x08,
        SET_TEXT = 0x09,
        START_GAME = 0x10,
        GAME_CONTROL = 0x11,
        GAME_STATUS = 0x12,
        SET_COLOR = 0x13,
        DISPLAY_ON = 0x14,
        INVERT_SCREEN = 0x15,
        SET_PIXEL_COLUMN = 0x16,
        FLUSH_FRAME_BUFFER = 0x17,
        CLEAR_RAM = 0x18,
        SCREEN_SAVER = 0x19,
        SET_FPS = 0x1A,
        SET_POWER_MODE = 0x1B,
        PWM_FREQ = 0x1E,
        DEBUG_MODE = 0x1F,
        VERSION = 0x20,
    };

    constexpr struct {
        const struct {
            const uint8_t SNAKE = 0x00;
            const uint8_t PONG = 0x01;
            const uint8_t TETRIS = 0x02;
            const uint8_t GAME_OF_LIFE = 0x03;
        } game_select;

        const struct {
            const uint8_t PERCENTAGE = 0x00;
            const uint8_t GRADIENT = 0x01;
            const uint8_t DOUBLE_GRADIENT = 0x02;
            const uint8_t DISPLAY_LOTUS = 0x03;
            const uint8_t ZIG_ZAG = 0x04;
            const uint8_t FULL_BRIGHTNESS = 0x05;
            const uint8_t DISPLAY_PANIC = 0x06;
            const uint8_t DISPLAY_LOTUS_2 = 0x07;
        } pattern;

        const struct {
            const uint8_t CURRENT_MATRIX = 0x00;
            const uint8_t PATTERN_1 = 0x01;
            const uint8_t BLINKER = 0x02;
            const uint8_t TOAD = 0x03;
            const uint8_t BEACON = 0x04;
            const uint8_t GLIDER = 0x05;
        } game_of_life_start_value;

        const struct{
            const uint8_t UP = 0;
            const uint8_t DOWN = 1;
            const uint8_t LEFT = 2;
            const uint8_t RIGHT = 3;
            const uint8_t QUIT = 4;
            const uint8_t LEFT2 = 5;
            const uint8_t RIGHT2 = 6;
        } game_control;
    } params;

    class LedMatrix {
    public:
        explicit LedMatrix(std::string path);
        ~LedMatrix() = default;

        /**
         * sends a command to the matrix
         * may block for up to 1.0s
         * @param cmd the Command to send
         * @param params a list of params to send, how many params are needed depends on the command
         * @param with_response if true, this will wait for a response for up to 1.0s,
         *      if no bytes where read the function wil return the timed out error code
         * @return an error code, returns 0 on success, returns errno on failure on linux
         */
        int send_command(Command cmd, const std::vector<uint8_t> &params, bool with_response = false);

        /**
         * get the last response
         * @return the last response
         */
        [[nodiscard]] const std::vector<uint8_t> &get_last_response() const;

        /**
         * get the internal matrix
         * @return the matrix
         */
        [[nodiscard]] const std::array<std::array<uint8_t, 34>, 9> &get_matrix() const;

        /**
         * blit some data to the internal matrix
         *
         * coordinates:
         * the top-left corner is (0, 0) the bottom-right corner is (8,33)
         *
         * @param data the data to blit, in column major order
         * @param x (accepted values: 0 to 8) where to blit the data on the x-axis, this will be used as the x-position of the top-left corner of data
         * @param y (accepted values: 0 to 33) where to blit the data on the y-axis, this will be used as the y-position of the top-left corner of data
         * @return `fw_led_matrix::SUCCESS` on success,
         *      `fw_led_matrix::X_OUT_OF_BOUNDS` if you are attempting to draw out of bounds on the x-axis,
         *      `fw_led_matrix::y_OUT_OF_BOUNDS` if you are attempting to draw out of bounds on the y-axis
         */
        int blit(const std::vector<std::vector<uint8_t>> &data, unsigned int x, unsigned int y);

        /**
         * draw the internal matrix using 1 bit color
         * the brightness can be set using `fw_led_matrix::send_command(fw_led_matrix::Command::BRIGHTNESS, { <BRIGHTNESS HERE> });`
         * @return an error code, returns 0 on success, returns errno on failure on linux
         */
        int draw_matrix_black_white();

        /**
         * draw the internal matrix using greyscale color
         * @return an error code, returns 0 on success, returns errno on failure on linux
         */
        int draw_matrix_greyscale();

        /**
         * sets the brightness of the LED matrix
         * @param brightness the new brightness
         * @return an error code, returns 0 on success, returns errno on failure on linux
         */
        int set_brightness(uint8_t brightness);

        /**
         * sets all values in the internal matrix to 0
         */
        void clear();

    private:
        std::string _path;
        std::vector<uint8_t> _response;
        std::array<std::array<uint8_t, 34>, 9> _matrix;
    };
}

#endif // FW_LED_MATRIX_H