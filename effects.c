#include "effects.h"

#define SINE_WAVE_LEN 1024
float sine_wave[SINE_WAVE_LEN] = {};

FILE *myfile;

void sine_wave_effect(float *in, float *out, float *arg, void* aux);

void init_effects(engine_config* config){
	//init sine buffer
    for (int i = 0; i < SINE_WAVE_LEN; ++i)
    {
        sine_wave[i] = (float)sin(2* i * 3.141592654 / SINE_WAVE_LEN);
    }
	effect_module e1 = {
		1, 1, 1,
		BUFFER_LEN, BUFFER_LEN,
		NULL, NULL, NULL,
		NULL, 0,
		"sine_gen",
		sine_wave_effect
	};
	ms_add_effect(e1, config);
	wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, config);
	w.inp[0] = JACKD_INPUT;
	w.arg[0] = NO_INPUT;
	ms_set_effect_arg(0, 0, 440.0, config);
	ms_add_wire(w, config);
	ms_set_output_module(0, 0, config);
}

//sine wave generator
//argments: [freq in Hz]
void sine_wave_effect(float *in, float *out, float *arg, void* aux){
	static float index = 0;
	for (int i = 0; i < BUFFER_LEN; ++i)
    {
        out[i] = sine_wave[(int)index];
        index += arg[0]*SINE_WAVE_LEN/SAMPLE_RATE;
        if (index >= SINE_WAVE_LEN)
        {
            index -= SINE_WAVE_LEN;
        }
        if (index < 0)
        {
            index += SINE_WAVE_LEN;
        }
    }
}

//simple add 3 effect
//buffer size is 20
void myeffect2(float *in, float *out, float *arg, void* aux){
	int buf_len = 20;
	for (int i = 0; i < buf_len; ++i)
	{
		out[i] = in[i] + 3;
	}
}

void print_array(float *arr, int size){
	for (int i = 0; i < size; ++i)
	{
		printf("%f ", arr[i]);
	}
	printf("\n");
}