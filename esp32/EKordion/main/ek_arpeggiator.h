#include "ek_config.h"

#ifndef EK_ARPEGGIATOR_H
#define EK_ARPEGGIATOR_H

#define N_ARPEGGIATORS 8

typedef struct arpeggiator_s *arpeggiator_t;
arpeggiator_t ek_arpeggiator_get(uint8_t i_arpeggiator);
uint8_t ek_arpeggiator_get_length(arpeggiator_t arpeggiator);
uint8_t ek_arpeggiator_get_factor(arpeggiator_t arpeggiator, uint8_t step);
uint8_t ek_arpeggiator_get_shift(arpeggiator_t arpeggiator, uint8_t step);

#endif