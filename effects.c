#include "effects.h"

#define SINE_WAVE_LEN 2048
#define HALF_STEP_LENGTH 87
float sine_wave[SINE_WAVE_LEN] = {};
float half_step[HALF_STEP_LENGTH] = {};

FILE *myfile;

void sine_wave_effect(float *in, float *out, float *arg, void* aux);
void sample_player(float *in, float *out, float *arg, void *aux);
void note2freq(float *in, float *out, float *arg, void *aux);

void init_effects(engine_config* config){
	FILE *f = fopen("test.raw", "wb");
	//init sine buffer
    for (int i = 0; i < SINE_WAVE_LEN; ++i)
    {
        sine_wave[i] = (float)sin(2* i * 3.141592654 / SINE_WAVE_LEN);
        fwrite(&sine_wave[i], sizeof(float), 1, f);
    }
    fclose(f);
    //init half step buffer
    half_step[0] = 27.5;	//Frequency of A0
    for (int i = 1; i < HALF_STEP_LENGTH; ++i)
    {
    	half_step[i] = half_step[0] * pow(2.0, (float)i / 12.0);
    }
    //create sample
    midi_sample sample = ms_create_midi("samples/cello_C2.mp3", 0.0, 0.0);
	if(sample.attack_l == 0){
		ms_log("sample empty");
	}
	effect_module e1 = {
		0, 1, 1,
		0, BUFFER_LEN,
		NULL, NULL, NULL,
		&sample, sizeof(midi_sample),
		"cello_gen",
		sample_player
	};
	ms_add_effect(e1, config);
	effect_module e2 = {
		0, 1, 1,
		0, 1,
		NULL, NULL, NULL,
		NULL, 0,
		"half_step",
		note2freq
	};
	ms_add_effect(e2, config);
	//sine gen
	wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, config);
	w.arg[0] = NO_INPUT;
	ms_set_effect_arg(0, 0, 1.0, config);
	ms_add_wire(w, config);
	//half step converter
	wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, config);
	w2.arg[0] = NO_INPUT;
	ms_set_effect_arg(1, 0, 0, config);
	ms_add_wire(w2, config);
	ms_set_output_module(0, 0, config);
}

//sine wave generator
//argments: [freq in Hz]
//aux buffer: [index (float)]
void sine_wave_effect(float *in, float *out, float *arg, void *aux){
	float index = ((float*)aux)[0];	//get index value
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
    ((float*)aux)[0] = index;	//store index value
}

//sample player
//arguments: [freq in Hz]
//aux buffer: [index (float)] [Midi Sample Struct]
void sample_player(float *in, float *out, float *arg, void *aux){
	midi_sample* sample = (midi_sample*)aux;	//get sample
	for (int i = 0; i < BUFFER_LEN; ++i)
    {
        out[i] = sample->attack_buf[(int)sample->index];
        sample->index += arg[0]*(sample->attack_l/SAMPLE_RATE);
        if (sample->index >= sample->attack_l)
        {
            sample->index -= sample->attack_l;
        }
        if (sample->index < 0)
        {
            sample->index += sample->attack_l;
        }
    }
}

//converts a number representing a half step to a note
//half steps start at A0 and go until C8 (like on an 88 key piano)
void note2freq(float *in, float *out, float *arg, void *aux){
	int index = (int)arg[0];
	if(index < 0 || index >= HALF_STEP_LENGTH)
		return;
	out[0] = half_step[index];
}

void print_array(float *arr, int size){
	for (int i = 0; i < size; ++i)
	{
		printf("%f ", arr[i]);
	}
	printf("\n");
}