#include "ek_adsr.h"

#define STATE_IDLE    0
#define STATE_ATTACK  1
#define STATE_DECAY   2
#define STATE_SUSTAIN 3
#define STATE_RELEASE 4

struct ek_adsr_s {
    int32_t value;
    uint8_t state;
};

extern ek_adsr_t ek_adsr_create() {
    ek_adsr_t adsr = (ek_adsr_t)malloc(sizeof(struct ek_adsr_s));

    if (adsr==NULL) {
        ESP_LOGI(TAG, "ADSR : Unable to create !!!");
        return adsr;
    }
    adsr->value = 0;
    adsr->state = STATE_IDLE;
    return adsr;
}

extern int32_t ek_adsr_is_active(ek_adsr_t adsr) {
    return adsr->state!=STATE_IDLE;

}

extern void ek_adsr_compute(
    ek_adsr_t adsr, uint8_t on_off,
    int32_t delta_a, int32_t delta_d, int32_t value_s, int32_t delta_r,
    int32_t output_int32_buffer[DMA_BUF_LEN]
) {
    uint16_t i;
    int32_t value = adsr->value;

    if (on_off) {
        switch (adsr->state) {
        case STATE_IDLE:
            adsr->state = STATE_ATTACK;
        case STATE_ATTACK:
            for (i=0; i<DMA_BUF_LEN && value<ADSR_MAXIMUM_VALUE; i++) {
                output_int32_buffer[i] = value;
                value += delta_a;
            }
            if (i<DMA_BUF_LEN) {
                adsr->state = STATE_DECAY;
            }
            for (; i<DMA_BUF_LEN; i++) {
                output_int32_buffer[i] = ADSR_MAXIMUM_VALUE;
            }
            break;
        case STATE_DECAY:
            for (i=0; i<DMA_BUF_LEN && value>value_s; i++) {
                output_int32_buffer[i] = value;
                value -= delta_d;
            }
            if (i<DMA_BUF_LEN) {
                adsr->state = STATE_SUSTAIN;
            }
            for (; i<DMA_BUF_LEN; i++) {
                output_int32_buffer[i] = value_s;
            }
            break;
        case STATE_SUSTAIN:
            for (i=0; i<DMA_BUF_LEN; i++) {
                output_int32_buffer[i] = value_s;
            }
            break;
        default:
            ESP_LOGI(TAG, "ADSR : Unkown state value");
            break;
        }
    }
    else {
        switch (adsr->state) {
        case STATE_IDLE:
            for (i=0; i<DMA_BUF_LEN; i++) {
                output_int32_buffer[i] = 0;
            }
            break;
        case STATE_ATTACK:
        case STATE_DECAY:
            if (delta_d>delta_r) {
                delta_r = delta_d;
            }
            for (i=0; i<DMA_BUF_LEN && value>value_s; i++) {
                output_int32_buffer[i] = value;
                value -= delta_r;
            }
            if (i<DMA_BUF_LEN) {
                adsr->state = STATE_RELEASE;
            }
            for (; i<DMA_BUF_LEN; i++) {
                output_int32_buffer[i] = value_s;
            }
            break;
        case STATE_SUSTAIN:
            adsr->state = STATE_RELEASE;
        case STATE_RELEASE:
            for (i=0; i<DMA_BUF_LEN && value>0; i++) {
                output_int32_buffer[i] = value;
                value -= delta_r;
            }
            if (i<DMA_BUF_LEN) {
                adsr->state = STATE_IDLE;
            }
            for (; i<DMA_BUF_LEN; i++) {
                output_int32_buffer[i] = 0;
            }
        }
    }
    adsr->value = output_int32_buffer[DMA_BUF_LEN-1];
}



