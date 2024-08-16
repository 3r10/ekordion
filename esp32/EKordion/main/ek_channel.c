#include <string.h>
#include "ek_channel.h"
#include "ek_tables.h"
#include "ek_voice.h"
#include "ek_adsr.h"

struct channel_s {
    uint8_t base_midi_note;
    int8_t octave;
    int16_t i_buttons[N_VOICES];
    uint8_t tremolo;
    uint8_t downsampling;
    uint8_t dry_volume;
    uint8_t wet_volume;
    uint8_t n_voices;
    voice_t voices[N_VOICES];
    ek_adsr_t envelopes[N_VOICES];
    int32_t last_sample;
};

static void change_voice_midi_note(channel_t channel,uint8_t i_voice) {
    ek_voice_change_midi_note(
        channel->voices[i_voice],
        channel->base_midi_note+12*channel->octave+channel->i_buttons[i_voice]
    );
}

extern channel_t ek_channel_create(uint8_t n_voices, uint8_t base_midi_note) {
    channel_t channel = (channel_t)malloc(sizeof(struct channel_s));

    if (channel==NULL) {
        ESP_LOGI(TAG, "CHANNEL : Unable to create !!!");
        return channel;
    }
    // CHANNELS
    channel->base_midi_note = base_midi_note;
    channel->octave = 0;
    channel->n_voices = n_voices;
    channel->downsampling = 0;
    channel->tremolo = 0;
    channel->dry_volume = 200;
    channel->wet_volume = 100;
    channel->last_sample = 0;
    for (uint8_t i_voice=0; i_voice<N_VOICES; i_voice++) {
        channel->i_buttons[i_voice] = -1;
        if (i_voice<channel->n_voices) {
            channel->voices[i_voice] = ek_voice_create();
            channel->envelopes[i_voice] = ek_adsr_create();
        }
        else {
            channel->voices[i_voice] = NULL;
            channel->envelopes[i_voice] = NULL;
        }
    }
    return channel;
}

extern void ek_channel_change_table(channel_t channel, int16_t *table) {
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        ek_voice_change_table(channel->voices[i_voice],table);
    }
}

extern void ek_channel_change_resolution_mask(channel_t channel, int16_t resolution_mask) {
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        ek_voice_change_resolution_mask(channel->voices[i_voice],resolution_mask);
    }
}

extern void ek_channel_change_downsampling(channel_t channel, uint8_t downsampling) {
    channel->downsampling = downsampling;
}

extern void ek_channel_change_octave(channel_t channel, int8_t octave) {
    channel->octave = octave;
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        change_voice_midi_note(channel,i_voice);
    }
}

extern void ek_channel_change_arpeggio_duration(channel_t channel, uint16_t arpeggio_duration) {
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        ek_voice_change_arpeggio_duration(channel->voices[i_voice],arpeggio_duration);
    }
}

extern void ek_channel_change_arpeggiator(channel_t channel, arpeggiator_t arpeggiator) {
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        ek_voice_change_arpeggiator(channel->voices[i_voice],arpeggiator);
    }
}

extern void ek_channel_change_vibrato(channel_t channel, uint8_t vibrato) {
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        ek_voice_change_vibrato(channel->voices[i_voice],vibrato);
    }
}

extern void ek_channel_change_tremolo(channel_t channel, uint8_t tremolo) {
    channel->tremolo = tremolo;
}

extern void ek_channel_change_dry_volume(channel_t channel, uint8_t volume) {
    channel->dry_volume = volume;
}

extern void ek_channel_change_wet_volume(channel_t channel, uint8_t volume) {
    channel->wet_volume = volume;
}

static int8_t search_active_voice(channel_t channel, int16_t i_button) {
    for (int8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (channel->i_buttons[i_voice]==i_button) {
            return i_voice;
        }
    }
    return -1;
}

static int8_t search_inactive_voice(channel_t channel) {
    for (int8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (channel->i_buttons[i_voice]==-1 && ek_adsr_is_active(channel->envelopes[i_voice])==0) {
            return i_voice;
        }
    }
    return -1;
}

extern void ek_channel_button_on(channel_t channel, int16_t i_button) {
    int8_t i_voice = search_active_voice(channel,i_button);
    if (i_voice==-1) {
        i_voice = search_inactive_voice(channel);
    }
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel : Button %d ON",i_button);
    channel->i_buttons[i_voice] = i_button;
    change_voice_midi_note(channel,i_voice);
}

extern void ek_channel_button_off(channel_t channel, int16_t i_button) {
    int8_t i_voice = search_active_voice(channel,i_button);
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel : Button %d OFF",i_button);
    channel->i_buttons[i_voice] = -1;
}

extern void ek_channel_compute(
    channel_t channel,
    int32_t *lfo_int32_buffer,
    int32_t *input_output_dry_int32_buffer,
    int32_t *input_output_wet_int32_buffer    
) {
    int32_t output[DMA_BUF_LEN], voice_output[DMA_BUF_LEN], envelope_output[DMA_BUF_LEN];
    
    uint8_t tremolo = channel->tremolo;
    uint8_t downsampling = channel->downsampling;
    uint8_t downsampling_mask = DMA_BUF_LEN-1;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output[i] = 0;
    }
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        uint8_t on_off = channel->i_buttons[i_voice]!=-1;
        if (on_off || ek_adsr_is_active(channel->envelopes[i_voice])) {
            ek_voice_compute(channel->voices[i_voice],on_off,lfo_int32_buffer,voice_output);
            ek_adsr_compute(channel->envelopes[i_voice],on_off,envelope_output);
            for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
                output[i] += voice_output[i]*envelope_output[i];
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