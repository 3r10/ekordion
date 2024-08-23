#include "ek_config.h"
#include "ek_bluetooth.h"
#include "ek_keyboards.h"
#include "ek_synth.h"
#include "ek_i2s.h"

#if (MEASURE_CPU_USAGE == true)
#include "time.h"
#include "sys/time.h"
static struct timeval time_stamp;
float time_before_s, time_after_s, time_interval = 0;
uint16_t time_counter = 0;
#endif



static void write_buffer()
{
    static int32_t output_l_int32_buffer[DMA_BUF_LEN];
    static int32_t output_r_int32_buffer[DMA_BUF_LEN];
    
#if (MEASURE_CPU_USAGE == true)
    gettimeofday(&time_stamp,NULL);
    time_before_s = time_stamp.tv_sec + time_stamp.tv_usec / 1000000.0;
#endif

    ek_synth_compute(output_l_int32_buffer,output_r_int32_buffer);
    
#if (MEASURE_CPU_USAGE == true)
    gettimeofday(&time_stamp,NULL);
    time_after_s = time_stamp.tv_sec + time_stamp.tv_usec / 1000000.0;
    time_interval += time_after_s - time_before_s;
    time_counter++;
    if (time_counter>=SAMPLE_RATE/DMA_BUF_LEN) {
        ESP_LOGI(TAG, "CPU usage %.2f %%" , time_interval*100);
        time_interval = 0.0f;
        time_counter = 0;
    }
#endif
    ek_i2s_write(output_l_int32_buffer,output_r_int32_buffer);
}

static void audio_task(void *userData)
{
    while(1) {
        write_buffer();
    }
}

void keyboards_task(void *userData)
{   
    while (1) {
        ek_keyboards_get_data();
        vTaskDelay(4 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    uint8_t n_channels = 3;
    uint8_t n_voices_array[3] = {2,4,12}; // bass=2, chords=4, lead=12
    uint8_t base_midi_note_array[3] = {46,58,59}; // Bb2, Bb3, B3

    ek_i2s_init();
    ek_keyboards_init();   
    ek_synth_init(n_channels,n_voices_array,base_midi_note_array);
    ek_bluetooth_start(ek_synth_change_parameter);
    // TODO : xTaskCreatePinnedToCore ???
    xTaskCreate(audio_task,"audio",20480,NULL,configMAX_PRIORITIES-1,NULL);
    xTaskCreate(keyboards_task,"keyboards",8192,NULL,configMAX_PRIORITIES-2,NULL);
}
