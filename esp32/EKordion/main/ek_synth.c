#include <string.h>
#include "ek_tables.h"
#include "ek_lfo.h"
#include "ek_arpeggiator.h"
#include "ek_voice.h"
#include "ek_synth.h"


typedef struct channel_s {
    uint8_t base_midi_note;
    int8_t octave;
    int16_t *table;
    int16_t resolution_mask;
    uint8_t downsampling;
    uint16_t arpeggio_duration; // in samples
    arpeggiator_t arpeggiator;
    uint8_t vibrato;
    uint8_t tremolo;
    uint8_t dry_volume;
    uint8_t wet_volume;
    uint8_t n_voices;
    voice_t voices[N_VOICES];
} channel_t;


static lfo_t lfo;
static channel_t channels[N_CHANNELS];

extern void ek_synth_init() {
    // LFO
    lfo = ek_lfo_create();
    // CHANNELS
    channels[BASS_CHANNEL].base_midi_note = BASS_BASE_MIDI_NOTE;
    channels[CHORDS_CHANNEL].base_midi_note = CHORDS_BASE_MIDI_NOTE;
    channels[LEAD_CHANNEL].base_midi_note = LEAD_BASE_MIDI_NOTE;
    channels[BASS_CHANNEL].n_voices = 2;
    channels[CHORDS_CHANNEL].n_voices = 2;
    channels[LEAD_CHANNEL].n_voices = N_VOICES;
    for (int i_channel = 0; i_channel<N_CHANNELS; i_channel++) {
        channel_t *channel = &channels[i_channel];
        channel->table = tables[0];
        channel->resolution_mask = -1;
        channel->downsampling = 0;
        channel->arpeggiator = ek_arpeggiator_get(0);
        channel->arpeggio_duration = SAMPLE_RATE;
        channel->vibrato = 0;
        channel->tremolo = 0;
        channel->dry_volume = 200;
        channel->wet_volume = 100;
        channel->octave = 0;
        for (uint8_t i_voice=0; i_voice<N_VOICES; i_voice++) {
            if (i_voice<channel->n_voices) {
                channel->voices[i_voice] = ek_voice_create();
            }
            else {
                channel->voices[i_voice] = NULL;
            }
        }
    }
}

extern void ek_synth_change_custom_table(uint16_t length, uint8_t *data) {
    if (length!=2*CHANGE_TABLE_OFFSET+1) return;
    uint16_t offset = ((uint16_t)data[0])*CHANGE_TABLE_OFFSET;
    memcpy(tables[0]+offset,data+1,CHANGE_TABLE_OFFSET*sizeof(uint16_t));
}

extern void ek_synth_change_lfo_frequency(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    ek_lfo_change_phase_increment(lfo,3820*(uint32_t)data[0]);
}

extern void ek_synth_change_lfo_table(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    if (data[0]>=N_TABLES) return;
    ek_lfo_change_table(lfo,tables[data[0]]);
}

extern void ek_synth_change_table(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=N_TABLES) return;
    channels[i_channel].table = tables[data[1]];
}

extern void ek_synth_change_resolution(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>15) return;
    channels[i_channel].resolution_mask = ~((1<<data[1])-1);
}

extern void ek_synth_change_downsampling(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=DMA_BUF_LEN) return;
    channels[i_channel].downsampling = data[1];
}

extern void ek_synth_change_octave(uint16_t length, uint8_t *data) {
    channel_t *channel;
    
    if (length!=2) return;
    if (data[0]>=N_CHANNELS) return;
    if (data[1]>4) return;
    channel = &channels[data[0]];
    channel->octave = ((int8_t)data[1])-2;
    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        ek_voice_change_phase_increment(
            channel->voices[i_voice],
            channel->base_midi_note,
            channel->octave
        );
    }
}

extern void ek_synth_change_arpeggio_duration(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    channels[i_channel].arpeggio_duration = (1+(uint16_t)data[1])<<4;
}

extern void ek_synth_change_arpeggiator(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=N_ARPEGGIATORS) return;
    channels[i_channel].arpeggiator = ek_arpeggiator_get(data[1]);
}

extern void ek_synth_change_vibrato(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    channels[i_channel].vibrato = data[1];
}

extern void ek_synth_change_tremolo(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    channels[i_channel].tremolo = data[1];
}

extern void ek_synth_change_dry_volume(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    channels[i_channel].dry_volume = data[1];
}

extern void ek_synth_change_wet_volume(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    channels[i_channel].wet_volume = data[1];
}

static int search_active_voice(int i_channel, int i_button) {
    channel_t *channel = &channels[i_channel];

    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (ek_voice_get_i_button(channel->voices[i_voice])==i_button) {
            return i_voice;
        }
    }
    return -1;
}

static int search_inactive_voice(int i_channel) {
    channel_t *channel = &channels[i_channel];

    for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
        if (ek_voice_get_i_button(channel->voices[i_voice])==-1 && ek_voice_get_ramp_step(channel->voices[i_voice])==0) {
            return i_voice;
        }
    }
    return -1;
}

extern void button_on(int i_channel,int i_button) {
    channel_t *channel = &channels[i_channel];
    int i_voice = search_active_voice(i_channel,i_button);
    if (i_voice==-1) {
        i_voice = search_inactive_voice(i_channel);
    }
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel %d : Button %d ON",i_channel,i_button);
    ek_voice_set_i_button(channel->voices[i_voice],i_button);
    ek_voice_change_phase_increment(channel->voices[i_voice],channel->base_midi_note,channel->octave);
}

extern void button_off(int i_channel,int i_button) {
    channel_t *channel = &channels[i_channel];
    int i_voice = search_active_voice(i_channel,i_button);
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel %d : Button %d OFF",i_channel,i_button);
    ek_voice_set_i_button(channel->voices[i_voice],-1);
}

static void compute_voices(int32_t *lfo_int32_buffer,int32_t output_int32_buffers[N_CHANNELS][DMA_BUF_LEN]) {
    for (uint8_t i_channel=0; i_channel<N_CHANNELS; i_channel++) {
        channel_t *channel = &channels[i_channel];
        for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
            output_int32_buffers[i_channel][i] = 0;
        }
        for (uint8_t i_voice=0; i_voice<channel->n_voices; i_voice++) {
            int32_t *voice_output = ek_voice_compute(
                channel->voices[i_voice],
                lfo_int32_buffer,
                channel->table,channel->arpeggio_duration,
                channel->arpeggiator,
                channel->resolution_mask,channel->vibrato
            );
            if (voice_output!=NULL) {
                for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
                    output_int32_buffers[i_channel][i] += voice_output[i];
                }
            }
        }
    }
}

static void compute_tremolo(int32_t *lfo_int32_buffer,int32_t int32_buffers[N_CHANNELS][DMA_BUF_LEN]) {
    for (uint8_t i_channel=0; i_channel<N_CHANNELS; i_channel++) {
        uint8_t tremolo = channels[i_channel].tremolo;
        for (uint8_t i=0; i<DMA_BUF_LEN; i++) {
            int32_t factor = ((1<<24)+(lfo_int32_buffer[i]-(1<<15))*tremolo)>>16;
            int32_buffers[i_channel][i] = (int32_buffers[i_channel][i]*factor)>>8;
        }
    }
}

static void compute_downsampling(int32_t int32_buffers[N_CHANNELS][DMA_BUF_LEN]) {
    static int32_t last_samples[N_CHANNELS] = {0};
    uint8_t mask = DMA_BUF_LEN-1;

    for (uint8_t i_channel=0; i_channel<N_CHANNELS; i_channel++) {
        int32_t *buffer = int32_buffers[i_channel];
        uint8_t downsampling = channels[i_channel].downsampling;
        for (uint8_t i=0; i<DMA_BUF_LEN; i++) {
            if (((i*downsampling)&mask)<downsampling) {
                buffer[i] = last_samples[i_channel];
            }
            last_samples[i_channel] = buffer[i];
        }
    }
}

extern void ek_channels_compute(
    int32_t output_dry_int32_buffer[DMA_BUF_LEN],
    int32_t output_wet_int32_buffer[DMA_BUF_LEN]
) {
    int32_t *lfo_int32_buffer;
    int32_t output_int32_buffers[N_CHANNELS][DMA_BUF_LEN];

    // LFO
    lfo_int32_buffer = ek_lfo_compute(lfo);
    // CHANNELS BEFORE EFFECTS
    compute_voices(lfo_int32_buffer,output_int32_buffers);
    // EFFECTS
    compute_tremolo(lfo_int32_buffer,output_int32_buffers);
    compute_downsampling(output_int32_buffers);
    // FINAL MIX
    for (uint8_t i=0; i<DMA_BUF_LEN; i++) {
        output_dry_int32_buffer[i] = 0;
        output_wet_int32_buffer[i] = 0;
        for (uint8_t i_channel=0; i_channel<N_CHANNELS; i_channel++) {
            output_dry_int32_buffer[i] += output_int32_buffers[i_channel][i]*channels[i_channel].dry_volume;
            output_wet_int32_buffer[i] += output_int32_buffers[i_channel][i]*channels[i_channel].wet_volume;
        }
    }
}