/** file jack_client.c
*
* mostly copied off of the simple_client.c code since it was the
* only thing around.
*/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include <jack/jack.h>
#include <fftw3.h>
#include <math.h>

#include "engine.h"
#include "print_array.h"

jack_port_t *input_port;
jack_port_t *output_port;

fftw_plan p;
fftw_complex fft_in[BUFFER_LEN], fft_out[BUFFER_LEN];

#define SINE_WAVE_LEN   BUFFER_LEN
float sine_wave_buf[SINE_WAVE_LEN];

static bool running = true;

/**
* Sigint handler for shutting down client
*/
void intHandler(){
	running = false;
}

/**
* The process callback for this JACK application.
* It is called by JACK at the appropriate times.
*/
int process (jack_nframes_t nframes, void *arg)
{
        jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
        jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);

	/*//Copy data to FFT-input buffer
	for(int i = 0; i < BUFFER_LEN; i++){
		fft_in[i][0] = in[i];
	}
	//Run FFT
	fftw_execute(p);*/
	//Run all pedal effects
	run_engine(in, out, BUFFER_LEN);
	//print_array(fft_out, BUFFER_LEN);
    return 0;
}

/**
 * This is the shutdown callback for this JACK application.
 * It is called by JACK if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
	//endwin();
	fftw_destroy_plan(p);
    ms_exit();
    exit (1);
}

//simple generic volume module
void volume(float *in, float *out, float *arg){
    printf("Multiplying %d values by %f\n", (int)arg[0], arg[1]);
    for (int i = 0; i < (int)arg[0]; ++i)
    {
        out[i] = arg[1]*in[i];
    }
}

//simple generic adder
void adder_effect(float *in, float *out, float *arg){
    printf("Adding %f to  %d Values\n", arg[1], (int)arg[0]);
    for (int i = 0; i < (int)arg[0]; ++i)
    {
        printf("val2: %f\n", in[i]);
        out[i] = in[i] + arg[1];
    }
}

//sine wave generator
void sine_wave(float *in, float *out, float *arg){
    printf("Generating %f\n", arg[0]);
    static float index = 0;
    for (int i = 0; i < BUFFER_LEN; ++i)
    {
        out[i] = sine_wave_buf[(int)index];
        index += arg[0];
        if (index > SINE_WAVE_LEN)
        {
            index -= SINE_WAVE_LEN;
        }
        if (index < 0)
        {
            index += SINE_WAVE_LEN;
        }
    }
    //printf("index: %f\n", index);
    //printf("value: %f\n", sine_wave_buf[(int)index]);
}

int main (int argc, char *argv[])
{
        jack_client_t *client;
        const char **ports;

        /* try to become a client of the JACK server */

	jack_status_t err;
        client = jack_client_open ("pedal", JackNullOption, &err);
	if(0){
                fprintf (stderr, "jack server not running?\n");
                return 1;
        }

        /* tell the JACK server to call `process()' whenever
           there is work to be done.
        */

       jack_set_process_callback (client, process, 0);

        /* tell the JACK server to call `jack_shutdown()' if
           it ever shuts down, either entirely, or if it
           just decides to stop calling us.
        */

        jack_on_shutdown (client, jack_shutdown, 0);

        //display the current sample rate.
        printf ("engine sample rate: %" PRIu32 "\n",
                jack_get_sample_rate (client));
	//Initialize FFT
	p = fftw_plan_dft_1d(BUFFER_LEN, fft_in, fft_out, FFTW_FORWARD, FFTW_MEASURE);

	//initscr();
    //init sine buffer
    for (int i = 0; i < SINE_WAVE_LEN; ++i)
    {
        sine_wave_buf[i] = (float)sin(2* i * 3.141592654 / SINE_WAVE_LEN);
    }
    ms_init();
    effect_module e1 = {
        0, 1, 1,
        0, BUFFER_LEN,
        NULL, NULL, NULL,
        NULL, "sine generator",
        sine_wave
    };
    add_effect(e1);
    add_effect(e1);
    effect_module e2 = {
        1, 1, 2,
        1, 1,
        NULL, NULL, NULL,
        NULL, "adder",
        adder_effect
    };
    add_effect(e2);
    effect_module e3 = {
        1, 1, 2,
        1, 1,
        NULL, NULL, NULL,
        NULL, "volume",
        volume
    };
    add_effect(e3);
    wire w = {
        0,NULL,NULL,NULL,NULL
    };
    ms_wire_alloc(&w);
    w.arg[0] = NO_INPUT;
    add_wire(w);
    wire w2a = {
        3,NULL,NULL,NULL,NULL
    };
    ms_wire_alloc(&w2a);
    w2a.inp[0] = 0;
    w2a.inp_ports[0] = 0;
    add_wire(w2a);
    wire w2 = {
        2,NULL,NULL,NULL,NULL
    };
    ms_wire_alloc(&w2);
    w2.inp[0] = 3;
    w2.inp_ports[0] = 0;
    add_wire(w2);
    wire w3 = {
        1,NULL,NULL,NULL,NULL
    };
    ms_wire_alloc(&w3);
    w3.arg[0] = 2;
    w3.arg_ports[0] = 0;
    add_wire(w3);
    set_effect_arg(0, 0, 4.0f);
    set_effect_arg(2, 0, 1.0f);
    set_effect_arg(2, 1, 10.0f);
    set_effect_arg(3, 0, 1.0f);
    set_effect_arg(3, 1, 3.0f);
    set_output_module(0, 0);
         /* create two ports */

        input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

        /* tell the JACK server that we are ready to roll */

        if (jack_activate (client)) {
                fprintf (stderr, "cannot activate client");
                return 1;
        }

        /* connect the ports. Note: you can't do this before
           the client is activated, because we can't allow
           connections to be made to clients that aren't
           running.
        */

        if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput)) == NULL) {
                fprintf(stderr, "Cannot find any physical capture ports\n");
                exit(1);
        }

        if (jack_connect (client, ports[0], jack_port_name (input_port))) {
                fprintf (stderr, "cannot connect input ports\n");
        }

        free (ports);

        if ((ports = jack_get_ports (client, NULL, NULL, JackPortIsPhysical|JackPortIsInput)) == NULL) {
                fprintf(stderr, "Cannot find any physical playback ports\n");
                exit(1);
        }

        if (jack_connect (client, jack_port_name (output_port), ports[0])) {
                fprintf (stderr, "cannot connect output ports\n");
        }

        free (ports);

	//Set signal handler
	signal(SIGINT, intHandler);
	while(running){
        	sleep (5);
	}
        jack_client_close (client);
	//endwin();
	fftw_destroy_plan(p);
    ms_exit();
        exit (0);
}
