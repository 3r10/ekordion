#include <string.h>
#include "ek_channel.h"
#include "ek_tables.h"
#include "ek_voice.h"

struct channel_s {
    uint8_t tremolo;
    uint8_t downsampling;
    uint8_t dry_volume;
    uint8_t wet_volume;
    uint8_t n_voices;
    voice_t voices[N_VOICES];
    int32_t output[DMA_BUF_LEN];
};

extern channel_t ek_channel_create(uint8_t n_voices, uint8_t base_midi_note) {
    channel_t channel = (channel_t)malloc(sizeof(struct channel_s));

    if (channel==NULL) {
        ESP_LOGI(TAG, "CHANNEL : Unable to create !!!");
        return channel;
    }
    // CHANNELS
    channel->n_voices = n_voices;
    channel->downsampling = 0;
    channel->tremolo = 0;
    channel->dry_volume = 200;
    channel->wet_volume = 100;
    for (uint8_t i_voice=0; i_voice<N_VOICES; i_voice++) {
        if (i_voice<channel->n_voices) {
            channel->voices[i_voice] = ek_voice_create(base_midi_note);
        }
        else {
            channel->voices[i_voice] = NULL;
        }
    }
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        channel->output[i] = 0;
    }
    return channel;
}

extern uint8_t ek_channel_get_dry_volume(channel_t channel) {
    return channel->dry_volume;
}

extern uint8_t ek_channel_get_wet_volume(channel_t channel) {
    return channel->wet_volume;
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
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        ek_voice_change_octave(channel->voices[i_voice],octave);
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
        if (ek_voice_get_i_button(channel->voices[i_voice])==i_button) {
            return i_voice;
        }
    }
    return -1;
}

static int8_t search_inactive_voice(channel_t channel) {
    for (int8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (ek_voice_get_i_button(channel->voices[i_voice])==-1 && ek_voice_get_ramp_step(channel->voices[i_voice])==0) {
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
    ek_voice_change_i_button(channel->voices[i_voice],i_button);
}

extern void ek_channel_button_off(channel_t channel, int16_t i_button) {
    int8_t i_voice = search_active_voice(channel,i_button);
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel : Button %d OFF",i_button);
    ek_voice_change_i_button(channel->voices[i_voice],-1);
}

extern int32_t *ek_channel_compute(channel_t channel,int32_t *lfo_int32_buffer) {
    int32_t *output = channel->output;
    int32_t last_sample = output[DMA_BUF_LEN-1];
    uint8_t tremolo = channel->tremolo;
    uint8_t downsampling = channel->downsampling;
    uint8_t downsampling_mask = DMA_BUF_LEN-1;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output[i] = 0;
    }
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        int32_t *voice_output = ek_voice_compute(channel->voices[i_voice],lfo_int32_buffer);
        if (voice_output!=NULL) {
            for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
                output[i] += voice_output[i];
            }
        }
    }
    // TREMOLO
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        int32_t factor = ((1<<24)+(lfo_int32_buffer[i]-(1<<15))*tremolo)>>16;
        output[i] = (output[i]*factor)>>8;
    }
    // DOWNSAMPLING
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        if (((i*downsampling)&downsampling_mask)<downsampling) {
            output[i] = last_sample;
        }
        last_sample = output[i];
    }
    // OUTPUT
    return output;
}