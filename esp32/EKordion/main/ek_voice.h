#include "ek_config.h"
#include "ek_channel.h"

#ifndef EK_VOICE_H
#define EK_VOICE_H

typedef struct voice_s *voice_t;
voice_t ek_voice_create(uint8_t base_midi_note);
int16_t ek_voice_get_i_button(voice_t voice);
uint16_t ek_voice_get_ramp_step(voice_t voice);
void ek_voice_change_table(voice_t voice, int16_t *table);
void ek_voice_change_resolution_mask(voice_t voice, int16_t resolution_mask);
void ek_voice_change_i_button(voice_t voice, int16_t i_button);
void ek_voice_change_octave(voice_t voice, int8_t octave);
void ek_voice_change_arpeggio_duration(voice_t voice, uint16_t arpeggio_duration);
void ek_voice_change_arpeggiator(voice_t voice, arpeggiator_t arpeggiator);
void ek_voice_change_vibrato(voice_t voice, uint8_t vibrato);
int32_t *ek_voice_compute(
    voice_t voice,
    int32_t *lfo_int32_buffer
);
#endif