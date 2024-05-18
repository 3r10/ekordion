#include "ek_config.h"

#ifndef EK_VOICES_H
#define EK_VOICES_H

void ek_voices_init();
void ek_voices_change_custom_table(uint16_t length, uint8_t *data);
void ek_voices_change_bass_volume(uint16_t length, uint8_t *data);
void ek_voices_change_chords_volume(uint16_t length, uint8_t *data);
void ek_voices_change_lead_volume(uint16_t length, uint8_t *data);
void ek_voices_change_bass_table(uint16_t length, uint8_t *data);
void ek_voices_change_chords_table(uint16_t length, uint8_t *data);
void ek_voices_change_lead_table(uint16_t length, uint8_t *data);

void left_button_on(int i_button);
void left_button_off(int i_button);
void right_button_on(int i_button);
void right_button_off(int i_button);
void ek_voices_compute(int32_t output_int32_buffer[DMA_BUF_LEN]);

#endif