//
// Created by pim on 10/24/25.
//
#include <cstring>

#include "../fw_led_matrix.h"

std::vector<std::vector<uint8_t>> image = {
    {
        {
            0,10,20,30,40
        },{
            50,60,70,80,90
        },{
            100,110,120,130,140
        },{
            150,160,170,180,190
        },{
            200,210,220,230,240
        }}
};


int main() {
    int r;
    fw_led_matrix::LedMatrix led_matrix("/dev/ttyACM0");

    fw_led_matrix::Version version{};
    r = led_matrix.get_version(&version);
    printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());
    printf("Version: %s\n", version.to_string().c_str());

    r = led_matrix.send_command(fw_led_matrix::Command::BRIGHTNESS, {0x14});
    printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());

    r = led_matrix.blit(image, 0 ,0);
    printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());

    r = led_matrix.draw_matrix_greyscale();
    printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());

    // r = led_matrix.game_start(fw_led_matrix::GameID::PONG);
    // printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());
    //
    // r = led_matrix.game_control(fw_led_matrix::GameControl::LEFT);
    // printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());
    //
    // r = led_matrix.game_quit();
    // printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());

    uint8_t brightness = 0;
    r = led_matrix.get_brightness(&brightness);
    printf("Error %d (%s)\n", r, fw_led_matrix::error_to_string(r).c_str());
    printf("Brightness: 0x%02x\n", brightness);
}
