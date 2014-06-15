#include "effects.h"

void init_effects(engine_config* config){
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect1",
		myeffect1
	};
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, 0,
		"test_effect2",
		myeffect2
	};
	ms_add_effect(e1, config);
	ms_add_effect(e2, config);
	e2.name = "some_other_effect";
	ms_add_effect(e2, config);
	e2.name = "sine generator";
	ms_add_effect(e2, config);
	e2.name = "wubber";
	ms_add_effect(e2, config);
	e2.name = "crazy effect";
	ms_add_effect(e2, config);
	e2.name = "really really long name";
	ms_add_effect(e2, config);
	wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, config);
	w.inp[0] = JACKD_INPUT;
	ms_add_wire(w, config);
	wire w2 = {
		1, NULL, NULL, NULL, NULL
	};
	ms_wire_alloc(&w2, config);
	w2.inp[0] = 0;
	ms_add_wire(w2, config);
	ms_set_output_module(1, 0, config);
}

//simple volume module
//buffer size is 20
void myeffect1(float *in, float *out, float *arg, void* aux){
	int buf_len = 20;
	for (int i = 0; i < buf_len; ++i)
	{
		out[i] = arg[0]*in[i];
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