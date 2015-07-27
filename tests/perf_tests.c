/*
	Performance Tests for Mothership Engine
*/
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "../engine.h"

#define BUF_LEN  1024

//Adds 5 to output
void add_5(float *in, float *out, float *arg, void* aux){
	int buf_len = BUF_LEN;
	//printf("in2: \n");
	for (int i = 0; i < buf_len; ++i)
	{
		//printf("%f ", in[i]);
		out[i] = in[i] + 3.0f;
	}
	//printf("\n");
}

//test lots of effects at once x100
void test_many1(){
	struct timeval start, stop;
	double long accum;
	engine_config config = ms_init();
	float in[BUF_LEN], out[BUF_LEN];
	for (int i = 0; i < BUF_LEN; ++i)
	{
		in[i] = i * i * 0.5;	//why not?
	}
	ms_create_effect(1, 1, 0, BUF_LEN, BUF_LEN, "add_5", add_5, &config);
	wire w1 = ms_create_wire(0, &config);
	w1.inp[0] = JACKD_INPUT;
	ms_add_wire(w1, &config);
	int i;
	for (i = 1; i < 2000; ++i)
	{
		//printf("adding effect %d\n", i);
		ms_create_effect(1, 1, 0, BUF_LEN, BUF_LEN, "add_5", add_5, &config);
		wire w2 = ms_create_wire(i, &config);
		w2.inp[0] = i - 1;
		w2.inp_ports[0] = 0;
		ms_add_wire(w2, &config);
	}
	ms_set_output_module(i-1, 0, &config);
	int run_times = 1000;
	//start
	gettimeofday(&start, NULL);
	for (int i = 0; i < run_times; ++i)
	{
		ms_run_engine(in, out, BUF_LEN, &config);
	}
	//stop
	gettimeofday(&stop, NULL);
	accum = ( stop.tv_sec - start.tv_sec );
	accum +=( stop.tv_usec - start.tv_usec ) * (1.0/1000000);
	accum *= 1000; //turn to milliseconds
	printf("total time: %.1Lfms\n", accum);
	printf("average time: %.1Lfms\n", accum / run_times);
}

int main(int argc, char const *argv[])
{
	printf("starting tests\n");
	test_many1();
	return 0;
}