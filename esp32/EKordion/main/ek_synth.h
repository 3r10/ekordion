#include "ek_config.h"

#ifndef EK_SYNTH_H
#define EK_SYNTH_H

void ek_synth_init(
    uint8_t n_channels, 
    uint8_t *n_voices_array,      // of size n_channels
    uint8_t *base_midi_note_array // of size n_channels
);
void ek_synth_change_parameter(uint16_t length, uint8_t *data);
void ek_synth_button_on(uint8_t i_channel,int16_t i_button);
void ek_synth_button_off(uint8_t i_channel,int16_t i_button);
void ek_synth_compute(    
    int32_t output_l_int32_buffer[DMA_BUF_LEN],
    int32_t output_r_int32_buffer[DMA_BUF_LEN]
);

#endif