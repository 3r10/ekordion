#include "ek_config.h"

#ifndef EK_VOICES_H
#define EK_VOICES_H

void ek_voices_init();
void ek_voices_change_custom_table(uint16_t length, uint8_t *data);
void ek_voices_change_volume(uint16_t length, uint8_t *data);
void ek_voices_change_octave(uint16_t length, uint8_t *data);
void ek_voices_change_table(uint16_t length, uint8_t *data);
void ek_voices_change_lfo_frequency(uint16_t length, uint8_t *data);
void ek_voices_change_lfo_table(uint16_t length, uint8_t *data);
void ek_voices_change_vibrato(uint16_t length, uint8_t *data);

void button_on(int i_channel,int i_button);
void button_off(int i_channel,int i_button);
void ek_voices_compute(int32_t output_int32_buffer[DMA_BUF_LEN]);

#endif