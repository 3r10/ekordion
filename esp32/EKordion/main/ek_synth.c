#include <string.h>
#include "ek_tables.h"
#include "ek_lfo.h"
#include "ek_arpeggiator.h"
#include "ek_voice.h"
#include "ek_channel.h"
#include "ek_synth.h"

static lfo_t lfo;
static channel_t channels[N_CHANNELS];

extern void ek_synth_init() {
    // LFO
    lfo = ek_lfo_create();
    // CHANNELS
    channels[BASS_CHANNEL] = ek_channel_create(2,BASS_BASE_MIDI_NOTE);
    channels[CHORDS_CHANNEL] = ek_channel_create(2,CHORDS_BASE_MIDI_NOTE);
    channels[LEAD_CHANNEL] = ek_channel_create(N_VOICES,LEAD_BASE_MIDI_NOTE);
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
    ek_channel_change_table(channels[i_channel],tables[data[1]]);
}

extern void ek_synth_change_resolution(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>15) return;
    ek_channel_change_resolution(channels[i_channel],data[1]);
}

extern void ek_synth_change_downsampling(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=DMA_BUF_LEN) return;
    ek_channel_change_downsampling(channels[i_channel],data[1]);
}

extern void ek_synth_change_octave(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>4) return;
    ek_channel_change_octave(channels[i_channel],((int8_t)data[1])-2);
}

extern void ek_synth_change_arpeggio_duration(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_arpeggio_duration(channels[i_channel],(1+(uint16_t)data[1])<<4);
}

extern void ek_synth_change_arpeggiator(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=N_ARPEGGIATORS) return;
    ek_channel_change_arpeggiator(channels[i_channel],ek_arpeggiator_get(data[1]));
}

extern void ek_synth_change_vibrato(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_vibrato(channels[i_channel],data[1]);
}

extern void ek_synth_change_tremolo(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_tremolo(channels[i_channel],data[1]);
}

extern void ek_synth_change_dry_volume(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_dry_volume(channels[i_channel],data[1]);
}

extern void ek_synth_change_wet_volume(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_wet_volume(channels[i_channel],data[1]);
}

extern void ek_synth_button_on(uint8_t i_channel,int16_t i_button) {
    ek_channel_button_on(channels[i_channel],i_button);
}

extern void ek_synth_button_off(uint8_t i_channel,int16_t i_button) {
    ek_channel_button_off(channels[i_channel],i_button);
}

extern void ek_synth_compute(
    int32_t output_dry_int32_buffer[DMA_BUF_LEN],
    int32_t output_wet_int32_buffer[DMA_BUF_LEN]
) {
    int32_t *lfo_int32_buffer;
    int32_t *channel_outputs[N_CHANNELS];

    // LFO
    lfo_int32_buffer = ek_lfo_compute(lfo);
    for (uint8_t i_channel=0; i_channel<N_CHANNELS; i_channel++) {
        channel_outputs[i_channel] = ek_channel_compute(channels[i_channel],lfo_int32_buffer);
    }
    // FINAL MIX
    for (uint8_t i=0; i<DMA_BUF_LEN; i++) {
        output_dry_int32_buffer[i] = 0;
        output_wet_int32_buffer[i] = 0;
        for (uint8_t i_channel=0; i_channel<N_CHANNELS; i_channel++) {
            output_dry_int32_buffer[i] += channel_outputs[i_channel][i]*ek_channel_get_dry_volume(channels[i_channel]);
            output_wet_int32_buffer[i] += channel_outputs[i_channel][i]*ek_channel_get_wet_volume(channels[i_channel]);
        }
    }
}