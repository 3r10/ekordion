#include "ek_config.h"
#include "ek_arpeggiator.h"

#ifndef EK_CHANNEL_H
#define EK_CHANNEL_H

typedef struct channel_s *channel_t;
channel_t ek_channel_create(uint8_t n_voices, uint8_t base_midi_note);
void ek_channel_change_table(channel_t channel, int16_t *table);
void ek_channel_change_resolution_mask(channel_t channel, int16_t resolution_mask);
void ek_channel_change_downsampling(channel_t channel, uint8_t downsampling);
void ek_channel_change_octave(channel_t channel, int8_t octave);
void ek_channel_change_arpeggio_duration(channel_t channel, uint16_t arpeggio_duration);
void ek_channel_change_arpeggiator_pattern(channel_t channel, uint8_t i_pattern);
void ek_channel_change_vibrato(channel_t channel, uint8_t vibrato);
void ek_channel_change_envelope_a(channel_t channel, uint8_t a);
void ek_channel_change_envelope_d(channel_t channel, uint8_t d);
void ek_channel_change_envelope_s(channel_t channel, uint8_t s);
void ek_channel_change_envelope_r(channel_t channel, uint8_t r);
void ek_channel_change_tremolo(channel_t channel, uint8_t tremolo);
void ek_channel_change_dry_volume(channel_t channel, uint8_t volume);
void ek_channel_change_wet_volume(channel_t channel, uint8_t volume);
//
void ek_channel_button_on(channel_t channel, int16_t i_button);
void ek_channel_button_off(channel_t channel, int16_t i_button);
void ek_channel_compute(    
    channel_t channel,
    int32_t *lfo_int32_buffer,
    int32_t *input_output_dry_int32_buffer,
    int32_t *input_output_wet_int32_buffer    
);

#endif