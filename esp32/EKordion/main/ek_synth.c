#include <string.h>
#include "ek_tables.h"
#include "ek_lfo.h"
#include "ek_synth.h"

#define N_RAMP_STEPS 32 // ~ 1 ms ramp

typedef struct arpeggiator_s {
    uint8_t length;
    uint8_t factors[ARPEGGIATOR_MAX_LENTGH];
    uint8_t shifts[ARPEGGIATOR_MAX_LENTGH];
} arpeggiator_t;

typedef struct channel_s {
    uint8_t base_midi_note;
    int8_t octave;
    int16_t *table;
    int16_t resolution_mask;
    uint8_t downsampling;
    uint16_t arpeggio_duration; // in samples
    arpeggiator_t *arpeggiator;
    uint8_t vibrato;
    uint8_t tremolo;
    uint8_t dry_volume;
    uint8_t wet_volume;
    uint8_t i_voice_start;
    uint8_t i_voice_end;
} channel_t;

typedef struct voice_s {
    int16_t i_button;
    uint32_t phase_increment;
    uint32_t phase;
    uint16_t ramp_step;
    uint16_t arpeggio_tick;
    uint8_t arpeggio_step;
    uint8_t i_channel;
} voice_t;

static lfo_t lfo;

#define N_ARPEGGIATORS 8
const arpeggiator_t arpeggiator00 = { .length = 1, .factors = {1}, .shifts = {0}};
const arpeggiator_t arpeggiator01 = { .length = 2, .factors = {1, 2}, .shifts = {0, 0}};
const arpeggiator_t arpeggiator02 = { .length = 2, .factors = {1, 3}, .shifts = {0, 1}};
const arpeggiator_t arpeggiator03 = { .length = 3, .factors = {1, 2, 4}, .shifts = {0, 0, 0}};
const arpeggiator_t arpeggiator04 = { .length = 3, .factors = {1, 3, 2}, .shifts = {0, 1, 0}};
const arpeggiator_t arpeggiator05 = { .length = 4, .factors = {1, 3, 2, 3}, .shifts = {0, 1, 0, 1}};
const arpeggiator_t arpeggiator06 = { .length = 4, .factors = {1, 3, 2, 3}, .shifts = {0, 1, 0, 0}};
const arpeggiator_t arpeggiator07 = { .length = 4, .factors = {1, 5, 3, 2}, .shifts = {0, 2, 1, 0}};

const arpeggiator_t arpeggiators[N_ARPEGGIATORS] = {
    arpeggiator00, arpeggiator01, arpeggiator02, arpeggiator03, arpeggiator04, arpeggiator05, arpeggiator06, arpeggiator07
};
static channel_t channels[N_CHANNELS];
static voice_t voices[N_VOICES];


static void compute_phase_increment(voice_t *voice) {
    channel_t *channel = &channels[voice->i_channel];
    uint8_t midi_note =channel->base_midi_note+12*channel->octave+voice->i_button;
    voice->phase_increment = 21500000*pow(2.0f,(midi_note-69)/12.0f);
}

extern void ek_synth_init() {
    // LFO
    lfo = ek_lfo_create();
    // CHANNELS
    channels[BASS_CHANNEL].base_midi_note = BASS_BASE_MIDI_NOTE;
    channels[CHORDS_CHANNEL].base_midi_note = CHORDS_BASE_MIDI_NOTE;
    channels[LEAD_CHANNEL].base_midi_note = LEAD_BASE_MIDI_NOTE;
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
        channels[i_channel].resolution_mask = -1;
        channels[i_channel].downsampling = 0;
        channels[i_channel].arpeggiator = &arpeggiators[0];
        channels[i_channel].arpeggio_duration = SAMPLE_RATE;
        channels[i_channel].vibrato = 0;
        channels[i_channel].tremolo = 0;
        channels[i_channel].dry_volume = 200;
        channels[i_channel].wet_volume = 100;
        channels[i_channel].octave = 0;
        for (int i_voice=i_start; i_voice<i_end; i_voice++) {
            voices[i_voice].i_channel = i_channel;
        }
    }
    // VOICES
    for (int i_voice=0; i_voice<N_VOICES; i_voice++) {
        voices[i_voice].i_button = -1;
        voices[i_voice].phase_increment = 0;
        voices[i_voice].phase = 0;
        voices[i_voice].ramp_step = 0;
        voices[i_voice].arpeggio_tick = 0;
        voices[i_voice].arpeggio_step = 0;
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
    channels[i_channel].arpeggiator = &(arpeggiators[data[1]]);
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

static void compute_voices(int32_t *lfo_int32_buffer,int32_t output_int32_buffers[N_CHANNELS][DMA_BUF_LEN]) {
    uint8_t i_channel;
    channel_t *channel;
    arpeggiator_t *arpeggiator;
    uint32_t phase = 0, phase_increment = 0;
    uint8_t on_off,vibrato, arpeggio_step, factor, shift;
    int16_t resolution_mask;
    const int16_t *table;
    uint16_t ramp_step, arpeggio_tick, arpeggio_duration;

    for (int i_channel=0; i_channel<N_CHANNELS; i_channel++) {
        for (int i=0; i<DMA_BUF_LEN; i++) {
            output_int32_buffers[i_channel][i] = 0;
        }
    }
    for (int i_voice=0; i_voice<N_VOICES; i_voice++) {
        on_off = voices[i_voice].i_button!=-1;
        ramp_step = voices[i_voice].ramp_step;
        if (on_off || ramp_step) {
            i_channel = voices[i_voice].i_channel;
            channel = &channels[i_channel];
            arpeggio_duration = channel->arpeggio_duration;
            arpeggio_tick = voices[i_voice].arpeggio_tick;
            arpeggio_step = voices[i_voice].arpeggio_step;
            arpeggiator = channel->arpeggiator;
            factor = arpeggiator->factors[arpeggio_step];
            shift = arpeggiator->shifts[arpeggio_step];
            phase_increment = voices[i_voice].phase_increment;
            phase = voices[i_voice].phase;
            table = channel->table;
            resolution_mask = channel->resolution_mask;
            vibrato = channel->vibrato;
            for (int i=0; i<DMA_BUF_LEN; i++) {
                if (on_off && ramp_step<N_RAMP_STEPS) {
                    ramp_step++;
                }
                if (!on_off && ramp_step>0) {
                    ramp_step--;
                }
                arpeggio_tick++;
                if (arpeggio_tick>=arpeggio_duration) {
                    arpeggio_step = (arpeggio_step+1)%arpeggiator->length;
                    factor = arpeggiator->factors[arpeggio_step];
                    shift = arpeggiator->shifts[arpeggio_step];
                    arpeggio_tick = 0;
                }
                output_int32_buffers[i_channel][i] += (table[phase>>TABLE_PHASE_SHIFT]&resolution_mask)*ramp_step;
                phase += ((phase_increment+(((lfo_int32_buffer[i]*(int32_t)(phase_increment>>20))*vibrato)>>8))*factor)>>shift;
            }
            voices[i_voice].phase = phase;
            voices[i_voice].ramp_step = ramp_step;
            voices[i_voice].arpeggio_tick = arpeggio_tick;
            voices[i_voice].arpeggio_step = arpeggio_step;
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