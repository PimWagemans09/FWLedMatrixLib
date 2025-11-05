//
// Created by pim on 11/3/25.
//

#ifndef FWLEDMATRIXLIB_SERIAL_WINDOWS_H
#define FWLEDMATRIXLIB_SERIAL_WINDOWS_H

#include <chrono>
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
    DWORD error = ERROR_SUCCESS;

    DWORD bytesWritten = 0;
    OVERLAPPED ov = {};

    HANDLE handle = ::CreateFile(device_path.c_str(),
                                          GENERIC_READ | GENERIC_WRITE, //access ( read and write)
                                          1, //(share) 0:cannot share the COM port
                                          nullptr, //security  (None)
                                          OPEN_EXISTING, // creation : open_existing
                                          FILE_ATTRIBUTE_NORMAL,
                                          nullptr // no templates file for COM port...
    );

    if (handle == INVALID_HANDLE_VALUE) {
        printf("Failed to open COM port\n");
        return GetLastError();
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

    /*
    //const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while (!HasOverlappedIoCompleted(&ov)) {
        const DWORD lastError = GetLastError();
        if (lastError == ERROR_SUCCESS) {
            continue;
        }
        if (lastError != ERROR_IO_PENDING) {
            error = lastError;
            break;

        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

        auto time_span = duration_cast<std::chrono::duration<double>>(t2 - start);
        if (time_span.count() >= 10) {
            printf("ERROR: led matrix cmd timeout\n");
            if (!CancelIoEx(handle, &ov)) {
                printf("ERROR: io cancel failed\n");
            }
            error = ERROR_TIMEOUT;
            break;
        }
    }*/

    if (with_response and error == ERROR_SUCCESS) {
        uint8_t buffer[32];
        DWORD bytesRead = 0;
        if (!ReadFile(handle, &buffer, 32, &bytesRead, nullptr)) {
            printf("READ failed\n");
        }

        response.clear();
        response.insert(response.end(), buffer, buffer + sizeof(buffer));
        error = GetLastError();
        /*while (!HasOverlappedIoCompleted(&ov)) {
            const DWORD lastError = GetLastError();
            if (lastError == ERROR_SUCCESS) {
                continue;
            }
            if (lastError != ERROR_IO_PENDING) {
                error = lastError;
                break;
            }
        }*/
    }

    CloseHandle(handle);

    return error;
}

#endif //FWLEDMATRIXLIB_SERIAL_WINDOWS_H