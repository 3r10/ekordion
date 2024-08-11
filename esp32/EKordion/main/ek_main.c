#include "ek_config.h"
#include "ek_bluetooth.h"
#include "ek_keyboards.h"
#include "ek_voices.h"
#include "ek_reverb.h"
#include "ek_i2s.h"

#if (MEASURE_CPU_USAGE == true)
#include "time.h"
#include "sys/time.h"
static struct timeval time_stamp;
float time_before_s, time_after_s, time_interval = 0;
uint16_t time_counter = 0;
#endif

/* BUFFERS */
static int32_t int32_dry_buffer[DMA_BUF_LEN];
static int32_t int32_wet_buffer[DMA_BUF_LEN];
static int32_t output_l_int32_buffer[DMA_BUF_LEN];
static int32_t output_r_int32_buffer[DMA_BUF_LEN];

#define N_CHANGE_FUNCTIONS 128
static void (*change_functions[N_CHANGE_FUNCTIONS])(uint16_t length, uint8_t *data) = {0};

static void write_buffer()
{

#if (MEASURE_CPU_USAGE == true)
    gettimeofday(&time_stamp,NULL);
    time_before_s = time_stamp.tv_sec + time_stamp.tv_usec / 1000000.0;
#endif

    ek_channels_compute(int32_dry_buffer,int32_wet_buffer);
    ek_reverb_compute(int32_dry_buffer,int32_wet_buffer,output_l_int32_buffer,output_r_int32_buffer);

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

void bluetooth_callback(uint16_t length, uint8_t *data) {
    uint8_t i_change_function = data[0];
    if (change_functions[i_change_function]!=NULL) {
        change_functions[i_change_function](length-1,data+1);
    }
}

void app_main(void)
{
    ek_i2s_init();
    ek_keyboards_init();   
    ek_voices_init();

    for (int i=0; i<N_CHANGE_FUNCTIONS; i++) {
        change_functions[i] = NULL;
    }
    change_functions[ 0] = &ek_voices_change_custom_table;
    change_functions[ 1] = &ek_voices_change_lfo_table;
    change_functions[ 2] = &ek_voices_change_lfo_frequency;
    change_functions[ 3] = &ek_reverb_change_feedback;
    change_functions[ 4] = &ek_reverb_change_damping;
    change_functions[ 5] = &ek_reverb_change_volume;
    change_functions[ 6] = &ek_voices_change_table;
    change_functions[ 7] = &ek_voices_change_octave;
    change_functions[ 8] = &ek_voices_change_vibrato;
    change_functions[ 9] = &ek_voices_change_dry_volume;
    change_functions[10] = &ek_voices_change_wet_volume;
    
    ek_bluetooth_start(bluetooth_callback);

    // TODO : xTaskCreatePinnedToCore ???
    xTaskCreate(audio_task,"audio",20480,NULL,configMAX_PRIORITIES-1,NULL);
    xTaskCreate(keyboards_task,"keyboards",8192,NULL,configMAX_PRIORITIES-2,NULL);
}
