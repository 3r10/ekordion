#include "ek_phasor.h"

struct ek_phasor_s {
    int16_t note; // relative to keyboard
    uint32_t phase_increment;
};

ek_phasor_t ek_phasor_create() {
    ek_phasor_t phasor = (ek_phasor_t)malloc(sizeof(struct ek_phasor_s));

    if (phasor==NULL) {
        ESP_LOGI(TAG, "PHASOR : Unable to create !!!");
        return phasor;
    }
    phasor->note = -1;
    phasor->phase_increment = 0;
    return phasor;
}

int16_t ek_phasor_get_note(ek_phasor_t phasor) {
    return phasor->note;
}

void ek_phasor_actualize(ek_phasor_t phasor, int16_t base_note) {
    phasor->phase_increment = 21500000*pow(2.0f,(base_note+phasor->note-69)/12.0f);
}

void ek_phasor_change_note(ek_phasor_t phasor, int16_t note) {
    phasor->note = note;
}

void ek_phasor_compute(ek_phasor_t phasor, int32_t output_int32_buffer[DMA_BUF_LEN]) {
    uint32_t phase_increment = phasor->phase_increment;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output_int32_buffer[i] = phase_increment;
    }
}
