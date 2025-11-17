#ifndef FW_LED_MATRIX_H
#define FW_LED_MATRIX_H
#include <string>
#include <cstdint>
#include <span>
#include <vector>

namespace fw_led_matrix {
    constexpr unsigned char FW_MAGIC[2] = {0x32,0xAC};

    enum error {
        SUCCESS = 0,
        ERROR = -1,
        X_OUT_OF_BOUNDS = -2,
        Y_OUT_OF_BOUNDS = -3,
        EXTRA_PARAM_REQUIRED = -4,
        TOO_MANY_PARAMS = -5,
    };

    // according to https://github.com/FrameworkComputer/inputmodule-rs/blob/main/commands.md
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
        START_GAME = 0x10,
        GAME_CONTROL = 0x11,
        GAME_STATUS = 0x12,
        VERSION = 0x20,
    };

        enum class GameID: uint8_t {
            SNAKE = 0x00,
            PONG = 0x01,
            TETRIS = 0x02,
            GAME_OF_LIFE = 0x03,
        };

        enum class Pattern: uint8_t {
            PERCENTAGE = 0x00,
            GRADIENT = 0x01,
            DOUBLE_GRADIENT = 0x02,
            DISPLAY_LOTUS = 0x03,
            ZIG_ZAG = 0x04,
            FULL_BRIGHTNESS = 0x05,
            DISPLAY_PANIC = 0x06,
            DISPLAY_LOTUS_2 = 0x07,
        };

        enum class GameOfLifeStartParam: uint8_t {
            CURRENT_MATRIX = 0x00,
            PATTERN_1 = 0x01,
            BLINKER = 0x02,
            TOAD = 0x03,
            BEACON = 0x04,
            GLIDER = 0x05,
        };

        enum class GameControl: uint8_t {
            UP = 0,
            DOWN = 1,
            LEFT = 2,
            RIGHT = 3,
            QUIT = 4,
            LEFT2 = 5,
            RIGHT2 = 6,
        };

        struct Version {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
            bool is_prerelease;

            [[nodiscard]] std::string to_string() const {
                std::string suffix{};
                if (is_prerelease) {
                    suffix += "_prerelease";
                }
                return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch) + suffix;
            }
        };

        /**
         * convert an error code returned by this library to a string, prefixed with the source of the code.
         * @param error the code to format
         * @return a string with the error message, the string is prefixed with the source of the error code.
         * When the source was this library or `error` == 0, the message will be prefixed by "fw_led_matrix:".
         * When the source was errno on Linux the message will be prefixed by "linux_errno:".
         * When the source was GetLastError on Windows the message will be prefixed by "windows_getlasterror:".
         */
        std::string error_to_string(int error);

    class LedMatrix {
    public:
        explicit LedMatrix(std::string path);
        ~LedMatrix() = default;

        /**
         * convert `enum class`'s to their underlying value useful for `fw_led_matrix::Command`,`fw_led_matrix::Pattern`, `fw_led_matrix::Game`, and `fw_led_matrix::GameOfLifeStartParam`
         * @param e the enum value to convert
         * @return the underlying value of the enum
         */
        template <typename E>
        static constexpr typename std::underlying_type<E>::type enum_to_value(E e) noexcept {
            return static_cast<typename std::underlying_type<E>::type>(e);
        }

        /**
         * sends a command to the matrix
         * may block for up to 1.0s
         * @param cmd the Command to send
         * @param params a list of params to send, how many params are needed depends on the command
         * @param with_response if true, this will wait for a response for up to 1.0s,
         *      if no bytes where read the function wil return the timed out error code
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int send_command(Command cmd, const std::vector<uint8_t> &params, bool with_response = false);

        /**
         * get the last response
         * @return the last response
         */
        [[nodiscard]] const std::vector<uint8_t> &get_last_response() const;

        /**
         * sets the brightness of the LED matrix
         * @param brightness the new brightness
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int set_brightness(uint8_t brightness);

        /**
         * gets the brightness of the matrix and stores it in brightness_out
         * @param brightness_out where to store the brightness
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int get_brightness(uint8_t *brightness_out);

        /**
         * display e pre-programmed pattern on the matrix
         * @param pattern the pattern to display
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int display_pattern(Pattern pattern);

        /**
         * sets if the matrix is asleep
         * @param sleep the new sleep state
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int set_sleep(bool sleep);

        /**
         * gets if the matrix is asleep
         * @param sleep_out where to store the sleep state
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int get_sleep(bool *sleep_out);

        /**
         * sets if the current pattern should scroll
         * @param animate the new animation state
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int set_animate(bool animate);

        /**
         * gets if the current pattern is scrolling
         * @param animate_out where to store if the current pattern is scrolling
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int get_animate(bool *animate_out);

        /**
         * gets the version info of the matrix
         * @param version_out where to store the version info
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int get_version(Version *version_out);

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
         * set a specific pixel in the internal matrix
         *
         * coordinates:
         * the top-left corner is (0, 0) the bottom-right corner is (8,33)
         *
         * @param value the new value for the pixel
         * @param x (accepted values: 0 to 8) the x position of the pixel
         * @param y (accepted values: 0 to 33) the y position of the pixel
         * @return `fw_led_matrix::SUCCESS` on success,
         *      `fw_led_matrix::X_OUT_OF_BOUNDS` if you are attempting to draw out of bounds on the x-axis,
         *      `fw_led_matrix::y_OUT_OF_BOUNDS` if you are attempting to draw out of bounds on the y-axis
         */
        int set_pixel(uint8_t value, unsigned int  x, unsigned int y);

        /**
         * draw the internal matrix using 1 bit color
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int draw_matrix_black_white();

        /**
         * draw the internal matrix using greyscale color
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int draw_matrix_greyscale();

        /**
         * sets all values in the internal matrix to 0
         */
        void clear();

        /**
         * start a preloaded game on the matrix
         * most other commands will stop working
         *
         * If you want to start the game of life you need to pass an extra parameter to this function.
         * It must be one of the values in `fw_led_matrix::params.game_of_life_start_param`
         * @param game_id the game to start
         * @return An error code.
         * Returns 0 on success.
         * Returns `fw_led_matrix::EXTRA_PARAM_REQUIRED` if you try to start the game of life WITHOUT the extra param.
         * Returns `fw_led_matrix::TOO_MANY_PARAMS` if you try to start any other game WITH the extra param.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int game_start(GameID game_id);

        /**
         * start a preloaded game on the matrix
         * most other commands will stop working
         *
         * If you want to start the game of life you need to pass an extra parameter to this function.
         * It must be one of the values in `fw_led_matrix::params.game_of_life_start_param`
         * @param game_id the game to start
         * @param game_of_life_param an extra param needed when starting the game of life
         * @return An error code.
         * Returns 0 on success.
         * Returns `fw_led_matrix::EXTRA_PARAM_REQUIRED` if you try to start the game of life WITHOUT the extra param.
         * Returns `fw_led_matrix::TOO_MANY_PARAMS` if you try to start any other game WITH the extra param.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int game_start(GameID game_id, GameOfLifeStartParam game_of_life_param);

        /**
         * quits the currently running preloaded game
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int game_quit();

        /**
         * sends a control value to the game
         * @param game_control_value the control value to send
         * @return An error code.
         * Returns 0 on success.
         * Returns errno on failure on linux.
         * Returns the result of GetLastError() on failure on windows.
         */
        int game_control(GameControl game_control_value);

    private:
        std::string _path;
        std::vector<uint8_t> _response;
        std::array<std::array<uint8_t, 34>, 9> _matrix;
    };
}

#endif // FW_LED_MATRIX_H