#include <string.h>
#include "ek_channel.h"
#include "ek_phasor.h"
#include "ek_arpeggiator.h"
#include "ek_tables.h"
#include "ek_adsr.h"

struct adsr_parameters_s {
    int32_t delta_a;
    int32_t value_d;
    int32_t delta_d;
    int32_t value_s; 
    int32_t value_r;
    int32_t delta_r;
};

struct ek_channel_s {
    uint8_t n_voices;
    uint8_t base_midi_note;
    int8_t octave;
    uint8_t vibrato;
    uint8_t i_arpeggiator_pattern;
    uint16_t arpeggio_duration;
    int16_t *table;
    int16_t resolution_mask;
    struct adsr_parameters_s envelope_parameters;
    uint8_t tremolo;
    uint8_t downsampling;
    uint8_t dry_volume;
    uint8_t wet_volume;
    ek_phasor_t phasors[N_VOICES];
    ek_arpeggiator_t arpeggiators[N_VOICES];
    int32_t phases[N_VOICES];
    ek_adsr_t envelopes[N_VOICES];
    int32_t last_sample;
};

static void change_voice_midi_note(ek_channel_t channel,uint8_t i_voice) {
    ek_phasor_actualize(channel->phasors[i_voice],channel->base_midi_note+12*channel->octave);
}

extern ek_channel_t ek_channel_create(uint8_t n_voices, uint8_t base_midi_note) {
    ek_channel_t channel = (ek_channel_t)malloc(sizeof(struct ek_channel_s));

    if (channel==NULL) {
        ESP_LOGI(TAG, "CHANNEL : Unable to create !!!");
        return channel;
    }
    // CHANNELS
    channel->base_midi_note = base_midi_note;
    channel->octave = 0;
    channel->n_voices = n_voices;
    channel->vibrato = 0;
    channel->i_arpeggiator_pattern = 0;
    channel->arpeggio_duration = SAMPLE_RATE;
    channel->table = tables[0];
    channel->resolution_mask = -1;
    channel->downsampling = 0;
    channel->envelope_parameters.delta_a = ADSR_MAXIMUM_VALUE/DMA_BUF_LEN;
    channel->envelope_parameters.value_d = DMA_BUF_LEN;
    channel->envelope_parameters.delta_d = 0;
    channel->envelope_parameters.value_s = ADSR_MAXIMUM_VALUE;
    channel->envelope_parameters.value_r = DMA_BUF_LEN;
    channel->envelope_parameters.delta_r = ADSR_MAXIMUM_VALUE/DMA_BUF_LEN;
    channel->tremolo = 0;
    channel->dry_volume = 200;
    channel->wet_volume = 100;
    channel->last_sample = 0;
    for (uint8_t i_voice=0; i_voice<N_VOICES; i_voice++) {
        channel->phases[i_voice] = 0;
        if (i_voice<channel->n_voices) {
            channel->phasors[i_voice] = ek_phasor_create();
            channel->arpeggiators[i_voice] = ek_arpeggiator_create();
            channel->envelopes[i_voice] = ek_adsr_create();
        }
        else {
            channel->phasors[i_voice] = NULL;
            channel->arpeggiators[i_voice] = NULL;
            channel->envelopes[i_voice] = NULL;
        }
    }
    return channel;
}

extern void ek_channel_change_table(ek_channel_t channel, int16_t *table) {
    channel->table = table;
}

extern void ek_channel_change_resolution_mask(ek_channel_t channel, int16_t resolution_mask) {
    channel->resolution_mask = resolution_mask;
}

extern void ek_channel_change_downsampling(ek_channel_t channel, uint8_t downsampling) {
    channel->downsampling = downsampling;
}

extern void ek_channel_change_octave(ek_channel_t channel, int8_t octave) {
    channel->octave = octave;
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        change_voice_midi_note(channel,i_voice);
    }
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

static int32_t slider_to_value(uint8_t slider) {
    return (((int32_t)slider)+1)<<6;
}

extern void ek_channel_change_envelope_a(ek_channel_t channel, uint8_t a) {
    channel->envelope_parameters.delta_a = ADSR_MAXIMUM_VALUE/slider_to_value(a);
}

extern void ek_channel_change_envelope_d(ek_channel_t channel, uint8_t d) {
    channel->envelope_parameters.value_d = slider_to_value(d);
    channel->envelope_parameters.delta_d = (ADSR_MAXIMUM_VALUE-channel->envelope_parameters.value_s)/channel->envelope_parameters.value_d;
}

extern void ek_channel_change_envelope_s(ek_channel_t channel, uint8_t s) {
    channel->envelope_parameters.value_s = ((int32_t)s)<<ADSR_SHIFT_VALUE;
    channel->envelope_parameters.delta_d = (ADSR_MAXIMUM_VALUE-channel->envelope_parameters.value_s)/channel->envelope_parameters.value_d;
    channel->envelope_parameters.delta_r = channel->envelope_parameters.value_s/channel->envelope_parameters.value_r;
}

extern void ek_channel_change_envelope_r(ek_channel_t channel, uint8_t r) {
    channel->envelope_parameters.value_r = slider_to_value(r);
    channel->envelope_parameters.delta_r = channel->envelope_parameters.value_s/channel->envelope_parameters.value_r;
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

static int8_t search_active_voice(ek_channel_t channel, int16_t i_button) {
    for (int8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (ek_phasor_get_note(channel->phasors[i_voice])==i_button) {
            return i_voice;
        }
    }
    return -1;
}

static int8_t search_inactive_voice(ek_channel_t channel) {
    for (int8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (ek_phasor_get_note(channel->phasors[i_voice])==-1 && ek_adsr_is_active(channel->envelopes[i_voice])==0) {
            return i_voice;
        }
    }
    return -1;
}

extern void ek_channel_button_on(ek_channel_t channel, int16_t i_button) {
    int8_t i_voice = search_active_voice(channel,i_button);
    if (i_voice==-1) {
        i_voice = search_inactive_voice(channel);
    }
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel : Button %d ON",i_button);
    ek_phasor_change_note(channel->phasors[i_voice],i_button);
    change_voice_midi_note(channel,i_voice);
}

extern void ek_channel_button_off(ek_channel_t channel, int16_t i_button) {
    int8_t i_voice = search_active_voice(channel,i_button);
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel : Button %d OFF",i_button);
    ek_phasor_change_note(channel->phasors[i_voice],-1);
}

static void compute_vibrato(ek_channel_t channel, int32_t *lfo_int32_buffer, int32_t *input_output_int32_buffer) {
    uint8_t vibrato = channel->vibrato;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        input_output_int32_buffer[i] += ((lfo_int32_buffer[i]*(input_output_int32_buffer[i]>>20))*vibrato)>>8;
    }
}

static void compute_voice(ek_channel_t channel,
    int8_t i_voice,
    int32_t *phase_increment_int32_buffer,
    int32_t *output_int32_buffer
) {
    uint32_t phase = channel->phases[i_voice];
    int16_t *table = channel->table;
    int16_t resolution_mask = channel->resolution_mask;
    
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output_int32_buffer[i] = (table[phase>>TABLE_PHASE_SHIFT]&resolution_mask);
        phase += phase_increment_int32_buffer[i];
    }
    channel->phases[i_voice] = phase;
}

extern void ek_channel_compute(
    ek_channel_t channel,
    int32_t *lfo_int32_buffer,
    int32_t *input_output_dry_int32_buffer,
    int32_t *input_output_wet_int32_buffer    
) {
    int32_t output[DMA_BUF_LEN], phase_increment[DMA_BUF_LEN], voice_output[DMA_BUF_LEN], envelope_output[DMA_BUF_LEN];
    
    uint8_t tremolo = channel->tremolo;
    uint8_t downsampling = channel->downsampling;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output[i] = 0;
    }
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        uint8_t on_off = ek_phasor_get_note(channel->phasors[i_voice])!=-1;
        if (on_off || ek_adsr_is_active(channel->envelopes[i_voice])) {
            ek_phasor_compute(channel->phasors[i_voice],phase_increment);
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
            compute_voice(channel,i_voice,phase_increment,voice_output);
            ek_adsr_compute(channel->envelopes[i_voice],on_off,
            channel->envelope_parameters.delta_a,
            channel->envelope_parameters.delta_d,
            channel->envelope_parameters.value_s,
            channel->envelope_parameters.delta_r,
            envelope_output);
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