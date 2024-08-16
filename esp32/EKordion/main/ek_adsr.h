#include "ek_config.h"

#ifndef EK_ADSR_H
#define EK_ADSR_H

// Prototype version : no ADSR parameters, juste in/out fixed ramps
typedef struct ek_adsr_s *ek_adsr_t;
ek_adsr_t ek_adsr_create();
uint16_t ek_adsr_is_active(ek_adsr_t adsr);
void ek_adsr_compute(ek_adsr_t adsr, uint8_t on_off, int32_t output_int32_buffer[DMA_BUF_LEN]);

#endif