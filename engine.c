#include "engine.h"
#include "effects.h"

int effects_alloc;			//size of effect_module buffer
int effects_size;			//number of active effects
effect_module *effects;		//all active effects

int run_order_alloc;		//size of run_order buffer
int run_order_size;			//number of wire patches
wire *run_order;			//how to run the program
//the last item in run_order contains which module outputs
//to the global JACKD output

//allocates all data used in input/output buffers
//and preps everything so it can be run
void refresh(){
	//allocate buffers if they don't exist
	for (int i = 0; i < effects_size; ++i)
	{
		if(effects[i].inp_buf == NULL)
			effects[i].inp_buf = (float*)malloc(effects[i].inp_size * effects[i].inp_ports * sizeof(float));
		if(effects[i].out_buf == NULL)
			effects[i].out_buf = (float*)malloc(effects[i].out_size * effects[i].out_ports * sizeof(float));
		if(effects[i].arg_buf == NULL)
			effects[i].arg_buf = (float*)malloc(effects[i].arg_size * effects[i].arg_ports * sizeof(float));
	}
}

//runs all effects
void run_engine(float* in, float* out){
	wire current;		//current wire
	effect tmp;
	int ports, size, inp_port;
	for (int i = 0; i < run_order_size - 1; ++i)
	{
		current = run_order[i];
		//copy inputs
		ports = effects[current.module].inp_ports;
		size = effects[current.module].inp_size;
		for (int i = 0; i < ports; ++i)
		{
			if (current.inp[i] >= 0)
			{
				//copy from other module
				tmp = effects[current.inp[i]];
				memcpy(effects[current.module].inp_buf + i*size, 
					tmp.out_buf + current.inp_ports[i]*tmp.out_size, size * sizeof(float));
			}else if (current.inp[i] == JACKD_INPUT)
			{
				//copy from global input
				memcpy(effects[current.module].inp_buf + i*size, in, size * sizeof(float));
			}
		}
		//copy arguments
		ports = effects[current.module].arg_ports;
		size = effects[current.module].arg_size;
		for (int i = 0; i < ports; ++i)
		{
			tmp = effects[current.arg[i]];
			memcpy(effects[current.module].arg_buf + i*size, 
				tmp.out_buf + current.arg_ports[i]*tmp.out_size);
		}
		//run the module function
		current.effect_function(current.inp_buf, current.out_buf, current.arg_buf);
	}
	//output to jackd
	current = run_order[run_order_size - 1];
	tmp = effects[current.inp[0]];
	memcpy(out, tmp.out_buf + current.inp_ports[0]*tmp.out_size, BUFFER_LEN * sizeof(float));
	return;
}

//Adds a effect to the list of active effects.
//Assumes there is room then doubles the size
//if neccessary.
void add_effect(effect_module e){
	e.inp_buf = NULL;
	e.out_buf = NULL;
	e.arg_buf = NULL;
	effects[effects_size++] = e;
	if (effects_size > effects_alloc)
	{
		effects_alloc *= 2;		//double array size
		effects = (effect_module*)realloc(effects, 
			effects_alloc * sizeof(effect_module));
	}
}

//Removes effect at specified index
void remove_effect(int index){
	//if out of bounds
	if (index < 0 || index >= effects_size)
		return;
	//downshift everything
	for (int i = index; i < effects_size - 2; i++)
	{
		effects[i] = effects[i+1];
	}
}

//Adds a patch to the list of patch cables.
//Assumes there is room then doubles the size
//if neccessary. Assumes the wire's inp / inp_ports
//have already been alocated
void add_wire(wire w){
	//if effect exists
	if (w.module < effects_size){
		//TODO: fix so that it adds in correct spot to
		//satisfy dependencies
		run_order[run_order_size++] = w;
		if (run_order_size > run_order_alloc)
		{
			run_order_alloc *= 2;		//double array size
			run_order = (wire*)realloc(run_order, 
				run_order_alloc * sizeof(wire));
		}
	}
}

//Removes patch cable at specified index
void remove_wire(int index){
	//if out of bounds
	if (index < 0 || index >= run_order_size)
		return;
	//downshift everything
	for (int i = index; i < run_order_size - 2; i++)
	{
		run_order[i] = run_order[i+1];
	}
}