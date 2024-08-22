#include "ek_config.h"

#ifndef EK_FILTER_H
#define EK_FILTER_H

typedef struct ek_filter_parameters_s *ek_filter_parameters_t;
ek_filter_parameters_t ek_filter_parameters_create();
void ek_filter_parameters_change_low_f(ek_filter_parameters_t parameters, uint8_t low_f_slider);
void ek_filter_parameters_change_high_f(ek_filter_parameters_t parameters, uint8_t high_f_slider);
void ek_filter_parameters_change_q(ek_filter_parameters_t parameters, uint8_t q_slider);

typedef struct ek_filter_s *ek_filter_t;
ek_filter_t ek_filter_create();
void ek_filter_compute(
    ek_filter_t filter, 
    ek_filter_parameters_t parameters,
    int32_t intput_output_int32_buffer[DMA_BUF_LEN]
);
void ek_dynamic_filter_compute(
    ek_filter_t filter, 
    ek_filter_parameters_t parameters,
    int32_t modulation_input_buffer[DMA_BUF_LEN],
    int32_t intput_output_int32_buffer[DMA_BUF_LEN]
);

#endif