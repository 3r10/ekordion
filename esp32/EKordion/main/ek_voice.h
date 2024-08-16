#include "ek_config.h"
#include "ek_channel.h"

#ifndef EK_VOICE_H
#define EK_VOICE_H

typedef struct voice_s *voice_t;
voice_t ek_voice_create();
void ek_voice_change_table(voice_t voice, int16_t *table);
void ek_voice_change_resolution_mask(voice_t voice, int16_t resolution_mask);
void ek_voice_change_arpeggio_duration(voice_t voice, uint16_t arpeggio_duration);
void ek_voice_change_arpeggiator(voice_t voice, arpeggiator_t arpeggiator);
void ek_voice_compute(
    voice_t voice,
    int32_t *lfo_int32_buffer,
    int32_t *phase_increment_int32_buffer,
    int32_t *output_int32_buffer
);
#endif