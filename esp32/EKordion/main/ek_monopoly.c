#include "ek_monopoly.h"

#define BUTTON_OFF 0
#define BUTTON_ON  1

#define MAX_BUTTON_STACK_SIZE 10

struct ek_monopoly_s {
    uint8_t n_voices;
    uint8_t base_midi_note;
    int8_t octave;
    uint8_t mode;
    int16_t latest_button_stack[MAX_BUTTON_STACK_SIZE];
    uint8_t latest_button_stack_size; 
    int16_t *buttons;
    uint8_t *onoffs;
    uint32_t *phase_increments;
};

static void change_voice_midi_note(ek_monopoly_t monopoly,uint8_t i_voice) {
    uint8_t midi_note = monopoly->base_midi_note+12*monopoly->octave+monopoly->buttons[i_voice];
    monopoly->phase_increments[i_voice] = 21500000*pow(2.0f,(midi_note-69)/12.0f);
}

static void latest_button_stack_delete(ek_monopoly_t monopoly,uint8_t i_in_stack) {
    if (i_in_stack>=monopoly->latest_button_stack_size) {
        return;
    }
    monopoly->latest_button_stack_size--;
    for (uint8_t i=i_in_stack; i<monopoly->latest_button_stack_size; i++) {
        monopoly->latest_button_stack[i] = monopoly->latest_button_stack[i+1];
    }
}

static void latest_button_stack_button_on(ek_monopoly_t monopoly, int16_t i_button) {
    if (monopoly->latest_button_stack_size==MAX_BUTTON_STACK_SIZE) {
        latest_button_stack_delete(monopoly,0);
    }
    monopoly->latest_button_stack[monopoly->latest_button_stack_size] = i_button;
    monopoly->latest_button_stack_size++;
}

static void latest_button_stack_button_off(ek_monopoly_t monopoly, int16_t i_button) {
    for (uint8_t i=0; i<monopoly->latest_button_stack_size; i++) {
        if (monopoly->latest_button_stack[i]==i_button) {
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

ek_monopoly_t ek_monopoly_create(uint8_t n_voices, uint8_t base_midi_note) {
    ek_monopoly_t monopoly = (ek_monopoly_t)malloc(sizeof(struct ek_monopoly_s));
    
    if (monopoly==NULL) {
        ESP_LOGI(TAG, "MONO-POLY : Unable to create !!!");
        return NULL;
    }
    monopoly->n_voices = n_voices;
    monopoly->base_midi_note = base_midi_note;
    monopoly->octave = 0;
    monopoly->mode = MONOPOLY_MODE_POLY;
    monopoly->latest_button_stack_size = 0;
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


static int8_t search_active_voice(ek_monopoly_t monopoly, int16_t i_button) {
    for (int8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
        if (monopoly->buttons[i_voice]==i_button) {
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

extern void ek_monopoly_button_on(ek_monopoly_t monopoly, int16_t i_button) {
    ESP_LOGI(TAG,"MONOPOLY : Button %d ON",i_button);
    latest_button_stack_button_on(monopoly,i_button);

    switch (monopoly->mode) {
    case MONOPOLY_MODE_POLY:
        int8_t i_voice = search_active_voice(monopoly,i_button);
        if (i_voice==-1) {
            i_voice = search_inactive_voice(monopoly);
        }
        if (i_voice==-1) {
            return;
        }
        monopoly->buttons[i_voice] = i_button;
        monopoly->onoffs[i_voice] = BUTTON_ON;
        change_voice_midi_note(monopoly,i_voice);
        break;
    case MONOPOLY_MODE_MONO_LATEST:
        latest_button_update(monopoly);
        break;
    default:
        ESP_LOGI(TAG,"MONOPOLY : Unknown mode %d",monopoly->mode);
        break;
    }
}

extern void ek_monopoly_button_off(ek_monopoly_t monopoly, int16_t i_button) {
    ESP_LOGI(TAG,"MONOPOLY : Button %d OFF",i_button);
    latest_button_stack_button_off(monopoly,i_button);

    switch (monopoly->mode) {
    case MONOPOLY_MODE_POLY:
        int8_t i_voice = search_active_voice(monopoly,i_button);
        if (i_voice==-1) {
            return;
        }
        monopoly->onoffs[i_voice] = BUTTON_OFF;
        break;
    case MONOPOLY_MODE_MONO_LATEST:
        latest_button_update(monopoly);
        break;
    default:
        ESP_LOGI(TAG,"MONOPOLY : Unknown mode %d",monopoly->mode);
        break;
    }
}

extern void ek_monopoly_change_octave(ek_monopoly_t monopoly, int8_t octave) {
    monopoly->octave = octave;
    for (uint8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
        change_voice_midi_note(monopoly,i_voice);
    }
}

extern void ek_monopoly_change_mode(ek_monopoly_t monopoly, uint8_t mode) {
    monopoly->mode = mode;
    if (mode!=MONOPOLY_MODE_POLY) {
        for (uint8_t i_voice=0; i_voice<monopoly->n_voices; i_voice++) {
            monopoly->onoffs[i_voice] = BUTTON_OFF;
        }
    }
    latest_button_update(monopoly);
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
