#include <string.h>
#include "ek_voices.h"
#include "ek_tables.h"

#define N_RAMP_STEPS 32 // ~ 1 ms ramp

typedef struct lfo_s {
    uint32_t phase_increment;
    uint32_t phase;
    int16_t *table;
} lfo_t;

typedef struct channel_s {
    uint8_t base_midi_note;
    int8_t octave;
    int16_t *table;
    uint8_t vibrato;
    uint8_t volume;
    uint8_t i_voice_start;
    uint8_t i_voice_end;
} channel_t;

typedef struct voice_s {
    int16_t i_button;
    uint32_t phase_increment;
    uint32_t phase;
    uint16_t ramp_step;
    channel_t *channel;
} voice_t;

static lfo_t lfo;
static channel_t channels[N_CHANNELS];
static voice_t voices[N_VOICES];

static void compute_phase_increment(voice_t *voice) {
    uint8_t midi_note = voice->channel->base_midi_note+12*voice->channel->octave+voice->i_button;
    voice->phase_increment = 21500000*pow(2.0f,(midi_note-69)/12.0f);
}

extern void ek_voices_init() {
    // LFO
    lfo.phase_increment = 0;
    lfo.phase = 0;
    lfo.table = tables[1]; // Sine wave
    // CHANNELS
    channels[BASS_CHANNEL].base_midi_note = BASS_BASE_MIDI_NOTE;
    channels[CHORDS_CHANNEL].base_midi_note = CHORDS_BASE_MIDI_NOTE;
    channels[LEAD_CHANNEL].base_midi_note = LEAD_BASE_MIDI_NOTE;
    channels[BASS_CHANNEL].vibrato = 0;
    channels[CHORDS_CHANNEL].vibrato = 0;
    channels[LEAD_CHANNEL].vibrato = 50;
    channels[BASS_CHANNEL].i_voice_start = 0;
    channels[BASS_CHANNEL].i_voice_end = 2;
    channels[CHORDS_CHANNEL].i_voice_start = 2;
    channels[CHORDS_CHANNEL].i_voice_end = 4;
    channels[LEAD_CHANNEL].i_voice_start = 4;
    channels[LEAD_CHANNEL].i_voice_end = N_VOICES;
    for (int i_channel = 0; i_channel<N_CHANNELS; i_channel++) {
        uint8_t i_start = channels[i_channel].i_voice_start;
        uint8_t i_end = channels[i_channel].i_voice_end;
        channels[i_channel].table = tables[0];
        channels[i_channel].volume = 200;
        channels[i_channel].octave = 0;
        for (int i_voice=i_start; i_voice<i_end; i_voice++) {
            voices[i_voice].channel = &channels[i_channel];
        }
    }
    // VOICES
    for (int i_voice=0; i_voice<N_VOICES; i_voice++) {
        voices[i_voice].i_button = -1;
        voices[i_voice].phase_increment = 0;
        voices[i_voice].phase = 0;
        voices[i_voice].ramp_step = 0;
    }
}

extern void ek_voices_change_custom_table(uint16_t length, uint8_t *data) {
    if (length!=2*CHANGE_TABLE_OFFSET+1) return;
    uint16_t offset = ((uint16_t)data[0])*CHANGE_TABLE_OFFSET;
    memcpy(tables[0]+offset,data+1,CHANGE_TABLE_OFFSET*sizeof(uint16_t));
}

extern void ek_voices_change_volume(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    channels[i_channel].volume = data[1];
}

extern void ek_voices_change_octave(uint16_t length, uint8_t *data) {
    uint8_t i_channel,i_start,i_end;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>4) return;
    channels[i_channel].octave = ((int8_t)data[1])-2;
    i_start = channels[i_channel].i_voice_start;
    i_end = channels[i_channel].i_voice_end;
    for (int i_voice=i_start; i_voice<i_end; i_voice++) {
        compute_phase_increment(&voices[i_voice]);
    }
}

extern void ek_voices_change_table(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=N_TABLES) return;
    channels[i_channel].table = tables[data[1]];
}

extern void ek_voices_change_lfo_frequency(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    lfo.phase_increment = 3820*data[0];
}

extern void ek_voices_change_lfo_table(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    if (data[0]>=N_TABLES) return;
    lfo.table = tables[data[0]];
}

extern void ek_voices_change_vibrato(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    channels[i_channel].vibrato = data[1];
}

static int search_active_voice(int i_channel, int i_button) {
    uint8_t i_start = channels[i_channel].i_voice_start;
    uint8_t i_end = channels[i_channel].i_voice_end;
    for (int i_voice=i_start; i_voice<i_end; i_voice++) {
        if (voices[i_voice].i_button==i_button) {
            return i_voice;
        }
    }
    return -1;
}

static int search_inactive_voice(int i_channel) {
    uint8_t i_start = channels[i_channel].i_voice_start;
    uint8_t i_end = channels[i_channel].i_voice_end;
    for (int i_voice=i_start; i_voice<i_end; i_voice++) {
        if (voices[i_voice].i_button==-1 && voices[i_voice].ramp_step==0) {
            return i_voice;
        }
    }
    return -1;
}

extern void button_on(int i_channel,int i_button) {
    int i_voice = search_active_voice(i_channel,i_button);
    if (i_voice==-1) {
        i_voice = search_inactive_voice(i_channel);
    }
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel %d : Button %d ON",i_channel,i_button);
    voices[i_voice].i_button = i_button;
    compute_phase_increment(&voices[i_voice]);
}

extern void button_off(int i_channel,int i_button) {
    int i_voice = search_active_voice(i_channel,i_button);
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Channel %d : Button %d OFF",i_channel,i_button);
    voices[i_voice].i_button = -1;
}

extern void ek_voices_compute(int32_t output_int32_buffer[DMA_BUF_LEN]) {
    int16_t lfo_buffer[DMA_BUF_LEN];
    channel_t *channel;
    uint32_t phase = 0, phase_increment = 0;
    uint8_t on_off,vibrato,volume;
    const int16_t *table;
    uint16_t ramp_step;

    phase_increment = lfo.phase_increment;
    phase = lfo.phase;
    table = lfo.table;
    for (int i=0; i<DMA_BUF_LEN; i++) {
        output_int32_buffer[i] = 0;
        lfo_buffer[i] = table[phase>>TABLE_PHASE_SHIFT];
        phase += phase_increment;
    }
    lfo.phase = phase;
    for (int i_voice=0; i_voice<N_VOICES; i_voice++) {
        channel = voices[i_voice].channel;
        on_off = voices[i_voice].i_button!=-1;
        ramp_step = voices[i_voice].ramp_step;
        volume = channel->volume;
        if (on_off || ramp_step) {
            phase_increment = voices[i_voice].phase_increment;
            phase = voices[i_voice].phase;
            table = channel->table;
            vibrato = channel->vibrato;
            for (int i=0; i<DMA_BUF_LEN; i++) {
                if (on_off && ramp_step<N_RAMP_STEPS) {
                    ramp_step++;
                }
                if (!on_off && ramp_step>0) {
                    ramp_step--;
                }
                output_int32_buffer[i] += table[phase>>TABLE_PHASE_SHIFT]*ramp_step*volume>>10;
                phase += phase_increment+(((lfo_buffer[i]*(int32_t)(phase_increment>>20))*vibrato)>>8);
            }
            voices[i_voice].ramp_step = ramp_step;
            voices[i_voice].phase = phase;
        }
    }
}