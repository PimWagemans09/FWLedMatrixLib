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
    fw_led_matrix::LedMatrix led_matrix("/dev/ttyACM0");
    int r = led_matrix.send_command(fw_led_matrix::Command::VERSION, {}, true);
    if (r != 0) {
        printf("Error %d (%s)\n", r, strerror(r));
    }
    const std::vector<uint8_t> buffer = led_matrix.get_last_response();
    for (const unsigned char i : buffer) {
        printf("%02x", i);
    }
    printf("\n");

    r = led_matrix.send_command(fw_led_matrix::Command::BRIGHTNESS, {0x14});
    printf("%d", r);

    led_matrix.blit(image, 1,1);
    led_matrix.draw_matrix_greyscale();

    led_matrix.send_command(fw_led_matrix::Command::GAME_CONTROL, {fw_led_matrix::params.game_control.QUIT});

    led_matrix.send_command(fw_led_matrix::Command::PATTERN, {fw_led_matrix::params.pattern.ZIG_ZAG});

}
