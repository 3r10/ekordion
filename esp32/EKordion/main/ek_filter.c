#include "ek_filter.h"
#include "ek_tables.h"

#define FIXED_POINT_SHIFT 12

struct ek_filter_parameters_s {
    uint8_t low_f_slider;
    uint8_t high_f_slider; // for dynamic filtering only
    uint8_t q_slider;
    int32_t b0,b1,b2; // numerator / for static filtering only
    int32_t a1,a2; // denominator (a0=1) / for static filtering only
};

struct ek_filter_s {
    int32_t x1,x2; // previous inputs
    int32_t y1,y2; // previous outputs
};

// FILTERS DESIGNS : http://shepazu.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html


/* FLOAT VERSION / STATIC FILTERING : could be useful at some point */
static void compute_coefficients(ek_filter_parameters_t parameters) {
    double nu0 = (((double)parameters->low_f_slider)+1)/512;
    double q = (((double)parameters->q_slider)+1)/24;
    ESP_LOGI(TAG, "FILTER nu0 q %f %f",nu0,q);
    double omega0 = 2*M_PI*nu0;
    double alpha = sin(omega0)/2/q;
    ESP_LOGI(TAG, "FILTER omegalpha %f %f",omega0,alpha);
    double factor = (1<<12)/(1+alpha);
    double coeff = (1-cos(omega0))*factor;
    parameters->b0 = coeff/2;
    parameters->b1 = coeff;
    parameters->b2 = coeff/2;
    parameters->a1 = -2*cos(omega0)*factor;
    parameters->a2 = (1-alpha)*factor;
    ESP_LOGI(TAG, "FILTER tmp %ld %ld %ld %ld %ld",
        parameters->b0,
        parameters->b1,
        parameters->b2,
        parameters->a1,
        parameters->a2
    );
}

/* INTERGER VERSION / STATIC FILTERING : for testing purpose only */
/*
static void compute_coefficients(ek_filter_parameters_t parameters) {
    uint16_t phase0 = (((uint16_t)parameters->low_f_slider)+1)<<2; // between 0 and 1024 (eq to pi)
    int32_t sin_phase0 = table_0001[phase0];
    int32_t cos_phase0 = table_0001[phase0+512];
    int32_t alpha = 12*(sin_phase0/(((uint16_t)parameters->q_slider)+1));
    int32_t factor = ((1<<15)<<FIXED_POINT_SHIFT)/((1<<15)+alpha);
    int32_t coeff = (((1<<15)-cos_phase0)*factor)>>15;
    parameters->b0 = coeff>>1;
    parameters->b1 = coeff;
    parameters->b2 = coeff>>1;
    parameters->a1 = -2*((cos_phase0*factor)>>15);
    parameters->a2 = (((1<<15)-alpha)*factor)>>15;
}
*/

extern ek_filter_parameters_t ek_filter_parameters_create() {
    ek_filter_parameters_t parameters = (ek_filter_parameters_t)malloc(sizeof(struct ek_filter_parameters_s));

    if (parameters==NULL) {
        ESP_LOGI(TAG, "FILTER PARAMETERS : Unable to create !!!");
        return parameters;
    }
    parameters->low_f_slider = 255;
    parameters->high_f_slider = 255;
    parameters->q_slider = 50;
    compute_coefficients(parameters);
    return parameters;
}

extern void ek_filter_parameters_change_low_f(ek_filter_parameters_t parameters, uint8_t low_f_slider) {
    parameters->low_f_slider = low_f_slider;
    compute_coefficients(parameters);
}

extern void ek_filter_parameters_change_high_f(ek_filter_parameters_t parameters, uint8_t high_f_slider) {
    parameters->high_f_slider = high_f_slider;
    compute_coefficients(parameters);
}

extern void ek_filter_parameters_change_q(ek_filter_parameters_t parameters, uint8_t q_slider) {
    parameters->q_slider = q_slider;
    compute_coefficients(parameters);
}

extern ek_filter_t ek_filter_create() {
    ek_filter_t filter = (ek_filter_t)malloc(sizeof(struct ek_filter_s));

    if (filter==NULL) {
        ESP_LOGI(TAG, "FILTER : Unable to create !!!");
        return filter;
    }
    filter->x1 = 0;
    filter->x2 = 0;
    filter->y1 = 0;
    filter->y2 = 0;
    return filter;
}

/* STATIC FILTERING : could be useful at some point */

extern void ek_filter_compute(
    ek_filter_t filter, 
    ek_filter_parameters_t parameters,
    int32_t *intput_output_int32_buffer
) {
    int32_t x1 = filter->x1;
    int32_t x2 = filter->x2;
    int32_t y1 = filter->y1;
    int32_t y2 = filter->y2;
    int32_t b0 = parameters->b0;
    int32_t b1 = parameters->b1;
    int32_t b2 = parameters->b2;
    int32_t a1 = parameters->a1;
    int32_t a2 = parameters->a2;
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        int32_t x0 = intput_output_int32_buffer[i]>>8;
        int32_t y0 = (b0*x0+b1*x1+b2*x2-a1*y1-a2*y2)>>12;
        y2 = y1;
        y1 = y0;
        x2 = x1;
        x1 = x0;
        intput_output_int32_buffer[i] = y0<<8;
    }
    filter->x1 = x1;
    filter->x2 = x2;
    filter->y1 = y1;
    filter->y2 = y2;
}

extern void ek_dynamic_filter_compute(
    ek_filter_t filter, ek_filter_parameters_t parameters,
    int32_t *modulation_input_buffer,
    int32_t *intput_output_int32_buffer
) {
    if (parameters->low_f_slider==255 && parameters->high_f_slider==255 ) {
        filter->x1 = intput_output_int32_buffer[DMA_BUF_LEN-1];
        filter->x2 = intput_output_int32_buffer[DMA_BUF_LEN-2];
        filter->y1 = intput_output_int32_buffer[DMA_BUF_LEN-1];
        filter->y2 = intput_output_int32_buffer[DMA_BUF_LEN-2];
        return;
    }
    int32_t x1 = filter->x1;
    int32_t x2 = filter->x2;
    int32_t y1 = filter->y1;
    int32_t y2 = filter->y2;
    int16_t low_f = ((uint16_t)parameters->low_f_slider)+1;
    int16_t high_f = ((uint16_t)parameters->high_f_slider)+1;
    for (uint16_t i=0; i<DMA_BUF_LEN; i++) {
        int32_t x0 = intput_output_int32_buffer[i]>>8;
        uint16_t phase0 = ((1<<16)*low_f+(modulation_input_buffer[i]>>14)*(high_f-low_f))>>14; // between 0 and 1024 (eq to pi)
        int32_t sin_phase0 = table_0001[phase0];
        int32_t cos_phase0 = table_0001[phase0+512];
        int32_t alpha = 12*(sin_phase0/(((uint16_t)parameters->q_slider)+1));
        int32_t factor = ((1<<15)<<FIXED_POINT_SHIFT)/((1<<15)+alpha);
        int32_t coeff = (((1<<15)-cos_phase0)*factor)>>15;
        int32_t b0 = coeff>>1;
        int32_t b1 = coeff;
        int32_t b2 = coeff>>1;
        int32_t a1 = -2*((cos_phase0*factor)>>15);
        int32_t a2 = (((1<<15)-alpha)*factor)>>15;    
        int32_t y0 = (b0*x0+b1*x1+b2*x2-a1*y1-a2*y2)>>12;
        y2 = y1;
        y1 = y0;
        x2 = x1;
        x1 = x0;
        intput_output_int32_buffer[i] = y0<<8;
    }
    filter->x1 = x1;
    filter->x2 = x2;
    filter->y1 = y1;
    filter->y2 = y2;
}

