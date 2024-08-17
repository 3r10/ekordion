#include "ek_adsr.h"

#define ADSR_SHIFT_VALUE 22
#define ADSR_MAXIMUM_VALUE (1<<30)

#define STATE_IDLE    0
#define STATE_ATTACK  1
#define STATE_DECAY   2
#define STATE_SUSTAIN 3
#define STATE_RELEASE 4

struct ek_adsr_parameters_s {
    int32_t delta_a;
    int32_t value_d;
    int32_t delta_d;
    int32_t value_s; 
    int32_t value_r;
    int32_t delta_r;
};

struct ek_adsr_s {
    int32_t value;
    uint8_t state;
};

static int32_t slider_to_value(uint8_t slider) {
    return (((int32_t)slider)+1)<<6;
}

extern ek_adsr_parameters_t ek_adsr_parameters_create() {
    ek_adsr_parameters_t parameters = (ek_adsr_parameters_t)malloc(sizeof(struct ek_adsr_parameters_s));

    if (parameters==NULL) {
        ESP_LOGI(TAG, "ADSR PARAMETERS : Unable to create !!!");
        return parameters;
    }
    parameters->delta_a = ADSR_MAXIMUM_VALUE/DMA_BUF_LEN;
    parameters->value_d = DMA_BUF_LEN;
    parameters->delta_d = 0;
    parameters->value_s = ADSR_MAXIMUM_VALUE;
    parameters->value_r = DMA_BUF_LEN;
    parameters->delta_r = ADSR_MAXIMUM_VALUE/DMA_BUF_LEN;
    return parameters;
}

extern void ek_adsr_parameters_change_a(ek_adsr_parameters_t parameters, uint8_t a) {
    parameters->delta_a = ADSR_MAXIMUM_VALUE/slider_to_value(a);
}

extern void ek_adsr_parameters_change_d(ek_adsr_parameters_t parameters, uint8_t d) {
    parameters->value_d = slider_to_value(d);
    parameters->delta_d = (ADSR_MAXIMUM_VALUE-parameters->value_s)/parameters->value_d;
}

extern void ek_adsr_parameters_change_s(ek_adsr_parameters_t parameters, uint8_t s) {
    parameters->value_s = ((int32_t)s)<<ADSR_SHIFT_VALUE;
    parameters->delta_d = (ADSR_MAXIMUM_VALUE-parameters->value_s)/parameters->value_d;
    parameters->delta_r = parameters->value_s/parameters->value_r;
}

extern void ek_adsr_parameters_change_r(ek_adsr_parameters_t parameters, uint8_t r) {
    parameters->value_r = slider_to_value(r);
    parameters->delta_r = parameters->value_s/parameters->value_r;
}

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
    ek_adsr_parameters_t parameters,
    int32_t output_int32_buffer[DMA_BUF_LEN]
) {
    int32_t delta_a = parameters->delta_a;
    int32_t delta_d = parameters->delta_d;
    int32_t value_s = parameters->value_s;
    int32_t delta_r = parameters->delta_r;
    int32_t value = adsr->value;
    uint16_t i;

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



