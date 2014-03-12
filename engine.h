#ifndef _ENGINE_H_
#define _ENGINE_H_

struct wire
{
	int module;			//The selected module to run
	int *inp;			//The selected module outputs to use as inputs
	int *inp_ports;		//Which output port to use in each selected input
	int *arg;			//The selected module outputs to use as arguments
	int *arg_ports;		//Which output port to use in each selected argument
};
#typedef struct wire wire

#define JACKD_INPUT 		-1
#define JACKD_OUTPUT		-2

void run_engine(float* in, float* out);

#endif