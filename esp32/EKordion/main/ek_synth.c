#include <string.h>
#include "ek_tables.h"
#include "ek_lfo.h"
#include "ek_arpeggiator.h"
#include "ek_channel.h"
#include "ek_reverb.h"
#include "ek_synth.h"

static void (*change_functions[N_CHANGE_FUNCTIONS])(uint16_t length, uint8_t *data) = {0};
static lfo_t lfo;
static channel_t channels[N_CHANNELS];

static void change_custom_table(uint16_t length, uint8_t *data) {
    if (length!=2*CHANGE_TABLE_OFFSET+1) return;
    uint16_t offset = ((uint16_t)data[0])*CHANGE_TABLE_OFFSET;
    memcpy(tables[0]+offset,data+1,CHANGE_TABLE_OFFSET*sizeof(uint16_t));
}

static void change_lfo_frequency(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    ek_lfo_change_phase_increment(lfo,3820*(uint32_t)data[0]);
}

static void change_lfo_table(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    if (data[0]>=N_TABLES) return;
    ek_lfo_change_table(lfo,tables[data[0]]);
}

static void change_table(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=N_TABLES) return;
    ek_channel_change_table(channels[i_channel],tables[data[1]]);
}

static void change_resolution(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>15) return;
    ek_channel_change_resolution_mask(channels[i_channel],~((1<<data[1])-1));
}

static void change_downsampling(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>=DMA_BUF_LEN) return;
    ek_channel_change_downsampling(channels[i_channel],data[1]);
}

static void change_octave(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    if (data[1]>6) return;
    ek_channel_change_octave(channels[i_channel],((int8_t)data[1])-3);
}

static void change_arpeggio_duration(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_arpeggio_duration(channels[i_channel],(1+(uint16_t)data[1])<<4);
}

static void change_arpeggiator(uint16_t length, uint8_t *data) {
    uint8_t i_channel;
    
    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_arpeggiator_pattern(channels[i_channel],data[1]);
}

static void change_vibrato(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_vibrato(channels[i_channel],data[1]);
}

static void change_tremolo(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_tremolo(channels[i_channel],data[1]);
}

static void change_dry_volume(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_dry_volume(channels[i_channel],data[1]);
}

static void change_wet_volume(uint16_t length, uint8_t *data) {
    uint8_t i_channel;

    if (length!=2) return;
    i_channel = data[0];
    if (i_channel>=N_CHANNELS) return;
    ek_channel_change_wet_volume(channels[i_channel],data[1]);
}

extern void ek_synth_init() {
    // LFO
    lfo = ek_lfo_create();
    // CHANNELS
    channels[BASS_CHANNEL] = ek_channel_create(2,BASS_BASE_MIDI_NOTE);
    channels[CHORDS_CHANNEL] = ek_channel_create(2,CHORDS_BASE_MIDI_NOTE);
    channels[LEAD_CHANNEL] = ek_channel_create(N_VOICES,LEAD_BASE_MIDI_NOTE);
    for (int i=0; i<N_CHANGE_FUNCTIONS; i++) {
        change_functions[i] = NULL;
    }
    change_functions[CHANGE_CUSTOM_TABLE] = &change_custom_table;
    change_functions[CHANGE_LFO_TABLE] = &change_lfo_table;
    change_functions[CHANGE_LFO_FREQUENCY] = &change_lfo_frequency;
    change_functions[CHANGE_REVERB_FEEDBACK] = &ek_reverb_change_feedback;
    change_functions[CHANGE_REVERB_DAMPING] = &ek_reverb_change_damping;
    change_functions[CHANGE_REVERB_VOLUME] = &ek_reverb_change_volume;
    change_functions[CHANGE_TABLE] = &change_table;
    change_functions[CHANGE_RESOLUTION] = &change_resolution;
    change_functions[CHANGE_DOWNSAMPLING] = &change_downsampling;
    change_functions[CHANGE_OCTAVE] = &change_octave;
    change_functions[CHANGE_ARPEGGIO_DURATION] = &change_arpeggio_duration;
    change_functions[CHANGE_ARPEGGIATOR] = &change_arpeggiator;
    change_functions[CHANGE_VIBRATO] = &change_vibrato;
    change_functions[CHANGE_TREMOLO] = &change_tremolo;
    change_functions[CHANGE_DRY_VOLUME] = &change_dry_volume;
    change_functions[CHANGE_WET_VOLUME] = &change_wet_volume;
}

extern void ek_synth_change_parameter(uint16_t length, uint8_t *data) {

    uint8_t i_change_function = data[0];
    if (change_functions[i_change_function]!=NULL) {
        change_functions[i_change_function](length-1,data+1);
    }
}

extern void ek_synth_button_on(uint8_t i_channel,int16_t i_button) {
    ek_channel_button_on(channels[i_channel],i_button);
}

extern void ek_synth_button_off(uint8_t i_channel,int16_t i_button) {
    ek_channel_button_off(channels[i_channel],i_button);
}

extern void ek_synth_compute(
    int32_t output_l_int32_buffer[DMA_BUF_LEN],
    int32_t output_r_int32_buffer[DMA_BUF_LEN]
) {
    int32_t *lfo_int32_buffer;
    int32_t *channel_outputs[N_CHANNELS];
    static int32_t dry_int32_buffer[DMA_BUF_LEN];
    static int32_t wet_int32_buffer[DMA_BUF_LEN];

    for (uint8_t i=0; i<DMA_BUF_LEN; i++) {
        dry_int32_buffer[i] = 0;
        wet_int32_buffer[i] = 0;
    }
    // LFO
    lfo_int32_buffer = ek_lfo_compute(lfo);
    for (uint8_t i_channel=0; i_channel<N_CHANNELS; i_channel++) {
        ek_channel_compute(channels[i_channel],lfo_int32_buffer,dry_int32_buffer,wet_int32_buffer);
    }
    ek_reverb_compute(dry_int32_buffer,wet_int32_buffer,output_l_int32_buffer,output_r_int32_buffer);
}