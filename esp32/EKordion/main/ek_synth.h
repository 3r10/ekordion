#include "ek_config.h"

#ifndef EK_SYNTH_H
#define EK_SYNTH_H

void ek_synth_init();
void ek_synth_change_lfo_frequency(uint16_t length, uint8_t *data);
void ek_synth_change_lfo_table(uint16_t length, uint8_t *data);
void ek_synth_change_custom_table(uint16_t length, uint8_t *data);
void ek_synth_change_table(uint16_t length, uint8_t *data);
void ek_synth_change_resolution(uint16_t length, uint8_t *data);
void ek_synth_change_downsampling(uint16_t length, uint8_t *data);
void ek_synth_change_octave(uint16_t length, uint8_t *data);
void ek_synth_change_arpeggio_duration(uint16_t length, uint8_t *data);
void ek_synth_change_arpeggiator(uint16_t length, uint8_t *data);
void ek_synth_change_vibrato(uint16_t length, uint8_t *data);
void ek_synth_change_tremolo(uint16_t length, uint8_t *data);
void ek_synth_change_dry_volume(uint16_t length, uint8_t *data);
void ek_synth_change_wet_volume(uint16_t length, uint8_t *data);

void ek_synth_button_on(uint8_t i_channel,int16_t i_button);
void ek_synth_button_off(uint8_t i_channel,int16_t i_button);
void ek_synth_compute(    
    int32_t output_dry_int32_buffer[DMA_BUF_LEN],
    int32_t output_wet_int32_buffer[DMA_BUF_LEN]
);

#endif