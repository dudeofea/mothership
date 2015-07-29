//usleep fix
#define _BSD_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <jack/jack.h>
#include "engine.h"
#include "effects.h"
#include "ble.h"

jack_port_t *input_port;
jack_port_t *output_port;

void process_cmds();
int process (jack_nframes_t nframes, void *arg);
jack_client_t * setup_jackd(engine_config* config);

int main(int argc, char const *argv[])
{
	// init
	engine_config config = ms_init();
	init_effects(&config);
	jack_client_t *client = setup_jackd(&config);
	// Run main refresh loop
	process_cmds(&config);
	// exit
	jack_client_close(client);
	ms_exit(&config);
	return 0;
}

//wait for commands and react accordingly by
//setting parameters, adding effects, etc
void process_cmds(engine_config* config){
	int done = 0, id;
	//int connected = 0;
	unsigned char buf[100];
	unsigned int vals[10];
	ble_connect("D0:39:72:C3:AC:AA");
	int count = 0;
	while(!done){
		//get input
		memset(buf, 0, sizeof(buf));
		int len = ble_char_read(0x0012, buf);
		if(len > 0){
			switch(buf[0]){
				case 1:		//pedal is sending values
					memset(vals, 0, sizeof(vals));
					int bitcount = 8;	//bit count in first byte
					int bufoff = 1;		//skip the command byte
					printf("[%d] ", count);
					for (int i = 0; i < 10; ++i)
					{
						vals[i] |= (buf[i+bufoff] & ((1<<bitcount)-1));
						vals[i] |= ((buf[i+1+bufoff]&0xFF)>>(bitcount+8-10))<<bitcount;
						bitcount = 8 + bitcount - 10;
						if(bitcount <= 0){
							bitcount = 8;
							bufoff++;
						}
						printf("%d ", vals[i]);
					}
					printf("\n");
					count++;
					//set the value
					for (int i = 0; i < config->effects[0].arg_ports; ++i)
					{
						ms_set_effect_arg(0, i, (float)vals[i], config);
					}
					break;
				case 2:		//pedal is requesting module details
					//get module id
					id = buf[1];
					//response breakdown:
					//<len> <in_ports> <out_ports> <arg_ports> <color> <name>
					buf[0] = 0;
					buf[1] = config->effects[id].inp_ports;
					buf[2] = config->effects[id].out_ports;
					buf[3] = config->effects[id].arg_ports;
					//RGB
					buf[4] = 255;
					buf[5] = 128;
					buf[6] = 0;
					//name
					strcpy((char*)buf+7, config->effects[id].name);
					buf[0] = strlen(config->effects[id].name) + 5;
					printf("len: %d\n", buf[0]);
					ble_char_write(0x0016, buf, buf[0]);
					break;
			}
			/*//edit an effect's parameters
			if(strncmp(cmd_buf, "edit ", 5) == 0){
				int id, param;
				float val;
				sscanf(&cmd_buf[5], "%d %d %f", &id, &param, &val);
				printf("id: %d param:%d val:%f\n", id, param, val);
				//error checks
				if(id < 0 || id >= config->effects_size){
					return;
				}
				if(param < 0 || param >= config->effects[id].arg_ports){
					return;
				}
				//set the value
				ms_set_effect_arg(id, param, val, config);
			//get setup info
			}else if(strncmp(cmd_buf, "info", 4) == 0){
				//printf("effects: %d\n", config->effects_size);
				for (int i = 0; i < config->effects_size; ++i)
				{
					printf("id: %d, inports: %d, outports: %d, argports: %d, name: %s\n",
						i,
						config->effects[i].inp_ports,
						config->effects[i].out_ports,
						config->effects[i].arg_ports,
						config->effects[i].name);
				}
			//ditch
			}else if(strncmp(cmd_buf, "exit", 4) == 0){
				return;
			}*/
		};
		usleep(1000000);
	}
}

//setup jackd client
jack_client_t *setup_jackd(engine_config* config){
	jack_client_t *client;
	/* Setup jack client */
	//try to become a client of the JACK server
	const char **ports;
	jack_status_t err;
	client = jack_client_open ("pedal", JackNullOption, &err);
	if(client == NULL){
		fprintf (stderr, "jack server not running?\n");
		exit(1);
	}
	//bind middle man callback
	jack_set_process_callback (client, process, config);
	//create two ports
	input_port = jack_port_register (client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	output_port = jack_port_register (client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	//activate client
	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit(1);
	}
	//connect the ports
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
	return client;
}

//process jackd input samples and output samples
int process (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
	jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);
	//get configuration
	engine_config* config = (engine_config*)arg;
	//Run all pedal effects
	ms_run_engine(in, out, BUFFER_LEN, config);
	return 0;
}