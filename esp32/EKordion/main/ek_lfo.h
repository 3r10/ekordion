#include "ek_config.h"

#ifndef EK_LFO_H
#define EK_LFO_H

typedef struct ek_lfo_s *ek_lfo_t;

ek_lfo_t ek_lfo_create();
void ek_lfo_change_phase_increment(ek_lfo_t lfo, uint32_t phase_increment);
void ek_lfo_change_table(ek_lfo_t lfo, const int16_t *table);
int32_t *ek_lfo_compute(ek_lfo_t lfo);

#endif