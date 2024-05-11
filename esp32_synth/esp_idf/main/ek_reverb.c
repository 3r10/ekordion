#include "ek_reverb.h"

// based on : https://ccrma.stanford.edu/~jos/pasp/Freeverb.html

#define DELAY_LENGTH 2048
#define DELAY_MASK   2047
static int32_t delay_line_1[DELAY_LENGTH] = {0};
static int32_t z_1 = 0;
static int32_t delay_line_2[DELAY_LENGTH] = {0};
static int32_t z_2 = 0;
static int32_t delay_line_3[DELAY_LENGTH] = {0};
static int32_t z_3 = 0;
static int32_t delay_line_4[DELAY_LENGTH] = {0};
static int32_t z_4 = 0;
static int32_t delay_line_5[DELAY_LENGTH] = {0};
static int32_t z_5 = 0;
static int32_t delay_line_6[DELAY_LENGTH] = {0};
static int32_t z_6 = 0;
static int32_t delay_line_7[DELAY_LENGTH] = {0};
static int32_t z_7 = 0;
static int32_t delay_line_8[DELAY_LENGTH] = {0};
static int32_t z_8 = 0;
static uint16_t read_index = 0;

static uint8_t damping = 192;
static uint8_t feedback = 224;

extern void ek_reverb_change_damping(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    damping = data[0];
}

extern void ek_reverb_change_feedback(uint16_t length, uint8_t *data) {
    if (length!=1) return;
    feedback = data[0];
}

extern void ek_reverb_compute(
    int32_t input_int32_buffer[DMA_BUF_LEN], 
    int32_t output_l_int32_buffer[DMA_BUF_LEN],
    int32_t output_r_int32_buffer[DMA_BUF_LEN]
) {
    int32_t y = 0;
    uint8_t damping_complement = 255-damping;
    
    for (int i=0; i<DMA_BUF_LEN; i++) {
        output_l_int32_buffer[i] = input_int32_buffer[i];
        output_r_int32_buffer[i] = input_int32_buffer[i];
        // 1
        y = delay_line_1[read_index&DELAY_MASK];
        output_l_int32_buffer[i] += y>>2;
        z_1 = ((damping*y+damping_complement*z_1)>>8);
        delay_line_1[(read_index+1557)&DELAY_MASK] = input_int32_buffer[i]+((z_1*feedback)>>8);
        // 2
        y = delay_line_2[read_index&DELAY_MASK];
        output_l_int32_buffer[i] += y>>2;
        z_2 = ((damping*y+damping_complement*z_2)>>8);
        delay_line_2[(read_index+1617)&DELAY_MASK] = input_int32_buffer[i]+((z_2*feedback)>>8);
        // 3
        y = delay_line_3[read_index&DELAY_MASK];
        output_l_int32_buffer[i] += y>>2;
        z_3 = ((damping*y+damping_complement*z_3)>>8);
        delay_line_3[(read_index+1491)&DELAY_MASK] = input_int32_buffer[i]+((z_3*feedback)>>8);
        // 4
        y = delay_line_4[read_index&DELAY_MASK];
        output_l_int32_buffer[i] += y>>2;
        z_4 = ((damping*y+damping_complement*z_4)>>8);
        delay_line_4[(read_index+1422)&DELAY_MASK] = input_int32_buffer[i]+((z_4*feedback)>>8);
        // 5
        y = delay_line_5[read_index&DELAY_MASK];
        output_r_int32_buffer[i] += y>>2;
        z_5 = ((damping*y+damping_complement*z_5)>>8);
        delay_line_5[(read_index+1277)&DELAY_MASK] = input_int32_buffer[i]+((z_5*feedback)>>8);
        // 6
        y = delay_line_6[read_index&DELAY_MASK];
        output_r_int32_buffer[i] += y>>2;
        z_6 = ((damping*y+damping_complement*z_6)>>8);
        delay_line_6[(read_index+1356)&DELAY_MASK] = input_int32_buffer[i]+((z_6*feedback)>>8);
        // 7
        y = delay_line_7[read_index&DELAY_MASK];
        output_r_int32_buffer[i] += y>>2;
        z_7 = ((damping*y+damping_complement*z_7)>>8);
        delay_line_7[(read_index+1188)&DELAY_MASK] = input_int32_buffer[i]+((z_7*feedback)>>8);
        // 8
        y = delay_line_8[read_index&DELAY_MASK];
        output_r_int32_buffer[i] += y>>2;
        z_8 = ((damping*y+damping_complement*z_8)>>8);
        delay_line_8[(read_index+1116)&DELAY_MASK] = input_int32_buffer[i]+((z_8*feedback)>>8);
        
        read_index++;
    }
}