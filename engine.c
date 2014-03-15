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

void print_wire(wire w){
	if (w.module < 0)
	{
		if (w.module == JACKD_OUTPUT)
			printf("Module: Global Output\n");
		if(w.inp[0] < 0){
			if (w.inp[0] == JACKD_INPUT){
				printf("Inputs: Global\n");
			}
		}else{
			printf("Inputs: %d[%d]", w.inp[0], w.inp_ports[0]);
		}
		printf("Arguments: None\n");
	}else{
		printf("Module: %d\n", w.module);
		effect_module tmp = effects[w.module];
		printf("Inputs: ");
		for (int i = 0; i < tmp.inp_ports; ++i)
		{
			if(w.inp[i] < 0){
				if (w.inp[i] == JACKD_INPUT){
					printf(" Global");
				}
			}else{
				printf(" %d[%d]", w.inp[i], w.inp_ports[i]);
			}
		}
		printf("\n");
		printf("Arguments: ");
		for (int i = 0; i < tmp.arg_ports; ++i)
		{
			if (w.arg[i] < 0)
			{
				printf(" None");
			}else{
				printf(" %d[%d]", w.arg[i], w.arg_ports[i]);
			}
		}
		printf("\n");
	}
}

void print_all_wires(){
	for (int i = 0; i < run_order_size; ++i)
	{
		print_wire(run_order[i]);
	}
}

void print_effect(effect_module e){
	printf("Input: %d ports, %d samples each\n", e.inp_ports, e.inp_size);
	printf("Input Buffer:\n");
	for (int i = 0; i < e.inp_ports; ++i)
	{
		printf("Port %d: ", i);
		for (int j = 0; j < e.inp_size; ++j)
		{
			printf("%f ", e.inp_buf[i*e.inp_size+j]);
		}
		printf("\n");
	}
	printf("Output:%d ports, %d samples each\n", e.out_ports, e.out_size);
	printf("Output Buffer:\n");
	for (int i = 0; i < e.out_ports; ++i)
	{
		printf("Port %d: ", i);
		for (int j = 0; j < e.out_size; ++j)
		{
			printf("%f ", e.out_buf[i*e.out_size+j]);
		}
		printf("\n");
	}
	printf("Arguments: %d ports\n", e.arg_ports);
}

//allocates all data used in input/output buffers
//and preps everything so it can be run
void ms_refresh(){
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

//init mothership
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
	ms_wire_alloc(&w);
	w.inp[0] = JACKD_INPUT;
	run_order[0] = w;
}

//close mothership and free all memory
void ms_exit(void){
	//free run_order
	for (int i = 0; i < run_order_size; ++i)
	{
		if (run_order[i].inp != NULL)
			free(run_order[i].inp);
		if (run_order[i].inp_ports != NULL)
			free(run_order[i].inp_ports);
		if (run_order[i].arg != NULL)
			free(run_order[i].arg);
		if (run_order[i].arg_ports != NULL)
			free(run_order[i].arg_ports);
	}
	if (run_order != NULL)
		free(run_order);
	//free effects
	for (int i = 0; i < effects_size; ++i)
	{
		if(effects[i].inp_buf != NULL)
			free(effects[i].inp_buf);
		if(effects[i].out_buf != NULL)
			free(effects[i].out_buf);
		if(effects[i].arg_buf != NULL)
			free(effects[i].arg_buf);
	}
	if (effects != NULL)
		free(effects);
}

//runs all effects
int ms_run_engine(float* in, float* out, int len){
	wire current;		//current wire
	effect_module tmp;
	int ports, size;
	//printf("# of effects: %d, # of wires: %d\n", effects_size, run_order_size);
	for (int i = 0; i < run_order_size - 1; ++i)
	{
		//printf("iteration %d\n", i+1);
		current = run_order[i];
		//copy inputs
		ports = effects[current.module].inp_ports;
		size = effects[current.module].inp_size;
		for (int j = 0; j < ports; ++j)
		{
			if (current.inp[j] >= 0)
			{
				//printf("Input: Module %d\n", current.inp[j]);
				//copy from other module
				tmp = effects[current.inp[j]];
				//print_effect(tmp);
				memcpy(effects[current.module].inp_buf + j*size, 
					tmp.out_buf + current.inp_ports[j]*tmp.out_size, size * sizeof(float));
			}else if (current.inp[j] == JACKD_INPUT)
			{
				//copy from global input
				//printf("Input: Global\n");
				memcpy(effects[current.module].inp_buf + j*size, in, size * sizeof(float));
			}
		}
		//copy arguments
		ports = effects[current.module].arg_ports;
		size = 1;
		for (int j = 0; j < ports; ++j)
		{
			if (current.arg[j] != NO_INPUT){
				//printf("Argument: Module %d\n", current.arg[j]);
				tmp = effects[current.arg[j]];
				memcpy(effects[current.module].arg_buf + j*size, 
					tmp.out_buf + current.arg_ports[j]*tmp.out_size, size * sizeof(float));
			}
		}
		//printf("running function\n");
		//run the module function
		tmp = effects[current.module];
		tmp.effect_function(tmp.inp_buf, tmp.out_buf, tmp.arg_buf);
	}
	//output to jackd
	current = run_order[run_order_size - 1];
	if (current.module != JACKD_OUTPUT)
	{
		//printf("Something's not wired right\n");
		return -1;
	}
	if(current.inp[0] == JACKD_INPUT){
		//printf("Outputting from Global Input\n");
		//no effects
		memcpy(out, in, len * sizeof(float));
	}else{
		//printf("Outputting from Module %d\n", current.inp[0]);
		//an effect is attached to the output
		tmp = effects[current.inp[0]];
		memcpy(out, tmp.out_buf + current.inp_ports[0]*tmp.out_size, len * sizeof(float));
	}
	return 0;
}

effect_module ms_get_effect(int index){
	return effects[index];
}
int ms_get_effect_num(){
	return effects_size;
}
void ms_set_effect_arg(int index, int arg_port, float val){
	effects[index].arg_buf[arg_port] = val;
}

//Adds a effect to the list of active effects.
//Assumes there is room then doubles the size
//if neccessary.
void ms_add_effect(effect_module e){
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
	ms_refresh();
}

//TODO: re-index all wires when this happens
//Removes effect at specified index
void ms_remove_effect(int index){
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
			ms_remove_wire(i);
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
				ms_remove_wire(i);
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
				ms_remove_wire(i);
			}else if (run_order[i].arg[j] > index)
			{
				run_order[i].arg[j]--;
			}
		}
	}
}

//sets which module outputs to JACKD_OUTPUT
void ms_set_output_module(int module, int port){
	run_order[run_order_size - 1].inp[0] = module;
	run_order[run_order_size - 1].inp_ports[0] = port;
}

void ms_remove_and_insert_wire(int index, int new_index){
	//if not equal
	if (index == new_index)
		return;
	//if out of bounds
	if (index < 0 || index >= run_order_size)
		return;
	wire w = run_order[index];
	//downshift everything
	for (int i = index; i < new_index; i++)
	{
		run_order[i] = run_order[i+1];
	}
	//if out of bounds
	if (new_index < 0 || new_index >= run_order_size)
		return;
	run_order[new_index] = w;
}

//Adds a patch cable to a specified index
void add_wire_to_index(wire w, int index){
	//printf("adding wire to %d\n", index);
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

//Finds the index of the wire associated with 
//a certain module
int get_assoc_wire_index(int module){
	for (int i = 0; i < run_order_size; ++i)
	{
		if (run_order[i].module == module)
		{
			return i;
		}
	}
	return -1;
}

//Makes sure the wires have their
//dependencies met
void ms_sort_wires(){
	print_all_wires();
	for (int i = 0; i < run_order_size - 1; ++i)
	{
		int insert_index = 0;
		int assoc_index = 0;
		effect_module tmp = effects[run_order[i].module];
		//must come after all it's inputs
		for (int j = 0; j < tmp.inp_ports; ++j)
		{
			assoc_index = get_assoc_wire_index(run_order[i].inp[j]);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index;
				printf("wire moved to %d\n", insert_index);
			}
		}
		//and arguments
		for (int j= 0; j < tmp.arg_ports; ++j)
		{
			assoc_index = get_assoc_wire_index(run_order[i].arg[j]);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index;
				printf("wire moved to %d\n", insert_index);
			}
		}
		//TODO: fix sorting
		/*if (insert_index != i)
		{
			ms_remove_and_insert_wire(i, insert_index);
			i--;
			continue;
		}*/
		printf("%d belongs in %d\n", i, insert_index);
	}
}

//Adds a patch to the list of patch cables.
//Assumes there is room then doubles the size
//if neccessary. Assumes the wire's inp / inp_ports
//have already been alocated
void ms_add_wire(wire w){
	//if effect exists
	if (w.module < effects_size){
		int insert_index = 0;
		int assoc_index = 0;
		effect_module tmp = effects[w.module];
		//must come after all it's inputs
		for (int i = 0; i < tmp.inp_ports; ++i)
		{
			assoc_index = get_assoc_wire_index(w.inp[i]);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index + 1;
				//printf("wire moved to %d\n", insert_index);
			}
		}
		//and arguments
		for (int i = 0; i < tmp.arg_ports; ++i)
		{
			assoc_index = get_assoc_wire_index(w.arg[i]);
			if (insert_index <= assoc_index)
			{
				insert_index = assoc_index + 1;
				//printf("wire moved to %d\n", insert_index);
			}
		}
		add_wire_to_index(w, insert_index);
	}
}

wire ms_get_wire(int index){
	return run_order[index];
}

//Removes patch cable at specified index
void ms_remove_wire(int index){
	//if out of bounds
	if (index < 0 || index >= run_order_size)
		return;
	//downshift everything
	for (int i = index; i < run_order_size; i++)
	{
		run_order[i] = run_order[i+1];
	}
	run_order_size--;
}

//allocates memory for a wire, assumes null pointers
void ms_wire_alloc(wire *w){
	int i_size;
	int a_size;
	if (w->module != JACKD_OUTPUT){
		i_size = effects[w->module].inp_ports;
		a_size = effects[w->module].arg_ports;
	}else{
		i_size = 1;
		a_size = 0;
	}
	w->inp = (int*)malloc(i_size * sizeof(int));
	w->inp_ports = (int*)malloc(i_size * sizeof(int));
	w->arg = (int*)malloc(a_size * sizeof(int));
	w->arg_ports = (int*)malloc(a_size * sizeof(int));
	for (int i = 0; i < a_size; ++i)
	{
		w->arg[i] = NO_INPUT;
		effects[w->module].arg_buf[i] = 0.0f;
	}
}