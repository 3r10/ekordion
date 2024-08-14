#include "ek_lfo.h"
#include "ek_tables.h"

struct lfo_s {
    uint32_t phase_increment;
    uint32_t phase;
    const int16_t *table;
    int32_t output[DMA_BUF_LEN];
};

extern lfo_t ek_lfo_create() {
    lfo_t lfo = (lfo_t)malloc(sizeof(struct lfo_s));

    if (lfo==NULL) {
        ESP_LOGI(TAG, "LFO : Unable to create !!!");
        return lfo;
    }
    lfo->phase_increment = 0;
    lfo->phase = 0;
    lfo->table = tables[1]; // Sine wave
    return lfo;
}

extern void ek_lfo_change_phase_increment(lfo_t lfo, uint32_t phase_increment) {
    lfo->phase_increment = phase_increment;
}

extern void ek_lfo_change_table(lfo_t lfo, const int16_t *table) {
    lfo->table = table;
}

extern int32_t *ek_lfo_compute(lfo_t lfo) {
    uint32_t phase = lfo->phase;
    uint32_t phase_increment = lfo->phase_increment;
    const int16_t *table = lfo->table;
    int32_t *output = lfo->output;

   for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        output[i] = table[phase>>TABLE_PHASE_SHIFT];
        phase += phase_increment;
    }
    lfo->phase = phase;
    return output;
}


