#include <stdio.h>
#include "engine.h"

//simple volume module
//buffer size is 20
void myeffect1(float *in, float *out, float *arg){
	int buf_len = 20;
	for (int i = 0; i < buf_len; ++i)
	{
		out[i] = arg[0]*in[i];
	}
}

//simple add 3 effect
//buffer size is 20
void myeffect2(float *in, float *out, float *arg){
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

int main(int argc, char const *argv[])
{
	ms_init();
	effect_module e1 = {
		1, 1, 1,
		20, 20,
		NULL, NULL, NULL,
		NULL, "test_effect1",
		myeffect1
	};
	effect_module e2 = {
		1, 1, 0,
		20, 20,
		NULL, NULL, NULL,
		NULL, "test_effect2",
		myeffect2
	};
	add_effect(e1);
	add_effect(e2);
	float in[20], out[20];
	for (int i = 0; i < 20; ++i)
	{
		in[i] = i;
	}
	printf("input: "); print_array(in, 20);
	float arg = 3.0f;
	//hardcode argument
	set_effect_arg(0, 0, arg);
	static wire w = {
		0,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w, 1, 1);
	w.inp[0] = JACKD_INPUT;
	w.arg[0] = NO_INPUT;
	add_wire(w);
	static wire w2 = {
		1,NULL,NULL,NULL,NULL
	};
	ms_wire_alloc(&w2, 1, 0);
	w2.inp[0] = 0;
	w2.inp_ports[0] = 0;
	add_wire(w2);
	set_output_module(1, 0);
	if(run_engine(in, out) < 0){
		printf("run error!\n");
	}
	printf("output: "); print_array(out, 20);
	return 0;
}