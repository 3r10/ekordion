#include "ek_arpeggiator.h"

struct arpeggiator_s {
    uint8_t length;
    uint8_t factors[ARPEGGIATOR_MAX_LENTGH];
    uint8_t shifts[ARPEGGIATOR_MAX_LENTGH];
};

const struct arpeggiator_s arpeggiator00 = { .length = 1, .factors = {1}, .shifts = {0}};
const struct arpeggiator_s arpeggiator01 = { .length = 2, .factors = {1, 2}, .shifts = {0, 0}};
const struct arpeggiator_s arpeggiator02 = { .length = 2, .factors = {1, 3}, .shifts = {0, 1}};
const struct arpeggiator_s arpeggiator03 = { .length = 3, .factors = {1, 2, 4}, .shifts = {0, 0, 0}};
const struct arpeggiator_s arpeggiator04 = { .length = 3, .factors = {1, 3, 2}, .shifts = {0, 1, 0}};
const struct arpeggiator_s arpeggiator05 = { .length = 4, .factors = {1, 3, 2, 3}, .shifts = {0, 1, 0, 1}};
const struct arpeggiator_s arpeggiator06 = { .length = 4, .factors = {1, 3, 2, 3}, .shifts = {0, 1, 0, 0}};
const struct arpeggiator_s arpeggiator07 = { .length = 4, .factors = {1, 5, 3, 2}, .shifts = {0, 2, 1, 0}};

const arpeggiator_t arpeggiators[N_ARPEGGIATORS] = {
    &arpeggiator00, &arpeggiator01, &arpeggiator02, &arpeggiator03, 
    &arpeggiator04, &arpeggiator05, &arpeggiator06, &arpeggiator07
};

extern arpeggiator_t ek_arpeggiator_get(uint8_t i_arpeggiator) {
    return arpeggiators[i_arpeggiator];
}

extern uint8_t ek_arpeggiator_get_length(arpeggiator_t arpeggiator) {
    return arpeggiator->length;
}

extern uint8_t ek_arpeggiator_get_factor(arpeggiator_t arpeggiator, uint8_t step) {
    return arpeggiator->factors[step];
}

extern uint8_t ek_arpeggiator_get_shift(arpeggiator_t arpeggiator, uint8_t step) {
    return arpeggiator->shifts[step];
}



