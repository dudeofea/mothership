#include <stdlib.h>
#include <string.h>
#include "effects.h"

#define SINE_WAVE_LEN 2048
#define HALF_STEP_LENGTH 87
float sine_wave[SINE_WAVE_LEN] = {};
float half_step[HALF_STEP_LENGTH] = {};

int index = 0;
float hold = 0.0;

typedef struct
{
	float BPM, hold;
	int index, len;
	int *notes;
}sequence;

void volume(float *in, float *out, float *arg, void *aux);
void low_pass_filter(float *in, float *out, float *arg, void *aux);
void beat_freq_effect(float *in, float *out, float *arg, void *aux);
void chordifier(float *in, float *out, float *arg, void *aux);
void sequencer_effect(float *in, float *out, float *arg, void *aux);
void sine_wave_effect(float *in, float *out, float *arg, void* aux);
void sample_player(float *in, float *out, float *arg, void *aux);
void note2freq(float *in, float *out, float *arg, void *aux);
void log_array(float *arr, int size);

//pitch selector
void pitch_effect(float *in, float *out, float *arg, void *aux){
	out[0] = 2.0*arg[0];
	//out[0] = (float)half_step[(int)(arg[0] / 16.0)];
}

void init_effects(engine_config* config){
	//init sine buffer
    for (int i = 0; i < SINE_WAVE_LEN; ++i)
    {
        sine_wave[i] = (float)sin(2* i * 3.141592654 / SINE_WAVE_LEN);
    }
    //init half step buffer
    half_step[0] = 27.5;	//Frequency of A0
    for (int i = 1; i < HALF_STEP_LENGTH; ++i)
    {
    	half_step[i] = half_step[0] * pow(2.0, (float)i / 12.0);
    }
    //create sample
    /*midi_sample sample = ms_create_midi("samples/cello_C2.mp3", 0.0, 0.0);
	if(sample.attack_l == 0){
		ms_log("sample empty");
	}*/
	/*effect_module e2 = {
		0, 1, 10,				//in, out, arg
		0, 1,			//in size, out size
		NULL, NULL, NULL,
		NULL, sizeof(float),
		"sequencer",
		sequencer_effect
	};
	wire w2 = {
		0,NULL,NULL,NULL,NULL
	};
	ms_add_effect(e2, config);
	ms_wire_alloc(&w2, config);
	ms_add_wire(w2, config);*/
	//sine wave generator
	effect_module e1 = {
		0, 1, 10,				//in, out, arg
		0, BUFFER_LEN,			//in size, out size
		NULL, NULL, NULL,
		NULL, 4*sizeof(float),
		"chordifier",
		chordifier,
		255,0,0
	};
	wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_add_effect(e1, config);
	ms_wire_alloc(&w, config);
	//w.arg[0] = 0;
	//w.arg_ports[0] = 0;
	ms_add_wire(w, config);

	effect_module e2 = {
		1, 1, 10,				//in, out, arg
		0, BUFFER_LEN,			//in size, out size
		NULL, NULL, NULL,
		NULL, 1*sizeof(float),
		"low pass filter",
		volume,
		126, 4, 90
	};
	wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_add_effect(e2, config);
	ms_wire_alloc(&w2, config);
	w2.inp[0] = 0;
	w2.inp_ports[0] = 0;
	ms_add_wire(w2, config);

	ms_set_effect_arg(0, 0, 200.0, config);
	ms_set_output_module(1, 0, config);
	/*effect_module e2 = {
		0, 1, 1,
		0, 1,
		NULL, NULL, NULL,
		NULL, 0,
		"half_step",
		note2freq
	};
	ms_add_effect(e2, config);
	sequence s = {200.0, 0, 0, 8, NULL};
	s.notes = malloc(8*sizeof(int));
	s.notes[0] = NOTE_Db(4);
	s.notes[1] = NOTE_G(3);
	s.notes[2] = NOTE_A(3);
	s.notes[3] = NOTE_E(3);
	s.notes[4] = NOTE_Db(4);
	s.notes[5] = NOTE_G(3);
	s.notes[6] = NOTE_A(3);
	s.notes[7] = NOTE_E(3);
	effect_module e3 = {
		1, 0, 1,
		1, 0,
		NULL, NULL, NULL,
		(void*)&s, sizeof(sequence),
		"sequencer",
		sequencer_effect
	};
	ms_add_effect(e3, config);

	//half step converter
	wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, config);
	w2.arg[0] = 2;
	ms_set_effect_arg(1, 0, 0, config);
	ms_add_wire(w2, config);

	wire w3 = {
		2,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w3, config);
	w3.arg[0] = NO_INPUT;
	ms_set_effect_arg(2, 0, 100.0, config);
	ms_add_wire(w3, config);*/
}

//scale traverse (A Pentatonic)
//for the trigger: 0 -> not running, 1 -> begin running, 2 -> is running
//arguments: [Start note], [End note], [BPM], [trigger boolean]
//aux buffer: [Current Note (int)], [hold (float)]
/*void pent_scale_traverse(float *in, float *out, float *arg, void *aux){
	if (trigger == 1)
	{
		//start the traversal
	}else if(trigger == 2){
		//do the traversal
	}
}*/

//a simple volume effect
void volume(float *in, float *out, float *arg, void *aux){
	for (int i = 0; i < BUFFER_LEN; ++i)
	{
		out[i] = arg[0]/1024 * in[i];
	}
}

//a low pass filter
void low_pass_filter(float *in, float *out, float *arg, void *aux){
	float last = ((float*)aux)[0];	//last in value
	float s = arg[0]/1024;	//strength of filter
	out[0] = (1.0-s)*in[0] + (s)*last;
	for (int i = 1; i < BUFFER_LEN; ++i)
	{
		out[i] = (1.0-s)*in[i] + (s)*in[i-1];
	}
	((float*)aux)[0] = in[BUFFER_LEN-1];	//store last value
}

//try to generate a beat frequency
void beat_freq_effect(float *in, float *out, float *arg, void *aux){
	float index = ((float*)aux)[0];	//get index value
	float tmp_index = index;
	float next_index;
	memset(out, 0, BUFFER_LEN*sizeof(float));
	float freq = arg[0]/2;
	for(int j = 0; j < 2; j++){
		for (int i = 0; i < BUFFER_LEN; ++i)
		{
			out[i] += sine_wave[(int)tmp_index];
			tmp_index += freq*SINE_WAVE_LEN/SAMPLE_RATE;
			if (tmp_index >= SINE_WAVE_LEN)
			{
				tmp_index -= SINE_WAVE_LEN;
			}
			if (tmp_index < 0)
			{
				tmp_index += SINE_WAVE_LEN;
			}
		}
		freq += (arg[1]-512)/16;
		//printf("freq: (%i) %f %f\n", j, arg[0]/2, (arg[1]-512)/16);
		if(j == 0){
			next_index = tmp_index;
		}
		tmp_index = index;
	}
	index = next_index;
    ((float*)aux)[0] = index;	//store index value
}

//make drone-like chords
void chordifier(float *in, float *out, float *arg, void *aux){
	memset(out, 0, BUFFER_LEN*sizeof(float));
	float main_freq = half_step[(int)(arg[0]/16)];
	float freq = main_freq;
	for(int j = 0; j < 4; j++){
		float vol_div = 512*(j+1);
		float index = ((float*)aux)[j];	//get index value
		for (int i = 0; i < BUFFER_LEN; ++i)
		{
			out[i] += arg[j+4]/vol_div*sine_wave[(int)index];
			index += freq*SINE_WAVE_LEN/SAMPLE_RATE;
			if (index >= SINE_WAVE_LEN)
			{
				index -= SINE_WAVE_LEN;
			}
			if (index < 0)
			{
				index += SINE_WAVE_LEN;
			}
		}
		freq = main_freq + half_step[(int)(arg[j+1]/16)];
		((float*)aux)[j] = index;	//store index value
	}
}

//sequencer
//arguments: [BPM]
//aux buffer: [index (float)] [hold (float)] [sequence length (int)] [sequence notes (ints)]
void sequencer_effect(float *in, float *out, float *arg, void *aux){
	//static int index = 0;
	//static float hold = 0;
	float max_hold = 60.0 / (float)arg[8];	//max length to hold note
	hold += (float) BUFFER_LEN / SAMPLE_RATE;
	if (hold > max_hold)
	{
		hold -= max_hold;
		index++;
		if (index >= 8)
		{
			index = 0;
		}
	}
	if(arg[index] > 10){
		out[0] = (float)half_step[(int)(arg[index] / 16.0)];
	}else{
		out[0] = 0.0;
	}
	//printf("hold: %f, max_hold: %f, index: %d, out: %f\n", hold, max_hold, index, out[0]);
}

//sine wave generator
//argments: [freq in Hz]
//aux buffer: [index (float)]
//global variables: [sine_wave (float)]
void sine_wave_effect(float *in, float *out, float *arg, void *aux){
	float index = ((float*)aux)[0];	//get index value
	float tmp_index = index;
	float next_index;
	memset(out, 0, BUFFER_LEN*sizeof(float));
	int mul = 1;
	printf("arg: %f\n", arg[8]);
	for(int j = 0; j < 6; j++){
		for (int i = 0; i < BUFFER_LEN; ++i)
		{
			out[i] += (arg[j]/1024.0)*sine_wave[(int)tmp_index];
			tmp_index += mul*arg[8]*SINE_WAVE_LEN/SAMPLE_RATE;
			if (tmp_index >= SINE_WAVE_LEN)
			{
				tmp_index -= SINE_WAVE_LEN;
			}
			if (tmp_index < 0)
			{
				tmp_index += SINE_WAVE_LEN;
			}
		}
		mul++;
		if(j == 0){
			next_index = tmp_index;
		}
		tmp_index = index;
	}
	index = next_index;
    //log_array(out, BUFFER_LEN);
    ((float*)aux)[0] = index;	//store index value
}

//sample player
//arguments: [freq in Hz]
//aux buffer: [Midi Sample Struct]
void sample_player(float *in, float *out, float *arg, void *aux){
	midi_sample* sample = (midi_sample*)aux;	//get sample
	for (int i = 0; i < BUFFER_LEN; ++i)
    {
        out[i] = sample->sample_buf[(int)sample->index];
        sample->index += arg[0]*(sample->sample_l/SAMPLE_RATE);
        if (sample->index >= sample->sample_l)
        {
            sample->index -= sample->sample_l;
        }
        if (sample->index < 0)
        {
            sample->index += sample->sample_l;
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

void log_array(float *arr, int size){
	FILE *f = fopen("log.txt", "a");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	for (int i = 0; i < size; ++i)
	{
		fprintf(f, "%f\t", arr[i]);
	}
	//fprintf(f, "\n");
	fclose(f);
}