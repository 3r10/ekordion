#include "ek_keyboards.h"
#include "ek_voices.h"

static uint8_t left_buttons_map[N_LEFT_BUTTONS] = {
     5, 0,17,12, 
     7, 2,19,14,
     9, 4,21,16,
    11, 6,23,18,
     1, 8,13,20, 
     3,10,15,22
};

static uint8_t right_buttons_on_off[N_RIGHT_BUTTONS] = {0};
static uint8_t left_buttons_on_off[N_LEFT_BUTTONS] = {0};

void ek_keyboards_init() {
    gpio_reset_pin(GPIO_SHIFT_LOAD);
    gpio_set_direction(GPIO_SHIFT_LOAD, GPIO_MODE_OUTPUT);
    gpio_reset_pin(GPIO_CLK);
    gpio_set_direction(GPIO_CLK, GPIO_MODE_OUTPUT);
    gpio_reset_pin(GPIO_RIGHT_SERIAL);
    gpio_set_direction(GPIO_RIGHT_SERIAL, GPIO_MODE_INPUT);
    gpio_reset_pin(GPIO_LEFT_SERIAL);
    gpio_set_direction(GPIO_LEFT_SERIAL, GPIO_MODE_INPUT);
}

void ek_keyboards_get_data() {
    uint8_t on_off; 

    gpio_set_level(GPIO_SHIFT_LOAD,0);
    gpio_set_level(GPIO_SHIFT_LOAD,1);
    for (int i=0; i<N_RIGHT_BUTTONS; i++) {
        // endianness correction
        int i_button = i+7-2*(i%8);
        on_off = gpio_get_level(GPIO_RIGHT_SERIAL);
        if (on_off!=right_buttons_on_off[i_button]) {
            right_buttons_on_off[i_button] = on_off;
            if (on_off) right_button_on(i_button);
            else right_button_off(i_button);
        }
        gpio_set_level(GPIO_CLK,1);
        gpio_set_level(GPIO_CLK,0);
    }
    gpio_set_level(GPIO_SHIFT_LOAD,0);
    gpio_set_level(GPIO_SHIFT_LOAD,1);
    for (int i=0; i<N_LEFT_BUTTONS; i++) {
        // endianness correction and mapping
        int i_button = left_buttons_map[i+7-2*(i%8)];
        on_off = gpio_get_level(GPIO_LEFT_SERIAL);
        if (on_off!=left_buttons_on_off[i_button]) {
            left_buttons_on_off[i_button] = on_off;
            if (on_off) left_button_on(i_button);
            else left_button_off(i_button);
        }
        gpio_set_level(GPIO_CLK,1);
        gpio_set_level(GPIO_CLK,0);
    }
}
