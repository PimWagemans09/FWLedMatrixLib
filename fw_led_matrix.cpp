#include "fw_led_matrix.h"

#include <algorithm>
#include <iostream>
#include <span>
#include <utility>

#include "internal/magic.h"

#if defined(__linux)
#include "internal/serial_linux.h"
#elif defined(__WIN32)
#include "serial_windows.h"
#else
#error unsupported OS. only linux and windows are supported. make sure either __linux or __WIN32 is defined
// avoid other errors caused by above error
inline int platform_send_command(
        const std::string &device_path,
        const uint8_t data[],
        size_t data_size,
        bool with_response,
        std::vector<uint8_t> &response);
#endif

namespace fw_led_matrix {

    LedMatrix::LedMatrix(std::string path): _path(std::move(path)), _matrix({{}}) {}

    int LedMatrix::send_command(Command cmd, const std::vector<uint8_t> &params, const bool with_response) {
        const size_t n = params.size() + 3;

        uint8_t bytes[n];

        std::ranges::copy(FW_MAGIC, bytes);
        bytes[2] = static_cast<char>(cmd);
        std::ranges::copy(params, bytes + 3);

        return platform_send_command(_path, bytes, n, with_response, _response);
    }

    const std::vector<uint8_t> &LedMatrix::get_last_response() const {
        return _response;
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

    int LedMatrix::set_brightness(const uint8_t brightness) {
        return send_command(Command::BRIGHTNESS, {brightness}, false);
    }
}
