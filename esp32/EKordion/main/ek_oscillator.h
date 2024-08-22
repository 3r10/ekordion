#include "ek_config.h"

#ifndef EK_OSCILLATOR_H
#define EK_OSCILLATOR_H


typedef struct ek_oscillator_parameters_s *ek_oscillator_parameters_t;
ek_oscillator_parameters_t ek_oscillator_parameters_create();
void ek_oscillator_parameters_change_n_oscillators(ek_oscillator_parameters_t parameters, uint8_t n_oscillators);
void ek_oscillator_parameters_change_detune_factor(ek_oscillator_parameters_t parameters, uint8_t detune_factor);
void ek_oscillator_parameters_change_table(ek_oscillator_parameters_t parameters, const int16_t *table);
void ek_oscillator_parameters_change_resolution_mask(ek_oscillator_parameters_t parameters, int16_t resolution_mask);

typedef struct ek_oscillator_s *ek_oscillator_t;
ek_oscillator_t ek_oscillator_create();
void ek_oscillator_compute(
    ek_oscillator_t oscillator, 
    ek_oscillator_parameters_t parameters,
    int32_t phase_increment_int32_buffer[DMA_BUF_LEN],
    int32_t output_int32_buffer[DMA_BUF_LEN]
);

#endif