#ifndef _EFFECTS_H_
#define _EFFECTS_H_
#include <fftw3.h>
#include <math.h>

#define BUFFER_LEN 1024

struct effect_module
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

	void* aux;			//pointer to any auxilary arguments you might need
	void (*effect_function)(float *in, float *out, float *arg);
};
#typedef struct effect_module effect_module

int run_effects(float* in, float* out, fftw_complex* fft_out);

#endif
