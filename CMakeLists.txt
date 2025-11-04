#For testing the library
cmake_minimum_required(VERSION 4.0)
project(FWLedMatrixLib)

set(CMAKE_CXX_STANDARD 20)

add_library(FWledMatrixLib STATIC fw_led_matrix.cpp fw_led_matrix.h
        internal/magic.h
        internal/serial_linux.h
        internal/serial_windows.h
)

if(PROJECT_IS_TOP_LEVEL)
    message(detected the project is being built as the top level project, building test)
    add_executable(Test test/test.cpp)

    target_link_libraries(Test PUBLIC FWledMatrixLib)
endif()