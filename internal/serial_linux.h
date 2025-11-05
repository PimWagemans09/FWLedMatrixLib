//
// Created by pim on 11/2/25.
//

#ifndef FWLEDMATRIXLIB_SERIAL_LINUX_H
#define FWLEDMATRIXLIB_SERIAL_LINUX_H
#include <cstdint>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <cerrno>
#include <termios.h>

inline int platform_send_command(
        const std::string &device_path,
        const uint8_t data[],
        const size_t data_size,
        const bool with_response,
        std::vector<uint8_t> *response) {
    int error = 0;
    const int serial_port = open(device_path.c_str(), O_RDWR);

    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, std::strerror(errno));
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

#endif //FWLEDMATRIXLIB_SERIAL_LINUX_H
