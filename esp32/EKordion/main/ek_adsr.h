#include "ek_config.h"

#ifndef EK_ADSR_H
#define EK_ADSR_H

#define ADSR_STATE_IDLE    0
#define ADSR_STATE_ATTACK  1
#define ADSR_STATE_DECAY   2
#define ADSR_STATE_SUSTAIN 3
#define ADSR_STATE_RELEASE 4


typedef struct ek_adsr_parameters_s *ek_adsr_parameters_t;
ek_adsr_parameters_t ek_adsr_parameters_create();
void ek_adsr_parameters_change_a(ek_adsr_parameters_t parameters, uint8_t a);
void ek_adsr_parameters_change_d(ek_adsr_parameters_t parameters, uint8_t d);
void ek_adsr_parameters_change_s(ek_adsr_parameters_t parameters, uint8_t s);
void ek_adsr_parameters_change_r(ek_adsr_parameters_t parameters, uint8_t r);

typedef struct ek_adsr_s *ek_adsr_t;
ek_adsr_t ek_adsr_create();
int32_t ek_adsr_is_active(ek_adsr_t adsr);
uint8_t ek_adsr_compute(
    ek_adsr_t adsr, uint8_t is_on, 
    ek_adsr_parameters_t parameters,
    int32_t output_int32_buffer[DMA_BUF_LEN]
);

#endif