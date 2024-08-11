#include "ek_config.h"

#ifndef EK_REVERB_H
#define EK_REVERB_H

void ek_reverb_change_damping(uint16_t length, uint8_t *data);
void ek_reverb_change_feedback(uint16_t length, uint8_t *data);
void ek_reverb_change_volume(uint16_t length, uint8_t *data);
void ek_reverb_compute(
    int32_t input_dry_int32_buffer[DMA_BUF_LEN], 
    int32_t input_wet_int32_buffer[DMA_BUF_LEN],
    int32_t output_l_int32_buffer[DMA_BUF_LEN],
    int32_t output_r_int32_buffer[DMA_BUF_LEN]
);

#endif
