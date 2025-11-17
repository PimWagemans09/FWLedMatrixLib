#include "fw_led_matrix.h"

#include <algorithm>
#include <iostream>
#include <span>
#include <utility>

#if defined(__linux)

#include <cstdint>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <cerrno>
#include <termios.h>

static int platform_send_command(
        const std::string &device_path,
        const uint8_t data[],
        const size_t data_size,
        const bool with_response,
        std::vector<uint8_t> *response) {
    int error = 0;
    const int serial_port = open(device_path.c_str(), O_RDWR);

    if (serial_port < 0) {
        return errno;
    }

    termios tty{};

    if(tcgetattr(serial_port, &tty) != 0) {
        return 1;
    }

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;
    ssize_t r = tcsetattr(serial_port, TCSANOW, &tty);
    if (r < 0) {
        error = errno;
    }

    if (error == 0) {
        r = write(serial_port, data, data_size);
        if (r < 0) {
            error = errno;
        }
    }

    if (with_response and error == 0) {
        response->clear();
        uint8_t read_buffer[32];
        r = read(serial_port, read_buffer, sizeof(read_buffer));
        if (r < 0) {
            error = errno;
        } else if (r == 0) {
            error = ETIMEDOUT;
        }else {
            response->insert(response->end(), read_buffer, read_buffer + sizeof(read_buffer));
        }
    }

    close(serial_port);

    return error;
}

static std::string platform_error_to_string(const int error) {
    return "linux_errno:" + std::string(strerror(error));
}

#elif defined(__WIN32)

#include <chrono>
#include <string>
#include <vector>

#include "Windows.h"
#include "intsafe.h"

static int platform_send_command(
        const std::string &device_path,
        const uint8_t data[],
        const size_t data_size,
        const bool with_response,
        std::vector<uint8_t> *response) {
    DWORD error = ERROR_SUCCESS;

    DWORD bytesWritten = 0;

    HANDLE handle = ::CreateFile(device_path.c_str(),
                                          GENERIC_READ | GENERIC_WRITE, //access ( read and write)
                                          1, //(share) 0:cannot share the COM port
                                          nullptr, //security  (None)
                                          OPEN_EXISTING, // creation : open_existing
                                          FILE_ATTRIBUTE_NORMAL,
                                          nullptr // no templates file for COM port...
    );

    if (handle == INVALID_HANDLE_VALUE) {
        int error_int;
        DWordToInt(GetLastError(), &error_int);
        return error_int;
    }

    DCB serialParams;
    GetCommState(handle, &serialParams);
    serialParams.BaudRate = 115200;
    SetCommState(handle, &serialParams);

    COMMTIMEOUTS commPortTimeouts;

    commPortTimeouts.ReadIntervalTimeout = 1000;
    commPortTimeouts.ReadTotalTimeoutMultiplier = 1000;
    commPortTimeouts.ReadTotalTimeoutConstant = 1000;
    commPortTimeouts.WriteTotalTimeoutMultiplier = 1000;
    commPortTimeouts.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(handle, &commPortTimeouts);

    WriteFile(handle, data, data_size, &bytesWritten, nullptr);

    error = GetLastError();

    if (with_response and error == ERROR_SUCCESS) {
        uint8_t buffer[32];
        DWORD bytesRead = 0;
        ReadFile(handle, &buffer, 32, &bytesRead, nullptr);

        response->clear();
        response->insert(response->end(), buffer, buffer + sizeof(buffer));
        error = GetLastError();
    }

    CloseHandle(handle);

    int error_int;
    DWordToInt(error, &error_int);
    return error_int;
}

static std::string platform_error_to_string(int error) {
    std::string message;
    LPVOID lpMsgBuf;
    DWORD bufLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        reinterpret_cast<LPTSTR>(&lpMsgBuf),
        0, nullptr );
    if (bufLen) {
        const LPCSTR lpMsgStr = static_cast<LPCSTR>(lpMsgBuf);
        message = std::string(lpMsgStr, lpMsgStr+bufLen);

        LocalFree(lpMsgBuf);
    }
    return "windows_getlasterror:" + std::string(message);
}

#else
#error unsupported OS. only linux and windows are supported. make sure either __linux or __WIN32 is defined
// avoid other errors caused by above error
static int platform_send_command(
        const std::string &device_path,
        const uint8_t data[],
        size_t data_size,
        bool with_response,
        std::vector<uint8_t> *response);

static std::string platform_error_to_string(int error);
#endif

namespace fw_led_matrix {

    std::string error_to_string(const int error) {
        if (error <= 0) {
            switch (error) {
                case 0:
                    return "fw_led_matrix:Success";
                case -1:
                    return "fw_led_matrix:Error";
                case -2:
                    return "fw_led_matrix:X out of bounds";
                case -3:
                    return "fw_led_matrix:Y out of bounds";
                case -4:
                    return "fw_led_matrix:Extra parameter required";
                case -5:
                    return "fw_led_matrix:Too many parameters";
                default:
                    return "fw_led_matrix:Unknown error";
            }
        }
        return platform_error_to_string(error);
    }

    LedMatrix::LedMatrix(std::string path): _path(std::move(path)), _matrix({{}}) {}

    int LedMatrix::send_command(Command cmd, const std::vector<uint8_t> &params, const bool with_response) {
        const size_t n = params.size() + 3;

        uint8_t bytes[n];

        std::ranges::copy(FW_MAGIC, bytes);
        bytes[2] = enum_to_value(cmd);
        std::ranges::copy(params, bytes + 3);

        return platform_send_command(_path, bytes, n, with_response, &_response);
    }

    const std::vector<uint8_t> &LedMatrix::get_last_response() const {
        return _response;
    }

    int LedMatrix::set_brightness(const uint8_t brightness) {
        return send_command(Command::BRIGHTNESS, {brightness}, false);
    }

    int LedMatrix::get_brightness(uint8_t *brightness_out) {
        int r = send_command(Command::BRIGHTNESS, {}, true);
        if (r == 0) {
            *brightness_out = get_last_response()[0];
        }
        return r;
    }

    int LedMatrix::display_pattern(Pattern pattern) {
        return send_command(Command::PATTERN, {enum_to_value(pattern)}, false);
    }

    int LedMatrix::set_sleep(bool sleep) {
        return send_command(Command::SLEEP, {sleep});
    }

    int LedMatrix::get_sleep(bool *sleep_out) {
        int r = send_command(Command::SLEEP, {}, true);
        if (r == 0) {
            *sleep_out = get_last_response()[0];
        }
        return r;
    }

    int LedMatrix::set_animate(bool animate) {
        return send_command(Command::ANIMATE, {animate});
    }

    int LedMatrix::get_animate(bool *animate_out) {
        int r = send_command(Command::ANIMATE, {}, true);
        if (r == 0) {
            *animate_out = get_last_response()[0];
        }
        return r;
    }

    int LedMatrix::get_version(Version *version_out) {
        int r = send_command(Command::VERSION, {}, true);
        if (r == 0) {
            version_out->major = get_last_response()[0];
            version_out->minor = (get_last_response()[1] & 0b11110000) >> 4;
            version_out->patch = get_last_response()[1] & 0b00001111;
            version_out->is_prerelease = get_last_response()[2] & 0b00000001;
        }
        return r;
    }


    const std::array<std::array<uint8_t, 34>, 9> &LedMatrix::get_matrix() const {
        return _matrix;
    }

    int LedMatrix::blit(const std::vector<std::vector<uint8_t> >& data, const unsigned int x, const unsigned int y) {
        unsigned int y_size = 0;
        for (const auto& i : data) {
            if (i.size() > y_size) {
                y_size = i.size();
            }
        }
        if ( y + y_size > 34) {
            return Y_OUT_OF_BOUNDS;
        }
        if ( x + data.size() > 9) {
            return X_OUT_OF_BOUNDS;
        }

        for (size_t i = 0; i < data.size(); i++) {
            for (size_t j = 0; j < data[i].size(); j++) {
                _matrix[x+ i][y + j] = data[i][j];
            }
        }

        return SUCCESS;
    }

    int LedMatrix::set_pixel(const uint8_t value, const unsigned int x, const unsigned int y) {
        if (x > 8) {
            return X_OUT_OF_BOUNDS;
        }
        if (y > 33) {
            return Y_OUT_OF_BOUNDS;
        }
        _matrix[x][y] = value;
        return SUCCESS;
    }


    int LedMatrix::draw_matrix_black_white() {
        std::vector<uint8_t> vals = {};

        for (int i = 0; i < 39; i++) {
            vals.push_back(0);
        }

        for (int x = 0; x < 9; x++) {
            for (int y = 0; y < 34; y++) {
                const size_t index = x + 9 * y;
                if (_matrix[x][y]) {
                    vals[index / 8u] = vals[index / 8u] | (1 << (index % 8u));
                }
            }
        }
        return send_command(Command::DRAW, vals, false);
    }

    int LedMatrix::draw_matrix_greyscale() {
        int r;
        for (uint8_t x = 0; x < 9; x++) {
            std::vector<uint8_t> vals = {x};
            vals.insert(vals.end(), std::begin(_matrix[x]), std::end(_matrix[x]));
            r  = send_command(Command::STAGE_COL, vals, false);
            if ( r != 0 ) {
                return r;
            }
        }
        r = send_command(Command::COMMIT_COL, {0x00}, false);
        if ( r != 0 ) {
            return r;
        }
        return SUCCESS;
    }

    void LedMatrix::clear() {
        for (int x = 0; x < 9; x++) {
            for (int y = 0; y < 34; y++) {
                _matrix[x][y] = 0;
            }
        }
    }

    int LedMatrix::game_start(const GameID game_id) {
        if (game_id == GameID::GAME_OF_LIFE) {
            return EXTRA_PARAM_REQUIRED;
        }
        return send_command(Command::START_GAME, std::vector<uint8_t>{enum_to_value(game_id)}, false);
    }

    int LedMatrix::game_start(const GameID game_id, const GameOfLifeStartParam game_of_life_param) {
        if (game_id != GameID::GAME_OF_LIFE) {
            return TOO_MANY_PARAMS;
        }
        return send_command(Command::START_GAME, std::vector<uint8_t>{enum_to_value(game_id), enum_to_value(game_of_life_param)}, false);
    }

    int LedMatrix::game_quit() {
        return send_command(Command::GAME_CONTROL, {enum_to_value(GameControl::QUIT)}, false);
    }

    int LedMatrix::game_control(GameControl game_control_value) {
        return send_command(Command::GAME_CONTROL, {enum_to_value(game_control_value)}, false);
    }

}
