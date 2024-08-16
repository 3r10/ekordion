#include "ek_adsr.h"

#define N_RAMP_STEPS 32 // ~ 1 ms ramp

struct ek_adsr_s {
    uint16_t value;
};

extern ek_adsr_t ek_adsr_create() {
    ek_adsr_t adsr = (ek_adsr_t)malloc(sizeof(struct ek_adsr_s));

    if (adsr==NULL) {
        ESP_LOGI(TAG, "ADSR : Unable to create !!!");
        return adsr;
    }
    adsr->value = 0;
    return adsr;
}

extern uint16_t ek_adsr_is_active(ek_adsr_t adsr) {
    return adsr->value;
}

extern void ek_adsr_compute(ek_adsr_t adsr, uint8_t on_off, int32_t output_int32_buffer[DMA_BUF_LEN]) {
    uint16_t value = adsr->value;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        if (on_off && value<N_RAMP_STEPS) {
            value++;
        }
        if (!on_off && value>0) {
            value--;
        }
        output_int32_buffer[i] = value;
    }

    adsr->value = value;
}



