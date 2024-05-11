#include "ek_config.h"

#ifndef EK_I2S_H
#define EK_I2S_H

void ek_i2s_init();
void ek_i2s_write(
    int32_t output_l_int32_buffer[DMA_BUF_LEN],
    int32_t output_r_int32_buffer[DMA_BUF_LEN]
);

#endif