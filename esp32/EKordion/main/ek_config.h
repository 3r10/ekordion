#include <math.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#ifndef EK_CONFIG_H
#define EK_CONFIG_H

static const char* TAG = "EKordion";

/* TIMING STATS */

// #define MEASURE_CPU_USAGE false
#define MEASURE_CPU_USAGE true

/* BLUETOOTH */
#define EK_BLUETOOTH_SERVER_NAME "EKordion server"
#define EK_BLUETOOTH_DEVICE_NAME "EKordion device"
#define CHANGE_CUSTOM_TABLE      0
#define CHANGE_LFO_TABLE         1
#define CHANGE_LFO_FREQUENCY     2
#define CHANGE_REVERB_FEEDBACK   3
#define CHANGE_REVERB_DAMPING    4
#define CHANGE_REVERB_VOLUME     5
#define CHANGE_TABLE             6
#define CHANGE_RESOLUTION        7
#define CHANGE_DOWNSAMPLING      8
#define CHANGE_OCTAVE            9
#define CHANGE_MONOPOLY_MODE     10
#define CHANGE_ARPEGGIO_DURATION 11
#define CHANGE_ARPEGGIATOR       12
#define CHANGE_VIBRATO           13
#define CHANGE_N_OSCILLATORS     14
#define CHANGE_DETUNE_FACTOR     15
#define CHANGE_ENVELOPE_A        16
#define CHANGE_ENVELOPE_D        17
#define CHANGE_ENVELOPE_S        18
#define CHANGE_ENVELOPE_R        19
#define CHANGE_FILTER_LOW_F      20
#define CHANGE_FILTER_HIGH_F     21
#define CHANGE_FILTER_Q          22
#define CHANGE_FILTER_MOD_A      23
#define CHANGE_FILTER_MOD_D      24
#define CHANGE_FILTER_MOD_S      25
#define CHANGE_FILTER_MOD_R      26
#define CHANGE_TREMOLO           27
#define CHANGE_DRY_VOLUME        28
#define CHANGE_WET_VOLUME        29
#define N_CHANGE_FUNCTIONS       30

/* GENERAL I2S PARAMETERS */
#define SAMPLE_RATE     44100
#define DMA_BUF_LEN     32
#define DMA_NUM_BUF     2
#define I2S_NUM         I2S_NUM_0

/* KEYBOARDS PINS*/
#define GPIO_SHIFT_LOAD   GPIO_NUM_15
#define GPIO_CLK          GPIO_NUM_2
#define GPIO_LEFT_SERIAL  GPIO_NUM_4
#define GPIO_RIGHT_SERIAL GPIO_NUM_16

/* KEYBOARD */
#define N_RIGHT_SWITCHES 40
#define N_LEFT_SWITCHES 24

#endif