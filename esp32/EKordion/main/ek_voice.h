#include "ek_config.h"
#include "ek_channel.h"

#ifndef EK_VOICE_H
#define EK_VOICE_H

typedef struct voice_s *voice_t;
voice_t ek_voice_create();
int16_t ek_voice_get_i_button(voice_t voice);
uint16_t ek_voice_get_ramp_step(voice_t voice);
void ek_voice_set_i_button(voice_t voice, int16_t i_button);
void ek_voice_change_phase_increment(
    voice_t voice, 
    uint8_t base_midi_note,
    int8_t octave
);
int32_t *ek_voice_compute(
    voice_t voice,
    int32_t *lfo_int32_buffer,
    channel_t channel
);
#endif