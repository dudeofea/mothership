#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "engine.h"

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
		if(effects[i].inp_buf == NULL){
			effects[i].inp_buf = (float*)malloc(effects[i].inp_size * effects[i].inp_ports * sizeof(float));
		}
		if(effects[i].out_buf == NULL){
			effects[i].out_buf = (float*)malloc(effects[i].out_size * effects[i].out_ports * sizeof(float));
		}
		if(effects[i].arg_buf == NULL){
			effects[i].arg_buf = (float*)malloc(1 * effects[i].arg_ports * sizeof(float));
		}
	}
}

void ms_init(){
	effects_alloc = 10;
	effects_size = 0;
	effects = (effect_module*)malloc(effects_alloc * sizeof(effect_module));
	run_order_alloc = 10;
	run_order_size = 1;
	run_order = (wire*)malloc(run_order_alloc * sizeof(wire));
	//set output wire to input
	wire w = {
		JACKD_OUTPUT, NULL, NULL, NULL, NULL
	};
	ms_wire_alloc(&w, 1, 1);
	w.inp[0] = JACKD_INPUT;
	run_order[0] = w;
}

//runs all effects
int run_engine(float* in, float* out){
	wire current;		//current wire
	effect_module tmp;
	int ports, size;
	printf("# of effects: %d, # of wires: %d\n", effects_size, run_order_size);
	for (int i = 0; i < run_order_size - 1; ++i)
	{
		current = run_order[i];
		//copy inputs
		ports = effects[current.module].inp_ports;
		size = effects[current.module].inp_size;
		for (int j = 0; j < ports; ++j)
		{
			if (current.inp[j] >= 0)
			{
				//copy from other module
				tmp = effects[current.inp[j]];
				memcpy(effects[current.module].inp_buf + j*size, 
					tmp.out_buf + current.inp_ports[j]*tmp.out_size, size * sizeof(float));
			}else if (current.inp[j] == JACKD_INPUT)
			{
				//copy from global input
				memcpy(effects[current.module].inp_buf + j*size, in, size * sizeof(float));
			}
		}
		//copy arguments
		ports = effects[current.module].arg_ports;
		size = 1;
		for (int j = 0; j < ports; ++j)
		{
			if (current.arg[j] != NO_INPUT){
				tmp = effects[current.arg[j]];
				memcpy(effects[current.module].arg_buf + j*size, 
					tmp.out_buf + current.arg_ports[j]*tmp.out_size, size * sizeof(float));
			}
		}
		//run the module function
		tmp = effects[current.module];
		tmp.effect_function(tmp.inp_buf, tmp.out_buf, tmp.arg_buf);
	}
	//output to jackd
	current = run_order[run_order_size - 1];
	if (current.module != JACKD_OUTPUT)
	{
		return -1;
	}
	if(current.inp[0] == JACKD_INPUT){
		//no effects
		memcpy(out, in, BUFFER_LEN * sizeof(float));
	}else{
		//an effect is attached to the output
		tmp = effects[current.inp[0]];
		memcpy(out, tmp.out_buf + current.inp_ports[0]*tmp.out_size, BUFFER_LEN * sizeof(float));
	}
	return 0;
}

effect_module get_effect(int index){
	return effects[index];
}
int get_effect_num(){
	return effects_size;
}
void set_effect_arg(int index, int arg_port, float val){
	effects[index].arg_buf[arg_port] = val;
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
	refresh();
}

//TODO: re-index all wires when this happens
//Removes effect at specified index
void remove_effect(int index){
	//if out of bounds
	if (index < 0 || index >= effects_size)
		return;
	//deallocate
	free(effects[index].inp_buf);
	effects[index].inp_buf = NULL;
	free(effects[index].out_buf);
	effects[index].out_buf = NULL;
	free(effects[index].arg_buf);
	effects[index].arg_buf = NULL;
	//downshift everything
	for (int i = index; i < effects_size - 2; i++)
	{
		effects[i] = effects[i+1];
	}
	//remove all wires referencing that module
	for (int i = 0; i < run_order_size; ++i)
	{
		//remove if equal or decrement if larger
		if (run_order[i].module == index)
		{
			remove_wire(i);
		}else if (run_order[i].module > index)
		{
			run_order[i].module--;
		}
		//remove if referencing
		effect_module tmp = effects[run_order[i].module];
		for (int j = 0; j < tmp.inp_ports; ++j)
		{
			//remove if equal or decrement if larger
			if (run_order[i].inp[j] == index)
			{
				remove_wire(i);
			}else if (run_order[i].inp[j] > index)
			{
				run_order[i].inp[j]--;
			}
		}
		for (int j = 0; j < tmp.arg_ports; ++j)
		{
			//remove if equal or decrement if larger
			if (run_order[i].arg[j] == index)
			{
				remove_wire(i);
			}else if (run_order[i].arg[j] > index)
			{
				run_order[i].arg[j]--;
			}
		}
	}
}

//sets which module outputs to JACKD_OUTPUT
void set_output_module(int module, int port){
	run_order[run_order_size - 1].inp[0] = module;
	run_order[run_order_size - 1].inp_ports[0] = port;
}

//Adds a patch cable to a specified index
void add_wire_to_index(wire w, int index){
	printf("adding wire to %d\n", index);
	run_order_size++;
	if (run_order_size > run_order_alloc)
	{
		run_order_alloc *= 2;		//double array size
		run_order = (wire*)realloc(run_order, 
			run_order_alloc * sizeof(wire));
	}
	//upshift everything
	for (int i = run_order_size - 1; i >= index; i--)
	{
		run_order[i+1] = run_order[i];
	}
	run_order[index] = w;
}

//Adds a patch to the list of patch cables.
//Assumes there is room then doubles the size
//if neccessary. Assumes the wire's inp / inp_ports
//have already been alocated
void add_wire(wire w){
	//if effect exists
	if (w.module < effects_size){
		int insert_index = 0;
		effect_module tmp = effects[w.module];
		//must come after all it's inputs
		for (int i = 0; i < tmp.inp_ports; ++i)
		{
			if (insert_index <= w.inp[i])
			{
				insert_index = w.inp[i] + 1;
			}
		}
		//and arguments
		for (int i = 0; i < tmp.arg_ports; ++i)
		{
			if (insert_index <= w.arg[i])
			{
				insert_index = w.arg[i] + 1;
			}
		}
		add_wire_to_index(w, insert_index);
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

//allocates memory for a wire, assumes null pointers
void ms_wire_alloc(wire *w, int i_size, int a_size){
	w->inp = (int*)malloc(i_size * sizeof(int));
	w->inp_ports = (int*)malloc(i_size * sizeof(int));
	w->arg = (int*)malloc(a_size * sizeof(int));
	w->arg_ports = (int*)malloc(a_size * sizeof(int));
}