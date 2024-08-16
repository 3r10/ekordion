#include "ek_config.h"

#ifndef EK_ADSR_H
#define EK_ADSR_H

#define ADSR_SHIFT_VALUE 22
#define ADSR_MAXIMUM_VALUE (1<<30)

typedef struct ek_adsr_s *ek_adsr_t;
ek_adsr_t ek_adsr_create();
int32_t ek_adsr_is_active(ek_adsr_t adsr);
void ek_adsr_compute(
    ek_adsr_t adsr, uint8_t on_off, 
    int32_t delta_a, int32_t delta_d, int32_t value_s, int32_t delta_r,
    int32_t output_int32_buffer[DMA_BUF_LEN]
);

#endif