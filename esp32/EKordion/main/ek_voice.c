#include "ek_voice.h"
#include "ek_tables.h"

#define N_RAMP_STEPS 32 // ~ 1 ms ramp

struct voice_s {
    int16_t i_button;
    uint32_t phase_increment;
    uint32_t phase;
    uint16_t ramp_step;
    uint16_t arpeggio_tick;
    uint8_t arpeggio_step;
    int32_t output[DMA_BUF_LEN];
};

extern voice_t ek_voice_create() {
    voice_t voice = (voice_t)malloc(sizeof(struct voice_s));

    if (voice==NULL) {
        ESP_LOGI(TAG, "VOICE : Unable to create !!!");
        return voice;
    }
    voice->i_button = -1;
    voice->phase_increment = 0;
    voice->phase = 0;
    voice->ramp_step = 0;
    voice->arpeggio_tick = 0;
    voice->arpeggio_step = 0;
    return voice;
}

extern int16_t ek_voice_get_i_button(voice_t voice) {
    return voice->i_button;
}

extern uint16_t ek_voice_get_ramp_step(voice_t voice) {
    return voice->ramp_step;
}

extern void ek_voice_set_i_button(voice_t voice, int16_t i_button) {
    voice->i_button = i_button;
}

extern void ek_voice_change_phase_increment(
    voice_t voice, 
    uint8_t base_midi_note,
    int8_t octave
) {
    uint8_t midi_note = base_midi_note+12*octave+voice->i_button;
    voice->phase_increment = 21500000*pow(2.0f,(midi_note-69)/12.0f);
}

extern int32_t *ek_voice_compute(
    voice_t voice,
    int32_t *lfo_int32_buffer,
    channel_t channel
) {
    arpeggiator_t arpeggiator;
    uint32_t phase,phase_increment;
    uint8_t arpeggio_step, arpeggio_factor, arpeggio_shift, vibrato;
    uint16_t arpeggio_tick, arpeggio_duration;
    int16_t resolution_mask, *table;
    int32_t *output; 

    uint8_t on_off = voice->i_button!=-1;
    uint16_t ramp_step = voice->ramp_step;
    
    if (!(on_off || ramp_step)) {
        return NULL;
    }
    // Get params
    phase = voice->phase;
    phase_increment = voice->phase_increment;
    arpeggio_step = voice->arpeggio_step;
    arpeggio_tick = voice->arpeggio_tick;
    output = voice->output;
    // 
    table = ek_channel_get_table(channel);
    resolution_mask = ek_channel_get_resolution_mask(channel);
    arpeggio_duration = ek_channel_get_arpeggio_duration(channel);
    arpeggiator = ek_channel_get_arpeggiator(channel);
    vibrato = ek_channel_get_vibrato(channel);
    // Arp
    arpeggio_factor = ek_arpeggiator_get_factor(arpeggiator,arpeggio_step);
    arpeggio_shift = ek_arpeggiator_get_shift(arpeggiator,arpeggio_step);

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        if (on_off && ramp_step<N_RAMP_STEPS) {
            ramp_step++;
        }
        if (!on_off && ramp_step>0) {
            ramp_step--;
        }
        arpeggio_tick++;
        if (arpeggio_tick>=arpeggio_duration) {
            arpeggio_step = (arpeggio_step+1)%ek_arpeggiator_get_length(arpeggiator);
            arpeggio_factor = ek_arpeggiator_get_factor(arpeggiator,arpeggio_step);
            arpeggio_shift = ek_arpeggiator_get_shift(arpeggiator,arpeggio_step);
            arpeggio_tick = 0;
        }
        output[i] = (table[phase>>TABLE_PHASE_SHIFT]&resolution_mask)*ramp_step;
        phase += ((phase_increment+(((lfo_int32_buffer[i]*(int32_t)(phase_increment>>20))*vibrato)>>8))*arpeggio_factor)>>arpeggio_shift;
    }
    // Record params
    voice->phase = phase;
    voice->ramp_step = ramp_step;
    voice->arpeggio_tick = arpeggio_tick;
    voice->arpeggio_step = arpeggio_step;
    return output;
}


