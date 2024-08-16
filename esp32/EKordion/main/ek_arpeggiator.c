#include "ek_arpeggiator.h"

#define N_ARPEGGIATOR_PATTERNS 8
#define PATTERN_MAX_LENGTH 16

typedef struct pattern_s {
    uint8_t length;
    uint8_t factors[PATTERN_MAX_LENGTH];
    uint8_t shifts[PATTERN_MAX_LENGTH];
} *pattern_t;

const struct pattern_s pattern00 = { .length = 1, .factors = {1}, .shifts = {0}};
const struct pattern_s pattern01 = { .length = 2, .factors = {1, 2}, .shifts = {0, 0}};
const struct pattern_s pattern02 = { .length = 2, .factors = {1, 3}, .shifts = {0, 1}};
const struct pattern_s pattern03 = { .length = 3, .factors = {1, 2, 4}, .shifts = {0, 0, 0}};
const struct pattern_s pattern04 = { .length = 3, .factors = {1, 3, 2}, .shifts = {0, 1, 0}};
const struct pattern_s pattern05 = { .length = 4, .factors = {1, 3, 2, 3}, .shifts = {0, 1, 0, 1}};
const struct pattern_s pattern06 = { .length = 4, .factors = {1, 3, 2, 3}, .shifts = {0, 1, 0, 0}};
const struct pattern_s pattern07 = { .length = 4, .factors = {1, 5, 3, 2}, .shifts = {0, 2, 1, 0}};

const pattern_t patterns[N_ARPEGGIATOR_PATTERNS] = {
    &pattern00, &pattern01, &pattern02, &pattern03, 
    &pattern04, &pattern05, &pattern06, &pattern07
};

struct ek_arpeggiator_s {
    uint8_t step;
    uint16_t tick;
};

extern ek_arpeggiator_t ek_arpeggiator_create() {
    ek_arpeggiator_t arpeggiator = (ek_arpeggiator_t)malloc(sizeof(struct ek_arpeggiator_s));

    if (arpeggiator==NULL) {
        ESP_LOGI(TAG, "ARPEGGIATOR : Unable to create !!!");
        return arpeggiator;
    }
    arpeggiator->step = 0;
    arpeggiator->tick = 0;
    return arpeggiator;
}

extern void ek_arpeggiator_compute(
    ek_arpeggiator_t arpeggiator,
    uint8_t i_pattern, uint16_t duration,
    int32_t *intput_output_int32_buffer
) {
    if (i_pattern>=N_ARPEGGIATOR_PATTERNS) {
        return;
    }
    uint8_t step = arpeggiator->step%patterns[i_pattern]->length;
    uint8_t factor = patterns[i_pattern]->factors[step];
    uint8_t shift = patterns[i_pattern]->shifts[step];
    uint16_t tick = arpeggiator->tick;

    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        tick++;
        if (tick>=duration) {
            step = (step+1)%patterns[i_pattern]->length;
            factor = patterns[i_pattern]->factors[step];
            shift = patterns[i_pattern]->factors[step];
            tick = 0;
        }
        intput_output_int32_buffer[i] = (intput_output_int32_buffer[i]*factor)>>shift;
    }
    arpeggiator->tick = tick;
    arpeggiator->step = step;
}
