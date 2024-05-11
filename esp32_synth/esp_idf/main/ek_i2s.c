#include "ek_i2s.h"
#include "driver/i2s.h"


static size_t bytes_written;
static uint16_t output_uint16_buffer[DMA_BUF_LEN * 2]; // 2 channels interleaved

extern void ek_i2s_init() {
    // I2S
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,       // 2 ?
        .dma_buf_count = DMA_NUM_BUF,                      
        .dma_buf_len = DMA_BUF_LEN,                            
        .use_apll=0, // false ?
        .tx_desc_auto_clear= true, 
        .fixed_mclk=-1    
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = 14,                // Bit clock
        .ws_io_num = 13,                 // Word select/ Left right clock
        .data_out_num = 12,              // DIN
        .data_in_num = I2S_PIN_NO_CHANGE // no input
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
}

extern void ek_i2s_write(
    int32_t output_l_int32_buffer[DMA_BUF_LEN],
    int32_t output_r_int32_buffer[DMA_BUF_LEN]
) {
    for (int i=0; i<DMA_BUF_LEN; i++) {
        output_uint16_buffer[i<<1] = (output_l_int32_buffer[i]>>7)+(1<<15);
        output_uint16_buffer[(i<<1)+1] = (output_r_int32_buffer[i]>>7)+(1<<15);
    }
    i2s_write(I2S_NUM, output_uint16_buffer, sizeof(output_uint16_buffer), &bytes_written, portMAX_DELAY);
}


