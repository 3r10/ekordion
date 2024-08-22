#include "ek_config.h"

#ifndef EK_MONOPOLY_H
#define EK_MONOPOLY_H

typedef struct ek_monopoly_s *ek_monopoly_t;
ek_monopoly_t ek_monopoly_create(uint8_t n_voices, uint8_t base_midi_note);
void ek_monopoly_button_on(ek_monopoly_t monopoly, int16_t i_button);
void ek_monopoly_button_off(ek_monopoly_t monopoly, int16_t i_button);
void ek_monopoly_change_octave(ek_monopoly_t monopoly, int8_t octave);
void ek_monopoly_deactivate_voice(ek_monopoly_t monopoly, uint8_t i_voice);
uint8_t ek_monopoly_is_voice_active(ek_monopoly_t monopoly, uint8_t i_voice);
uint8_t ek_monopoly_is_voice_on(ek_monopoly_t monopoly, uint8_t i_voice);
uint32_t ek_monopoly_get_voice_phase_increment(ek_monopoly_t monopoly, uint8_t i_voice);

#endif