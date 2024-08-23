#include "ek_config.h"

#ifndef EK_MONOPOLY_H
#define EK_MONOPOLY_H

#define MONOPOLY_MODE_POLY             0
#define MONOPOLY_MODE_MONO_LATEST      1
#define MONOPOLY_MODE_MONO_HIGHEST     2
#define MONOPOLY_MODE_MONO_ARP_FWD     3
#define MONOPOLY_MODE_MONO_ARP_UP      4
#define MONOPOLY_MODE_CHORD_2_LATEST   5
#define MONOPOLY_MODE_CHORD_3_LATEST   6
#define MONOPOLY_MODE_CHORD_4_LATEST   7
#define MONOPOLY_MODE_CHORD_2_HIGHEST  8
#define MONOPOLY_MODE_CHORD_3_HIGHEST  9
#define MONOPOLY_MODE_CHORD_4_HIGHEST 10

typedef struct ek_monopoly_s *ek_monopoly_t;
ek_monopoly_t ek_monopoly_create(uint8_t n_voices, uint8_t base_midi_note);
void ek_monopoly_change_octave(ek_monopoly_t monopoly, int8_t octave);
void ek_monopoly_change_mode(ek_monopoly_t monopoly, uint8_t mode);
void ek_monopoly_change_tonality(ek_monopoly_t monopoly, uint8_t tonality);
void ek_monopoly_change_chord_pattern(ek_monopoly_t monopoly, uint8_t i_chord_pattern);
void ek_monopoly_change_arpeggio_duration(ek_monopoly_t monopoly, uint16_t arpeggio_duration);
void ek_monopoly_button_on(ek_monopoly_t monopoly, int16_t button);
void ek_monopoly_button_off(ek_monopoly_t monopoly, int16_t button);
void ek_monopoly_deactivate_voice(ek_monopoly_t monopoly, uint8_t i_voice);
uint8_t ek_monopoly_is_voice_active(ek_monopoly_t monopoly, uint8_t i_voice);
uint8_t ek_monopoly_is_voice_on(ek_monopoly_t monopoly, uint8_t i_voice);
uint32_t ek_monopoly_get_voice_phase_increment(ek_monopoly_t monopoly, uint8_t i_voice);
void ek_monopoly_compute(ek_monopoly_t monopoly);

#endif