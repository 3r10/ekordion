#include <string.h>
#include "ek_channel.h"
#include "ek_monopoly.h"
#include "ek_arpeggiator.h"
#include "ek_oscillator.h"
#include "ek_tables.h"
#include "ek_adsr.h"
#include "ek_filter.h"

struct ek_channel_s {
    uint8_t n_voices;
    ek_monopoly_t monopoly;
    // DYNAMIC PARAMETERS (scalar)
    uint8_t vibrato;
    uint8_t i_arpeggiator_pattern;
    uint16_t arpeggio_duration;
    uint8_t tremolo;
    uint8_t downsampling;
    uint8_t dry_volume;
    uint8_t wet_volume;
    // DYNAMIC PARAMETERS (structured)
    ek_oscillator_parameters_t oscillator_parameters;
    ek_adsr_parameters_t envelope_parameters;
    ek_adsr_parameters_t filter_mod_parameters;
    ek_filter_parameters_t filter_parameters;
    // SIGNAL PROCESSING (arrays of structures)
    ek_arpeggiator_t *arpeggiators;
    ek_oscillator_t *oscillators;
    ek_adsr_t *envelopes;
    ek_adsr_t *filter_mods;
    ek_filter_t *filters;
    // STATE
    int32_t last_sample;
};

extern ek_channel_t ek_channel_create(uint8_t n_voices, uint8_t base_midi_note) {
    ek_channel_t channel = (ek_channel_t)malloc(sizeof(struct ek_channel_s));

    if (channel==NULL) {
        ESP_LOGI(TAG, "CHANNEL : Unable to create !!!");
        return channel;
    }
    // CHANNELS
    channel->n_voices = n_voices;
    channel->monopoly = ek_monopoly_create(n_voices,base_midi_note);
    channel->vibrato = 0;
    channel->i_arpeggiator_pattern = 0;
    channel->arpeggio_duration = SAMPLE_RATE;
    channel->oscillator_parameters = ek_oscillator_parameters_create();
    channel->envelope_parameters = ek_adsr_parameters_create();
    channel->filter_mod_parameters = ek_adsr_parameters_create();
    channel->downsampling = 0;
    channel->tremolo = 0;
    channel->filter_parameters = ek_filter_parameters_create();
    channel->dry_volume = 200;
    channel->wet_volume = 100;
    channel->last_sample = 0;
    // SIGNAL PROCESSING ARRAYS ALLOCATION
    channel->arpeggiators = (ek_arpeggiator_t *)malloc(n_voices*sizeof(ek_arpeggiator_t));
    channel->oscillators = (ek_oscillator_t *)malloc(n_voices*sizeof(ek_oscillator_t));
    channel->envelopes = (ek_adsr_t *)malloc(n_voices*sizeof(ek_adsr_t));
    channel->filter_mods = (ek_adsr_t *)malloc(n_voices*sizeof(ek_adsr_t));
    channel->filters = (ek_filter_t *)malloc(n_voices*sizeof(ek_filter_t));
    if (
        channel->arpeggiators==NULL || channel->oscillators==NULL
        || channel->envelopes==NULL || channel->filter_mods==NULL 
        || channel->filters==NULL
    ) {
        return NULL;
    }
    for (uint8_t i_voice=0; i_voice<n_voices; i_voice++) {
        channel->arpeggiators[i_voice] = ek_arpeggiator_create();
        channel->oscillators[i_voice] = ek_oscillator_create();
        channel->envelopes[i_voice] = ek_adsr_create();
        channel->filter_mods[i_voice] = ek_adsr_create();
        channel->filters[i_voice] = ek_filter_create();
    }
    return channel;
}

extern void ek_channel_change_table(ek_channel_t channel,  const int16_t *table) {
    ek_oscillator_parameters_change_table(channel->oscillator_parameters,table);
}

extern void ek_channel_change_resolution_mask(ek_channel_t channel, int16_t resolution_mask) {
    ek_oscillator_parameters_change_resolution_mask(channel->oscillator_parameters,resolution_mask);
}

extern void ek_channel_change_downsampling(ek_channel_t channel, uint8_t downsampling) {
    channel->downsampling = downsampling;
}

extern void ek_channel_change_octave(ek_channel_t channel, int8_t octave) {
    ek_monopoly_change_octave(channel->monopoly,octave);
}

extern void ek_channel_change_monopoly_mode(ek_channel_t channel, uint8_t mode) {
    ek_monopoly_change_mode(channel->monopoly,mode);
}

extern void ek_channel_change_n_oscillators(ek_channel_t channel, uint8_t n_oscillators) {
    ek_oscillator_parameters_change_n_oscillators(channel->oscillator_parameters,n_oscillators);
}

extern void ek_channel_change_detune_factor(ek_channel_t channel, uint8_t detune_factor) {
    ek_oscillator_parameters_change_detune_factor(channel->oscillator_parameters,detune_factor);
}

extern void ek_channel_change_arpeggio_duration(ek_channel_t channel, uint16_t arpeggio_duration) {
    channel->arpeggio_duration = arpeggio_duration;
}

extern void ek_channel_change_arpeggiator_pattern(ek_channel_t channel, uint8_t i_pattern) {
    channel->i_arpeggiator_pattern = i_pattern;
}

extern void ek_channel_change_vibrato(ek_channel_t channel, uint8_t vibrato) {
    channel->vibrato = vibrato;
}

extern void ek_channel_change_envelope_a(ek_channel_t channel, uint8_t a) {
    ek_adsr_parameters_change_a(channel->envelope_parameters,a);
}

extern void ek_channel_change_envelope_d(ek_channel_t channel, uint8_t d) {
    ek_adsr_parameters_change_d(channel->envelope_parameters,d);
}

extern void ek_channel_change_envelope_s(ek_channel_t channel, uint8_t s) {
    ek_adsr_parameters_change_s(channel->envelope_parameters,s);
}

extern void ek_channel_change_envelope_r(ek_channel_t channel, uint8_t r) {
    ek_adsr_parameters_change_r(channel->envelope_parameters,r);
}

extern void ek_channel_change_filter_low_f(ek_channel_t channel, uint8_t low_f_slider) {
    ek_filter_parameters_change_low_f(channel->filter_parameters,low_f_slider);
}

extern void ek_channel_change_filter_high_f(ek_channel_t channel, uint8_t high_f_slider) {
    ek_filter_parameters_change_high_f(channel->filter_parameters,high_f_slider);
}

extern void ek_channel_change_filter_q(ek_channel_t channel, uint8_t q_slider) {
    ek_filter_parameters_change_q(channel->filter_parameters,q_slider);
}

extern void ek_channel_change_filter_mod_a(ek_channel_t channel, uint8_t a) {
    ek_adsr_parameters_change_a(channel->filter_mod_parameters,a);
}

extern void ek_channel_change_filter_mod_d(ek_channel_t channel, uint8_t d) {
    ek_adsr_parameters_change_d(channel->filter_mod_parameters,d);
}

extern void ek_channel_change_filter_mod_s(ek_channel_t channel, uint8_t s) {
    ek_adsr_parameters_change_s(channel->filter_mod_parameters,s);
}

extern void ek_channel_change_filter_mod_r(ek_channel_t channel, uint8_t r) {
    ek_adsr_parameters_change_r(channel->filter_mod_parameters,r);
}

extern void ek_channel_change_tremolo(ek_channel_t channel, uint8_t tremolo) {
    channel->tremolo = tremolo;
}

extern void ek_channel_change_dry_volume(ek_channel_t channel, uint8_t volume) {
    channel->dry_volume = volume;
}

extern void ek_channel_change_wet_volume(ek_channel_t channel, uint8_t volume) {
    channel->wet_volume = volume;
}

extern void ek_channel_button_on(ek_channel_t channel, int16_t i_button) {
    ek_monopoly_button_on(channel->monopoly,i_button);
}

extern void ek_channel_button_off(ek_channel_t channel, int16_t i_button) {
    ek_monopoly_button_off(channel->monopoly,i_button);
}

static void compute_vibrato(ek_channel_t channel, int32_t *lfo_int32_buffer, int32_t *input_output_int32_buffer) {
    uint8_t vibrato = channel->vibrato;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        input_output_int32_buffer[i] += ((lfo_int32_buffer[i]*(input_output_int32_buffer[i]>>20))*vibrato)>>8;
    }
}

extern void ek_channel_compute(
    ek_channel_t channel,
    int32_t lfo_int32_buffer[DMA_BUF_LEN],
    int32_t input_output_dry_int32_buffer[DMA_BUF_LEN],
    int32_t input_output_wet_int32_buffer[DMA_BUF_LEN]
) {
    int32_t output[DMA_BUF_LEN], phase_increment[DMA_BUF_LEN], voice_output[DMA_BUF_LEN];
    int32_t envelope_output[DMA_BUF_LEN], filter_mod_output[DMA_BUF_LEN];
    
    uint8_t tremolo = channel->tremolo;
    uint8_t downsampling = channel->downsampling;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output[i] = 0;
    }
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (ek_monopoly_is_voice_active(channel->monopoly,i_voice)) {
            uint32_t increment = ek_monopoly_get_voice_phase_increment(channel->monopoly,i_voice);
            for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
                phase_increment[i] = increment;
            }
            if (channel->vibrato) {
                compute_vibrato(channel,lfo_int32_buffer,phase_increment);
            }
            if (channel->i_arpeggiator_pattern) {
                ek_arpeggiator_compute(
                    channel->arpeggiators[i_voice], 
                    channel->i_arpeggiator_pattern, 
                    channel->arpeggio_duration,
                    phase_increment
                );
            }
            ek_oscillator_compute(
                channel->oscillators[i_voice],
                channel->oscillator_parameters,
                phase_increment,voice_output
            );
            uint8_t is_on = ek_monopoly_is_voice_on(channel->monopoly,i_voice);
            uint8_t envelope_state = ek_adsr_compute(
                channel->envelopes[i_voice],is_on,
                channel->envelope_parameters,
                envelope_output
            );
            uint8_t filter_mod_state = ek_adsr_compute(
                channel->filter_mods[i_voice],is_on,
                channel->filter_mod_parameters,
                filter_mod_output
            );
            if (envelope_state==ADSR_STATE_IDLE && filter_mod_state==ADSR_STATE_IDLE) {
                ek_monopoly_deactivate_voice(channel->monopoly,i_voice);
            }
            ek_dynamic_filter_compute(channel->filters[i_voice], channel->filter_parameters, filter_mod_output, voice_output);
            for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
                output[i] += (voice_output[i]*(envelope_output[i]>>16))>>9;
            }
        }
    }
    // TREMOLO
    if (tremolo) {
        for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
            int32_t factor = ((1<<24)+(lfo_int32_buffer[i]-(1<<15))*tremolo)>>16;
            output[i] = (output[i]*factor)>>8;
        }
    }
    // DOWNSAMPLING
    if (downsampling) {
        uint8_t downsampling_mask = DMA_BUF_LEN-1;
        int32_t last_sample = channel->last_sample;
        for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
            if (((i*downsampling)&downsampling_mask)<downsampling) {
                output[i] = last_sample;
            }
            last_sample = output[i];
        }
        channel->last_sample = last_sample;
    }
    // OUTPUT
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        input_output_dry_int32_buffer[i] += output[i]*channel->dry_volume;
        input_output_wet_int32_buffer[i] += output[i]*channel->wet_volume;
    }
}