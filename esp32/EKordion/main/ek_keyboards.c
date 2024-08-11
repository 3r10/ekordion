#include "ek_keyboards.h"
#include "ek_voices.h"

static uint8_t left_switch_to_channel[N_LEFT_SWITCHES] = {
    BASS_CHANNEL, BASS_CHANNEL, CHORDS_CHANNEL, CHORDS_CHANNEL,
    BASS_CHANNEL, BASS_CHANNEL, CHORDS_CHANNEL, CHORDS_CHANNEL,
    BASS_CHANNEL, BASS_CHANNEL, CHORDS_CHANNEL, CHORDS_CHANNEL,
    BASS_CHANNEL, BASS_CHANNEL, CHORDS_CHANNEL, CHORDS_CHANNEL,
    BASS_CHANNEL, BASS_CHANNEL, CHORDS_CHANNEL, CHORDS_CHANNEL,
    BASS_CHANNEL, BASS_CHANNEL, CHORDS_CHANNEL, CHORDS_CHANNEL,
};

static uint8_t left_switch_to_button[N_LEFT_SWITCHES] = {
     5, 0, 5, 0, 
     7, 2, 7, 2,
     9, 4, 9, 4,
    11, 6,11, 6,
     1, 8, 1, 8, 
     3,10, 3,10,
};

static uint8_t right_switches_on_off[N_RIGHT_SWITCHES] = {0};
static uint8_t left_switches_on_off[N_LEFT_SWITCHES] = {0};

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
    for (int i=0; i<N_RIGHT_SWITCHES; i++) {
        // endianness correction
        int i_switch = i+7-2*(i%8);
        on_off = gpio_get_level(GPIO_RIGHT_SERIAL);
        if (on_off!=right_switches_on_off[i_switch]) {
            right_switches_on_off[i_switch] = on_off;
            // i_button = i_switch (strict correspondance here)
            if (on_off) button_on(LEAD_CHANNEL,i_switch);
            else button_off(LEAD_CHANNEL,i_switch);
        }
        gpio_set_level(GPIO_CLK,1);
        gpio_set_level(GPIO_CLK,0);
    }
    gpio_set_level(GPIO_SHIFT_LOAD,0);
    gpio_set_level(GPIO_SHIFT_LOAD,1);
    for (int i=0; i<N_LEFT_SWITCHES; i++) {
        // endianness correction and mapping
        int i_switch = i+7-2*(i%8);
        on_off = gpio_get_level(GPIO_LEFT_SERIAL);
        if (on_off!=left_switches_on_off[i_switch]) {
            int i_channel = left_switch_to_channel[i_switch];
            int i_button = left_switch_to_button[i_switch];
            left_switches_on_off[i_switch] = on_off;
            if (on_off) button_on(i_channel,i_button);
            else button_off(i_channel,i_button);
        }
        gpio_set_level(GPIO_CLK,1);
        gpio_set_level(GPIO_CLK,0);
    }
}
