#include "ek_monopoly.h"

#define BUTTON_OFF 0
#define BUTTON_ON  1

#define MAX_BUTTON_STACK_SIZE 10

struct ek_monopoly_s {
    uint8_t n_voices;
    uint8_t n_chord_voices;
    uint8_t base_midi_note;
    int8_t octave;
    uint8_t mode;
    int8_t *chord_pattern;
    uint8_t tonality;
    uint8_t arpeggio_step;
    uint16_t arpeggio_tick;
    uint16_t arpeggio_duration;
    uint8_t latest_button_stack_size; 
    int16_t latest_button_stack[MAX_BUTTON_STACK_SIZE];
    uint8_t highest_button_stack_size; 
    int16_t highest_button_stack[MAX_BUTTON_STACK_SIZE];
    int16_t *buttons;
    uint8_t *onoffs;
    uint32_t *phase_increments;
};

static const int8_t chord_pattern_open[36] = {
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19,
    12, 7, 19
};

static const int8_t chord_pattern_major[36] = {
    7, 4, 11, // I
    6, 3, 9,  // #I
    7, 3, 10, // II
    6, 3, 9,  // #II
    7, 3, 10, // III
    7, 4, 11, // IV
    6, 3, 9,  // #IV
    7, 4, 10, // V
    6, 3, 9,  // #V
    7, 3, 10, // VI
    6, 3, 9,  // #VI
    6, 3, 10  // VII
};

static const int8_t chord_pattern_minor[36] = {
    7, 3, 10, // I
    7, 4, 11,  // bII
    6, 3, 10, // II
    7, 4, 11, // III
    6, 3, 9,  // #III
    7, 3, 10, // IV
    6, 3, 9,  // #IV
    7, 4, 10, // V
    7, 4, 11, // VI
    6, 3, 9,  // #VI
    7, 4, 10, // VII
    6, 3, 9 // #VII
};

#define N_CHORD_PATTERNS 3
static int8_t *chord_patterns[N_CHORD_PATTERNS] = {chord_pattern_open,chord_pattern_major,chord_pattern_minor};

static void change_voice_midi_note(ek_monopoly_t monopoly,uint8_t i_voice) {
    uint8_t midi_note = monopoly->base_midi_note+12*monopoly->octave+monopoly->buttons[i_voice];
    monopoly->phase_increments[i_voice] = 21500000*pow(2.0f,(midi_note-69)/12.0f);
}

static void latest_button_stack_delete(ek_monopoly_t monopoly,uint8_t i_button) {
    if (i_button>=monopoly->latest_button_stack_size) {
        return;
    }
    monopoly->latest_button_stack_size--;
    for (uint8_t i=i_button; i<monopoly->latest_button_stack_size; i++) {
        monopoly->latest_button_stack[i] = monopoly->latest_button_stack[i+1];
    }
}

static void latest_button_stack_button_on(ek_monopoly_t monopoly, int16_t button) {
    if (monopoly->latest_button_stack_size==MAX_BUTTON_STACK_SIZE) {
        latest_button_stack_delete(monopoly,0);
    }
    monopoly->latest_button_stack[monopoly->latest_button_stack_size] = button;
    monopoly->latest_button_stack_size++;
}

static void latest_button_stack_button_off(ek_monopoly_t monopoly, int16_t button) {
    for (uint8_t i=0; i<monopoly->latest_button_stack_size; i++) {
        if (monopoly->latest_button_stack[i]==button) {
            latest_button_stack_delete(monopoly,i);
            break;
        }
    }
}

static void latest_button_update(ek_monopoly_t monopoly) {
    if (monopoly->latest_button_stack_size==0) {
        monopoly->onoffs[0] = BUTTON_OFF;
        return;
    }
    monopoly->buttons[0] = monopoly->latest_button_stack[monopoly->latest_button_stack_size-1];
    monopoly->onoffs[0] = BUTTON_ON;
    change_voice_midi_note(monopoly,0);
}

static void highest_button_stack_delete(ek_monopoly_t monopoly,uint8_t i_button) {
    if (i_button>=monopoly->highest_button_stack_size) {
        return;
    }
    monopoly->highest_button_stack_size--;
    for (uint8_t i=i_button; i<monopoly->highest_button_stack_size; i++) {
        monopoly->highest_button_stack[i] = monopoly->highest_button_stack[i+1];
    }
}

static void highest_button_stack_button_on(ek_monopoly_t monopoly, int16_t button) {
    if (monopoly->highest_button_stack_size==MAX_BUTTON_STACK_SIZE) {
        highest_button_stack_delete(monopoly,0);
    }
    uint8_t i;
    for (
        i=monopoly->highest_button_stack_size; 
        i>0 && monopoly->highest_button_stack[i-1]>button;
        i--
    ) {
        monopoly->highest_button_stack[i] = monopoly->highest_button_stack[i-1];
    }
    monopoly->highest_button_stack[i] = button;
    monopoly->highest_button_stack_size++;
}

static void highest_button_stack_button_off(ek_monopoly_t monopoly, int16_t button) {
    for (uint8_t i=0; i<monopoly->highest_button_stack_size; i++) {
        if (monopoly->highest_button_stack[i]==button) {
            highest_button_stack_delete(monopoly,i);
            break;
        }
    }
}

static void highest_button_update(ek_monopoly_t monopoly) {
    if (monopoly->highest_button_stack_size==0) {
        monopoly->onoffs[0] = BUTTON_OFF;
        return;
    }
    monopoly->buttons[0] = monopoly->highest_button_stack[monopoly->highest_button_stack_size-1];
    monopoly->onoffs[0] = BUTTON_ON;
    change_voice_midi_note(monopoly,0);
}

ek_monopoly_t ek_monopoly_create(uint8_t n_voices, uint8_t base_midi_note) {
    ek_monopoly_t monopoly = (ek_monopoly_t)malloc(sizeof(struct ek_monopoly_s));
    
    if (monopoly==NULL) {
        ESP_LOGI(TAG, "MONO-POLY : Unable to create !!!");
        return NULL;
    }
    monopoly->n_voices = n_voices;
    monopoly->n_chord_voices = n_voices<4 ? n_voices : 4;
    monopoly->base_midi_note = base_midi_note;
    monopoly->octave = 0;
    monopoly->mode = MONOPOLY_MODE_POLY;
    monopoly->chord_pattern = chord_patterns[0];
    monopoly->tonality = 0;
    monopoly->arpeggio_step = 0;
    monopoly->arpeggio_tick = 0;
    monopoly->arpeggio_duration = DMA_BUF_LEN;    
    monopoly->latest_button_stack_size = 0;
    monopoly->highest_button_stack_size = 0;
    monopoly->buttons = (int16_t *)malloc(n_voices*sizeof(int16_t));
    monopoly->onoffs = (uint8_t *)malloc(n_voices*sizeof(uint8_t));
    monopoly->phase_increments = (uint32_t *)malloc(n_voices*sizeof(uint32_t));
    if (monopoly->buttons==NULL || monopoly->onoffs==NULL || monopoly->phase_increments==NULL) {
        ESP_LOGI(TAG, "MONO-POLY : Unable to create !!!");
        return NULL;
    }
    for (uint8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
        monopoly->buttons[i_voice] = -1;
        monopoly->onoffs[i_voice] = BUTTON_OFF;
        monopoly->phase_increments[i_voice] = 0;
    }
    return monopoly;
}


static int8_t search_active_voice(ek_monopoly_t monopoly, int16_t button) {
    for (int8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
        if (monopoly->buttons[i_voice]==button) {
            return i_voice;
        }
    }
    return -1;
}

static int8_t search_inactive_voice(ek_monopoly_t monopoly) {
    for (int8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
        if (monopoly->buttons[i_voice]==-1 ) {
            return i_voice;
        }
    }
    return -1;
}

extern void ek_monopoly_button_on(ek_monopoly_t monopoly, int16_t button) {
    ESP_LOGI(TAG,"MONOPOLY : Button %d ON",button);
    latest_button_stack_button_on(monopoly,button);
    highest_button_stack_button_on(monopoly,button);

    if (monopoly->mode!=MONOPOLY_MODE_POLY) {
        return;
    } 
    int8_t i_voice = search_active_voice(monopoly,button);
    if (i_voice==-1) {
        i_voice = search_inactive_voice(monopoly);
    }
    if (i_voice==-1) {
        return;
    }
    monopoly->buttons[i_voice] = button;
    monopoly->onoffs[i_voice] = BUTTON_ON;
    change_voice_midi_note(monopoly,i_voice);
}

extern void ek_monopoly_button_off(ek_monopoly_t monopoly, int16_t button) {
    ESP_LOGI(TAG,"MONOPOLY : Button %d OFF",button);
    latest_button_stack_button_off(monopoly,button);
    highest_button_stack_button_off(monopoly,button);

    if (monopoly->mode!=MONOPOLY_MODE_POLY) {
        return;
    }
    int8_t i_voice = search_active_voice(monopoly,button);
    if (i_voice==-1) {
        return;
    }
    monopoly->onoffs[i_voice] = BUTTON_OFF;
}

extern void ek_monopoly_change_octave(ek_monopoly_t monopoly, int8_t octave) {
    monopoly->octave = octave;
    for (uint8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
        change_voice_midi_note(monopoly,i_voice);
    }
}

extern void ek_monopoly_change_mode(ek_monopoly_t monopoly, uint8_t mode) {
    monopoly->mode = mode;
    if (mode==MONOPOLY_MODE_POLY) {
        return;
    }
    for (uint8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
        monopoly->onoffs[i_voice] = BUTTON_OFF;
    }
}

extern void ek_monopoly_change_tonality(ek_monopoly_t monopoly, uint8_t tonality) {
    if (tonality>=12) return;
    monopoly->tonality = tonality;
}

extern void ek_monopoly_change_chord_pattern(ek_monopoly_t monopoly, uint8_t i_chord_pattern) {
    if (i_chord_pattern>=N_CHORD_PATTERNS) return;
    monopoly->chord_pattern = chord_patterns[i_chord_pattern];
}

extern void ek_monopoly_change_arpeggio_duration(ek_monopoly_t monopoly, uint16_t arpeggio_duration) {
    monopoly->arpeggio_duration = arpeggio_duration;
}

extern void ek_monopoly_deactivate_voice(ek_monopoly_t monopoly, uint8_t i_voice) {
    if (i_voice>=monopoly->n_voices) return;
    if (monopoly->onoffs[i_voice]!=BUTTON_OFF) {
        ESP_LOGI(TAG, "MONO-POLY : inactivate while button on!!!");
    }
    monopoly->buttons[i_voice] = -1;
}

extern uint8_t ek_monopoly_is_voice_active(ek_monopoly_t monopoly, uint8_t i_voice) {
    if (i_voice>=monopoly->n_voices) return 0;
    return monopoly->buttons[i_voice]!=-1;
}

extern uint8_t ek_monopoly_is_voice_on(ek_monopoly_t monopoly, uint8_t i_voice) {
    if (i_voice>=monopoly->n_voices) return 0;
    return monopoly->onoffs[i_voice];
}

extern uint32_t ek_monopoly_get_voice_phase_increment(ek_monopoly_t monopoly, uint8_t i_voice) {
    if (i_voice>=monopoly->n_voices) return 0;
    return monopoly->phase_increments[i_voice];
}

static int16_t harmonize(ek_monopoly_t monopoly, int16_t button,uint8_t component) {
    uint8_t relative_note = (monopoly->base_midi_note+button-monopoly->tonality)%12;
    return button+monopoly->chord_pattern[relative_note*3+component];
}

extern void ek_monopoly_compute(ek_monopoly_t monopoly) {
    uint8_t stack_size = 0;
    int16_t *button_stack = NULL;

    switch (monopoly->mode) {
    case MONOPOLY_MODE_MONO_LATEST:
    case MONOPOLY_MODE_MONO_ARP_FWD:
    case MONOPOLY_MODE_CHORD_2_LATEST:
    case MONOPOLY_MODE_CHORD_3_LATEST:
    case MONOPOLY_MODE_CHORD_4_LATEST:
        stack_size = monopoly->latest_button_stack_size;
        button_stack = monopoly->latest_button_stack;
        break;
    case MONOPOLY_MODE_MONO_HIGHEST:
    case MONOPOLY_MODE_MONO_ARP_UP:
    case MONOPOLY_MODE_CHORD_2_HIGHEST:
    case MONOPOLY_MODE_CHORD_3_HIGHEST:
    case MONOPOLY_MODE_CHORD_4_HIGHEST:
        stack_size = monopoly->highest_button_stack_size;
        button_stack = monopoly->highest_button_stack;
        break;
    default:
        return;
    }
    if (stack_size==0) {
        for (uint8_t i=0; i<monopoly->n_chord_voices;i++) {
            monopoly->onoffs[i] = BUTTON_OFF;
        }
        return;
    }
    switch (monopoly->mode) {
    case MONOPOLY_MODE_CHORD_4_LATEST:
    case MONOPOLY_MODE_CHORD_4_HIGHEST:
        if (monopoly->n_chord_voices>=4) {
            monopoly->buttons[3] = harmonize(monopoly,button_stack[stack_size-1],2);
            monopoly->onoffs[3] = BUTTON_ON;
            change_voice_midi_note(monopoly,3);
        }
    case MONOPOLY_MODE_CHORD_3_LATEST:
    case MONOPOLY_MODE_CHORD_3_HIGHEST:
        if (monopoly->n_chord_voices>=3) {
            monopoly->buttons[2] = harmonize(monopoly,button_stack[stack_size-1],1);
            monopoly->onoffs[2] = BUTTON_ON;
            change_voice_midi_note(monopoly,2);
        }
    case MONOPOLY_MODE_CHORD_2_LATEST:
    case MONOPOLY_MODE_CHORD_2_HIGHEST:
        if (monopoly->n_chord_voices>=2) {
            monopoly->buttons[1] = harmonize(monopoly,button_stack[stack_size-1],0);
            monopoly->onoffs[1] = BUTTON_ON;
            change_voice_midi_note(monopoly,1);
        }
    case MONOPOLY_MODE_MONO_LATEST:
    case MONOPOLY_MODE_MONO_HIGHEST:
        monopoly->buttons[0] = button_stack[stack_size-1];
        break;
    case MONOPOLY_MODE_MONO_ARP_FWD:
    case MONOPOLY_MODE_MONO_ARP_UP:
        monopoly->arpeggio_tick += DMA_BUF_LEN;
        if (monopoly->arpeggio_tick>=monopoly->arpeggio_duration) {
            monopoly->arpeggio_step = (monopoly->arpeggio_step+1)%stack_size;
            monopoly->arpeggio_tick -= monopoly->arpeggio_duration;
        }
        monopoly->buttons[0] = button_stack[monopoly->arpeggio_step];
        break;
    }    
    monopoly->onoffs[0] = BUTTON_ON;
    change_voice_midi_note(monopoly,0);
}
