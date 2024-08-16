#include "ek_config.h"

#ifndef EK_ARPEGGIATOR_H
#define EK_ARPEGGIATOR_H

typedef struct ek_arpeggiator_s *ek_arpeggiator_t;

ek_arpeggiator_t ek_arpeggiator_create();
void ek_arpeggiator_compute(
    ek_arpeggiator_t arpeggiator,
    uint8_t i_pattern, uint16_t duration,
    int32_t *intput_output_int32_buffer
);

#endif