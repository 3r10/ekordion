#include "ek_voice.h"
#include "ek_tables.h"

struct voice_s {
    int16_t *table;
    int16_t resolution_mask;
    uint16_t arpeggio_duration; // in samples
    uint16_t arpeggio_tick;
    uint8_t arpeggio_step;
    arpeggiator_t arpeggiator;
    uint8_t vibrato;
    uint32_t phase_increment;
    uint32_t phase;
};

extern voice_t ek_voice_create() {
    voice_t voice = (voice_t)malloc(sizeof(struct voice_s));

    if (voice==NULL) {
        ESP_LOGI(TAG, "VOICE : Unable to create !!!");
        return voice;
    }
    voice->table = tables[0];
    voice->resolution_mask = -1;
    voice->arpeggiator = ek_arpeggiator_get(0);
    voice->arpeggio_duration = SAMPLE_RATE;
    voice->vibrato = 0;
    voice->phase_increment = 0;
    voice->phase = 0;
    voice->arpeggio_tick = 0;
    voice->arpeggio_step = 0;
    return voice;
}

extern void ek_voice_change_table(voice_t voice, int16_t *table) {
    voice->table = table;
}

extern void ek_voice_change_resolution_mask(voice_t voice, int16_t resolution_mask) {
    voice->resolution_mask = resolution_mask;
}

extern void ek_voice_change_midi_note(voice_t voice, uint8_t midi_note) {
    voice->phase_increment = 21500000*pow(2.0f,(midi_note-69)/12.0f);
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

extern void ek_voice_compute(
    voice_t voice,
    uint8_t on_off,
    int32_t *lfo_int32_buffer,
    int32_t *output_int32_buffer
) {
    arpeggiator_t arpeggiator;
    uint32_t phase,phase_increment;
    uint8_t arpeggio_step, arpeggio_factor, arpeggio_shift, vibrato;
    uint16_t arpeggio_tick, arpeggio_duration;
    int16_t resolution_mask, *table;
    
    // Get params
    phase = voice->phase;
    phase_increment = voice->phase_increment;
    arpeggio_step = voice->arpeggio_step;
    arpeggio_tick = voice->arpeggio_tick;
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
        arpeggio_tick++;
        if (arpeggio_tick>=arpeggio_duration) {
            arpeggio_step = (arpeggio_step+1)%ek_arpeggiator_get_length(arpeggiator);
            arpeggio_factor = ek_arpeggiator_get_factor(arpeggiator,arpeggio_step);
            arpeggio_shift = ek_arpeggiator_get_shift(arpeggiator,arpeggio_step);
            arpeggio_tick = 0;
        }
        output_int32_buffer[i] = (table[phase>>TABLE_PHASE_SHIFT]&resolution_mask);
        phase += ((phase_increment+(((lfo_int32_buffer[i]*(int32_t)(phase_increment>>20))*vibrato)>>8))*arpeggio_factor)>>arpeggio_shift;
    }
    // Record params
    voice->phase = phase;
    voice->arpeggio_tick = arpeggio_tick;
    voice->arpeggio_step = arpeggio_step;
}


