#ifndef _ENGINE_H_
#define _ENGINE_H_

#include "effects.h"

typedef struct
{
	int module;			//The selected module to run
	int *inp;			//The selected module outputs to use as inputs
	int *inp_ports;		//Which output port to use in each selected input
	int *arg;			//The selected module outputs to use as arguments
	int *arg_ports;		//Which output port to use in each selected argument
} wire;

typedef struct
{
	int inp_ports;		//number of input buffers
	int out_ports;		//number of output buffers
	int arg_ports;		//number of argument buffers
	int inp_size;		//input buffer size
	int out_size;		//output buffer size
						//argument port size is set to 1
	float *inp_buf;		//buffer used to hold input data
	float *out_buf;		//buffer used to hold output data
	float *arg_buf;		//buffer used to hold argument data
						//argument data can also be placed directly in the buffer by way
						//of directly editing it in the UI

	void* aux;			//pointer to any auxilary arguments you might need
	int aux_size;		//size of auxilary buffer
	char* name;			//name of the effect
	void (*effect_function)(float *in, float *out, float *arg, void* aux);
} effect_module;

#define JACKD_INPUT 		-1
#define JACKD_OUTPUT		-2
#define NO_INPUT			-3

void ms_init(void);
void ms_exit(void);
void ms_refresh();
int ms_run_engine(float* in, float* out, int len);
effect_module ms_get_effect(int index);
void ms_set_effect_arg(int index, int arg_port, float val);
int ms_get_effect_num(void);
void ms_add_effect(effect_module e);
void ms_remove_effect(int index);
void ms_set_output_module(int module, int port);
void ms_remove_and_insert_wire(int index, int new_index);
void ms_sort_wires();
void ms_add_wire(wire w);
wire ms_get_wire(int index);
void ms_remove_wire(int index);
void ms_wire_alloc(wire *w);

#endif