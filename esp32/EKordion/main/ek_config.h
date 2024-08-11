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

/* SYNTH */
#define N_CHANNELS 3
#define N_VOICES 12
#define BASS_CHANNEL   0
#define CHORDS_CHANNEL 1
#define LEAD_CHANNEL   2
#define BASS_BASE_MIDI_NOTE   46 // Bb2
#define CHORDS_BASE_MIDI_NOTE 58 // Bb3
#define LEAD_BASE_MIDI_NOTE  59 // B3

#endif