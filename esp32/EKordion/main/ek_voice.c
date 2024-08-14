#include "ek_voice.h"
#include "ek_tables.h"

#define N_RAMP_STEPS 32 // ~ 1 ms ramp

struct voice_s {
    uint8_t base_midi_note;
    int8_t octave;
    int16_t *table;
    int16_t resolution_mask;
    uint16_t arpeggio_duration; // in samples
    uint16_t arpeggio_tick;
    uint8_t arpeggio_step;
    arpeggiator_t arpeggiator;
    uint8_t vibrato;
    int16_t i_button;
    uint32_t phase_increment;
    uint32_t phase;
    uint16_t ramp_step;
    int32_t output[DMA_BUF_LEN];
};

static void change_phase_increment(voice_t voice) {
    uint8_t midi_note = voice->base_midi_note+12*voice->octave+voice->i_button;
    voice->phase_increment = 21500000*pow(2.0f,(midi_note-69)/12.0f);
}


extern voice_t ek_voice_create(uint8_t base_midi_note) {
    voice_t voice = (voice_t)malloc(sizeof(struct voice_s));

    if (voice==NULL) {
        ESP_LOGI(TAG, "VOICE : Unable to create !!!");
        return voice;
    }
    voice->base_midi_note = base_midi_note;
    voice->table = tables[0];
    voice->resolution_mask = -1;
    voice->i_button = -1;
    voice->arpeggiator = ek_arpeggiator_get(0);
    voice->arpeggio_duration = SAMPLE_RATE;
    voice->vibrato = 0;
    voice->octave = 0;
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

extern void ek_voice_change_table(voice_t voice, int16_t *table) {
    voice->table = table;
}

extern void ek_voice_change_resolution_mask(voice_t voice, int16_t resolution_mask) {
    voice->resolution_mask = resolution_mask;
}

extern void ek_voice_change_i_button(voice_t voice, int16_t i_button) {
    voice->i_button = i_button;
    change_phase_increment(voice);
}

extern void ek_voice_change_octave(voice_t voice, int8_t octave) {
    voice->octave = octave;
    change_phase_increment(voice);
}

extern void ek_voice_change_arpeggio_duration(voice_t voice, uint16_t arpeggio_duration) {
    voice->arpeggio_duration = arpeggio_duration;
}

extern void ek_voice_change_arpeggiator(voice_t voice, arpeggiator_t arpeggiator) {
    voice->arpeggiator = arpeggiator;
}

extern void ek_voice_change_vibrato(voice_t voice, uint8_t vibrato) {
    voice->vibrato = vibrato;
}

extern int32_t *ek_voice_compute(
    voice_t voice,
    int32_t *lfo_int32_buffer
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
    table = voice->table;
    resolution_mask = voice->resolution_mask;
    arpeggio_duration = voice->arpeggio_duration;
    arpeggiator = voice->arpeggiator;
    vibrato = voice->vibrato;
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


