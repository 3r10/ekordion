#include "ek_config.h"

#ifndef EK_PHASOR_H
#define EK_PHASOR_H

typedef struct ek_phasor_s *ek_phasor_t;
ek_phasor_t ek_phasor_create();
int16_t ek_phasor_get_note(ek_phasor_t phasor);
void ek_phasor_change_note(ek_phasor_t phasor, int16_t note);
void ek_phasor_actualize(ek_phasor_t phasor, int16_t base_note);
void ek_phasor_compute(ek_phasor_t phasor, int32_t output_int32_buffer[DMA_BUF_LEN]);

#endif