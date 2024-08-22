#include "ek_lfo.h"
#include "ek_tables.h"

struct ek_lfo_s {
    const int16_t *table;
    uint32_t phase_increment;
    uint32_t phase;
};

extern ek_lfo_t ek_lfo_create() {
    ek_lfo_t lfo = (ek_lfo_t)malloc(sizeof(struct ek_lfo_s));

    if (lfo==NULL) {
        ESP_LOGI(TAG, "LFO : Unable to create !!!");
        return lfo;
    }
    lfo->phase_increment = 0;
    lfo->phase = 0;
    lfo->table = tables[1]; // Sine wave
    return lfo;
}

extern void ek_lfo_change_phase_increment(ek_lfo_t lfo, uint32_t phase_increment) {
    lfo->phase_increment = phase_increment;
}

extern void ek_lfo_change_table(ek_lfo_t lfo, const int16_t *table) {
    lfo->table = table;
}

extern void ek_lfo_compute(ek_lfo_t lfo, int32_t output_int32_buffer[DMA_BUF_LEN]) {
    const int16_t *table = lfo->table;
    uint32_t phase_increment = lfo->phase_increment;
    uint32_t phase = lfo->phase;

   for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output_int32_buffer[i] = table[phase>>TABLE_PHASE_SHIFT];
        phase += phase_increment;
    }
    lfo->phase = phase;
}


