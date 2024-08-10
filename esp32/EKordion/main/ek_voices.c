#include <string.h>
#include "ek_voices.h"
#include "ek_tables.h"

#define N_RAMP_STEPS 32 // ~ 1 ms ramp

typedef struct voice_s {
    int16_t i_button;
    uint32_t phase_increment;
    uint32_t phase;
    uint16_t ramp_step;
    int16_t *table;
    uint8_t vibrato_shift;
    uint8_t volume;
} voice_t;

static voice_t voices[N_VOICES];
static int16_t vibrato = 0;
static int16_t vibrato_step = 200;

extern void ek_voices_init() {
    for (int i_voice=0; i_voice<N_VOICES; i_voice++) {
        voices[i_voice].i_button = -1;
        voices[i_voice].phase_increment = 0;
        voices[i_voice].phase = 0;
        voices[i_voice].ramp_step = 0;
        voices[i_voice].volume = 200;
    }
    voices[0].table = tables[0];
    voices[0].vibrato_shift = 2;
    voices[1].table = tables[0];
    voices[1].vibrato_shift = 2; 
    for (int i_voice=2; i_voice<N_VOICES; i_voice++) {
        voices[i_voice].table = tables[0];
        voices[i_voice].vibrato_shift = 3;
    }
}

extern void ek_voices_change_custom_table(uint16_t length, uint8_t *data) {
    if (length!=2*CHANGE_TABLE_OFFSET+1) return;
    uint16_t offset = ((uint16_t)data[0])*CHANGE_TABLE_OFFSET;
    memcpy(tables[0]+offset,data+1,CHANGE_TABLE_OFFSET*sizeof(uint16_t));
}

extern void ek_voices_change_bass_volume(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    voices[0].volume = data[0];
}

extern void ek_voices_change_chords_volume(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    voices[1].volume = data[0];
}

extern void ek_voices_change_lead_volume(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    for (int i_voice=2; i_voice<N_VOICES; i_voice++) {
        voices[i_voice].volume = data[0];
    }
}

extern void ek_voices_change_bass_table(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    if (data[0]>=N_TABLES) return;
    voices[0].table = tables[data[0]];
}

extern void ek_voices_change_chords_table(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    if (data[0]>=N_TABLES) return;
    voices[1].table = tables[data[0]];
}

extern void ek_voices_change_lead_table(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    if (data[0]>=N_TABLES) return;
    for (int i_voice=2; i_voice<N_VOICES; i_voice++) {
        voices[i_voice].table = tables[data[0]];
    }
}

static int search_active_voice(int i_button) {
    for (int i_voice=2; i_voice<N_VOICES; i_voice++) {
        if (voices[i_voice].i_button==i_button) {
            return i_voice;
        }
    }
    return -1;
}

static int search_inactive_voice() {
    for (int i_voice=2; i_voice<N_VOICES; i_voice++) {
        if (voices[i_voice].i_button==-1 && voices[i_voice].ramp_step==0) {
            return i_voice;
        }
    }
    return -1;
}

extern void right_button_on(int i_button) {
    int i_voice = search_active_voice(i_button);
    if (i_voice==-1) {
        i_voice = search_inactive_voice();
    }
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Right button on : %d",i_button);
    voices[i_voice].i_button = i_button;
    voices[i_voice].phase_increment = 21500000*pow(2.0f,(59+i_button-69)/12.0f);
    
    // Microtonal
    // if (i_button%12==2) {
    //     voices[i_voice].phase_increment = 21500000*pow(2.0f,(58.5+i_button-69)/12.0f);
    // }
    // if (i_button%12==6) {
    //     voices[i_voice].phase_increment = 21500000*pow(2.0f,(59.5+i_button-69)/12.0f);
    // }
}

extern void right_button_off(int i_button) {
    int i_voice = search_active_voice(i_button);
    if (i_voice==-1) {
        return;
    }
    ESP_LOGI(TAG, "Right button off : %d",i_button);
    voices[i_voice].i_button = -1;
}

extern void left_button_on(int i_button) {
    ESP_LOGI(TAG, "Left button on : %d",i_button);
    if (i_button<12) {
        voices[0].i_button = i_button;
        voices[0].phase_increment = 21500000*pow(2.0f,(46+i_button-69)/12.0f);
    }
    else {
        voices[1].i_button = i_button;
        voices[1].phase_increment = 21500000*pow(2.0f,(46+i_button-69)/12.0f);
    }
}

extern void left_button_off(int i_button) {
    ESP_LOGI(TAG, "Left button off : %d",i_button);
    if (i_button<12 && voices[0].i_button==i_button) {
        voices[0].i_button = -1;
        voices[0].phase_increment = 0;
    }
    else if (voices[1].i_button==i_button) {
        voices[1].i_button = -1;
        voices[1].phase_increment = 0;
    }
}

extern void ek_voices_compute(int32_t output_int32_buffer[DMA_BUF_LEN]) {
    uint32_t phase = 0, phase_increment = 0;
    uint8_t on_off,vibrato_shift,volume;
    const int16_t *table;
    uint16_t ramp_step;

    for (int i=0; i<DMA_BUF_LEN; i++) {
        output_int32_buffer[i] = 0;
    }
    for (int i_voice=0; i_voice<N_VOICES; i_voice++) {
        on_off = voices[i_voice].i_button!=-1;
        ramp_step = voices[i_voice].ramp_step;
        volume = voices[i_voice].volume;
        if (on_off || ramp_step) {
            phase_increment = voices[i_voice].phase_increment;
            phase = voices[i_voice].phase;
            table = voices[i_voice].table;
            vibrato_shift = voices[i_voice].vibrato_shift;
            for (int i=0; i<DMA_BUF_LEN; i++) {
                if (on_off && ramp_step<N_RAMP_STEPS) {
                    ramp_step++;
                }
                if (!on_off && ramp_step>0) {
                    ramp_step--;
                }
                output_int32_buffer[i] += table[phase>>TABLE_PHASE_SHIFT]*ramp_step*volume>>10;
                phase += phase_increment+(vibrato<<vibrato_shift);
            }

            voices[i_voice].ramp_step = ramp_step;
            voices[i_voice].phase = phase;
        }
    }
    vibrato += vibrato_step;
    if (vibrato<0) {
        vibrato -= vibrato_step;
        vibrato_step = -vibrato_step;
        vibrato += vibrato_step;
    }
}