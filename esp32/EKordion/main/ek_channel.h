#include "ek_config.h"
#include "ek_arpeggiator.h"

#ifndef EK_CHANNEL_H
#define EK_CHANNEL_H

typedef struct ek_channel_s *ek_channel_t;
ek_channel_t ek_channel_create(uint8_t n_voices, uint8_t base_midi_note);
void ek_channel_change_table(ek_channel_t channel, int16_t *table);
void ek_channel_change_resolution_mask(ek_channel_t channel, int16_t resolution_mask);
void ek_channel_change_downsampling(ek_channel_t channel, uint8_t downsampling);
void ek_channel_change_octave(ek_channel_t channel, int8_t octave);
void ek_channel_change_arpeggio_duration(ek_channel_t channel, uint16_t arpeggio_duration);
void ek_channel_change_arpeggiator_pattern(ek_channel_t channel, uint8_t i_pattern);
void ek_channel_change_vibrato(ek_channel_t channel, uint8_t vibrato);
void ek_channel_change_envelope_a(ek_channel_t channel, uint8_t a);
void ek_channel_change_envelope_d(ek_channel_t channel, uint8_t d);
void ek_channel_change_envelope_s(ek_channel_t channel, uint8_t s);
void ek_channel_change_envelope_r(ek_channel_t channel, uint8_t r);
void ek_channel_change_filter_low_f(ek_channel_t channel, uint8_t low_f_slider);
void ek_channel_change_filter_high_f(ek_channel_t channel, uint8_t high_f_slider);
void ek_channel_change_filter_q(ek_channel_t channel, uint8_t q_slider);
void ek_channel_change_filter_mod_a(ek_channel_t channel, uint8_t a);
void ek_channel_change_filter_mod_d(ek_channel_t channel, uint8_t d);
void ek_channel_change_filter_mod_s(ek_channel_t channel, uint8_t s);
void ek_channel_change_filter_mod_r(ek_channel_t channel, uint8_t r);
void ek_channel_change_tremolo(ek_channel_t channel, uint8_t tremolo);
void ek_channel_change_dry_volume(ek_channel_t channel, uint8_t volume);
void ek_channel_change_wet_volume(ek_channel_t channel, uint8_t volume);
//
void ek_channel_button_on(ek_channel_t channel, int16_t i_button);
void ek_channel_button_off(ek_channel_t channel, int16_t i_button);
void ek_channel_compute(    
    ek_channel_t channel,
    int32_t *lfo_int32_buffer,
    int32_t *input_output_dry_int32_buffer,
    int32_t *input_output_wet_int32_buffer    
);

#endif