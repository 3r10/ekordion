#include "ek_voice.h"
#include "ek_tables.h"

struct voice_s {
    uint32_t phase;
};

extern voice_t ek_voice_create() {
    voice_t voice = (voice_t)malloc(sizeof(struct voice_s));

    if (voice==NULL) {
        ESP_LOGI(TAG, "VOICE : Unable to create !!!");
        return voice;
    }
    voice->phase = 0;
    return voice;
}

extern void ek_


