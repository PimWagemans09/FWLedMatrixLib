//
// Created by pim on 11/3/25.
//

#ifndef FWLEDMATRIXLIB_SERIAL_WINDOWS_H
#define FWLEDMATRIXLIB_SERIAL_WINDOWS_H

#include <cstdint>
#include <string>
#include <vector>

#include "Windows.h"

inline int platform_send_command(
        const std::string &device_path,
        const uint8_t data[],
        const size_t data_size,
        const bool with_response,
        std::vector<uint8_t> &response) {
    int error = 0;



    if (error == 0) {
    }

    if (with_response and error == 0) {
        error = 1;
        printf("response not implemented on windows");
    }


    return error;
}

#endif //FWLEDMATRIXLIB_SERIAL_WINDOWS_H