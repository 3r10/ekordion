#include "ek_oscillator.h"
#include "ek_tables.h"

#define N_OSCILLATORS_MAX 8
#define DETUNE_SHIFT 10
static const uint8_t volume_shifts[N_OSCILLATORS_MAX] = {0,1,2,2,3,3,3,3};

struct ek_oscillator_parameters_s {
    uint8_t n_oscillators;
    int32_t detune_factor;
    const int16_t *table;
    int16_t resolution_mask;
    uint8_t volume_shift;
};

struct ek_oscillator_s {
    int32_t phases[N_OSCILLATORS_MAX];
};

extern ek_oscillator_parameters_t ek_oscillator_parameters_create() {
    ek_oscillator_parameters_t parameters = (ek_oscillator_parameters_t)malloc(sizeof(struct ek_oscillator_parameters_s));

    if (parameters==NULL) {
        ESP_LOGI(TAG, "OSCILLATOR PARAMETERS : Unable to create !!!");
        return parameters;
    }
    parameters->n_oscillators = 1;
    parameters->detune_factor = 0;
    parameters->table = tables[0];
    parameters->resolution_mask = -1;
    parameters->volume_shift = 0;
    return parameters;
}

extern void ek_oscillator_parameters_change_n_oscillators(ek_oscillator_parameters_t parameters, uint8_t n_oscillators) {
    if (n_oscillators==0 || n_oscillators>N_OSCILLATORS_MAX) {
        return;
    }
    parameters->n_oscillators = n_oscillators;
    parameters->volume_shift = volume_shifts[n_oscillators-1];
}

extern void ek_oscillator_parameters_change_detune_factor(ek_oscillator_parameters_t parameters, uint8_t detune_factor) {
    parameters->detune_factor = detune_factor>>4;
}

extern void ek_oscillator_parameters_change_table(ek_oscillator_parameters_t parameters, const int16_t *table) {
    parameters->table = table;
}

extern void ek_oscillator_parameters_change_resolution_mask(ek_oscillator_parameters_t parameters, int16_t resolution_mask) {
    parameters->resolution_mask = resolution_mask;
}

extern ek_oscillator_t ek_oscillator_create() {
    ek_oscillator_t oscillator = (ek_oscillator_t)malloc(sizeof(struct ek_oscillator_s));

    if (oscillator==NULL) {
        ESP_LOGI(TAG, "OSCILLATOR : Unable to create !!!");
        return oscillator;
    }
    for (uint8_t i_oscillator=0; i_oscillator<N_OSCILLATORS_MAX; i_oscillator++) {
        oscillator->phases[i_oscillator] = 0;
    }
    return oscillator;
}

extern void ek_oscillator_compute(
    ek_oscillator_t oscillator, 
    ek_oscillator_parameters_t parameters,
    int32_t *phase_increment_int32_buffer,
    int32_t *output_int32_buffer
) {
    const int16_t *table = parameters->table;
    int16_t resolution_mask = parameters->resolution_mask;
    uint8_t n_oscillators = parameters->n_oscillators;
    int32_t detune_factor = parameters->detune_factor;
    
    if (n_oscillators==1 || detune_factor==0) {
        // MONO-OSCILLATOR
        uint32_t phase = oscillator->phases[0];
        for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
            output_int32_buffer[i] = table[phase>>TABLE_PHASE_SHIFT]&resolution_mask;
            phase += phase_increment_int32_buffer[i];
        }
        oscillator->phases[0] = phase;
        return;
    }
    // MULTI-OSCILLATOR
    uint8_t volume_shift = parameters->volume_shift;
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output_int32_buffer[i] = 0;
    }
    for (int8_t i_oscillator=0; i_oscillator<n_oscillators; i_oscillator++) {
        uint32_t phase = oscillator->phases[i_oscillator];
        int32_t factor = detune_factor*(i_oscillator-(n_oscillators>>1))+(1<<DETUNE_SHIFT);

        for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
            output_int32_buffer[i] += table[phase>>TABLE_PHASE_SHIFT]&resolution_mask;
            phase += (phase_increment_int32_buffer[i]>>DETUNE_SHIFT)*factor;
        }
        oscillator->phases[i_oscillator] = phase;
    }
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output_int32_buffer[i] >>= volume_shift;
    }
}


