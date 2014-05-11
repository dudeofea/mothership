#include "motherGUI.h"

#define COLOR_FRONT1	1
#define COLOR_BACK1		0

#define UP_ARROW		65
#define DOWN_ARROW		66
#define LEFT_ARROW		68
#define RIGHT_ARROW		67

#define KEY_Q			113

window_t selected_window = DETAIL;
int redraw = 1;
int term_height = 0;
int term_width = 0;

void mgui_init(){
	initscr();			/* Start curses mode 		*/
	if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}
	start_color();			/* Start color 			*/
	init_pair(1, COLOR_FRONT1, COLOR_BACK1);
	init_pair(2, COLOR_BACK1, COLOR_FRONT1);
	init_pair(3, COLOR_BLUE, COLOR_BACK1);
	noecho();	//remove key stroke echo
	curs_set(0); //remove cursor
}

void mgui_exit(){
	echo();
	endwin();
}

void draw_toolbar(WINDOW *win){
	attron(COLOR_PAIR(1));
	//Detailed
	move(LINES - 1, 0);
	attron(COLOR_PAIR(2));
	printw("D");
	attron(COLOR_PAIR(1));
	printw("etailed ");
	//Graph
	move(LINES - 1, COLS / 5);
	attron(COLOR_PAIR(2));
	printw("G");
	attron(COLOR_PAIR(1));
	printw("raph ");
	//All effects
	move(LINES - 1, 2 * COLS / 5);
	attron(COLOR_PAIR(2));
	printw("A");
	attron(COLOR_PAIR(1));
	printw("ll Effects ");
	//Quit
	move(LINES - 1, 4 * COLS / 5);
	attron(COLOR_PAIR(2));
	printw("Q");
	attron(COLOR_PAIR(1));
	printw("uit ");
}

void print_fixed_string(char* str, int len){
	int str_len = strlen(str);
	if(str_len >= len){
		for (int i = 0; i < len; ++i)
		{
			addch(str[i]);
		}
	}else{
		printw(str);
		for (int i = 0; i < len - str_len; ++i)
		{
			addch(' ');
		}
	}
}

void draw_detailed(engine_config* config, int key){
	static int sel_i = 0;
	//get input
	if(key == 27 && getch() == 91){
		//arrow key
		key = getch();
		switch(key){
			case UP_ARROW:
				sel_i--;
				if(sel_i < 0){ sel_i = config->effects_size - 1; }
				break;
			case DOWN_ARROW:
				sel_i++;
				if(sel_i >= config->effects_size){ sel_i = 0; }
				break;
		}
	}
	attron(COLOR_PAIR(1));
	//print header
	move(0, 1); printw(": in    |   out :");
	//print effect names
	for (int i = 0; i < config->effects_size; ++i)
	{
		if(i == sel_i)	//highlight selected
			attron(COLOR_PAIR(2));
		move(i+1, 1);
		print_fixed_string(config->effects[i].name, 17);
		attron(COLOR_PAIR(1));
	}
	int pos = COLS / 4;
	//print wire connection indicators
	int w_i = ms_get_assoc_wire_index(sel_i, config);
	if(w_i >= 0){
		//inputs
		for (int i = 0; i < config->effects[sel_i].inp_ports; ++i)
		{
			if(config->run_order[w_i].inp[i] >= 0){
				move(i+1, 0);
				addch(ACS_DIAMOND);
				move(i+8, pos+2);
				printw("m:%d p:%d", config->run_order[w_i].inp[i], config->run_order[w_i].inp_ports[i]);
			}else if(config->run_order[w_i].inp[i] == JACKD_INPUT){
				move(0, 0);
				addch(ACS_DIAMOND);
				move(i+8, pos+2);
				printw("m:GLOBAL_INPUT");
			}else{
				move(i+8, pos+2);
				printw("m:NO_INPUT");
			}
		}
		int output_num = 0;
		//outputs
		for (int i = 0; i < config->run_order_size; i++){
			int m = config->run_order[i].module;
			if(m >= 0){
				//another module
				for (int j = 0; j < config->effects[m].inp_ports; ++j)
				{
					if(config->run_order[i].inp[j] == sel_i){
						move(m+1, 18);
						addch(ACS_DIAMOND);
						move(output_num+8, pos*2+2);
						printw("m:%d p:%d", config->run_order[i].module, j);
						output_num++;
					}
				}
			}else{
				//global output
				move(0, 18);
				addch(ACS_DIAMOND);
				move(output_num+8, pos*2+2);
				printw("m:GLOBAL_OUTPUT");
				output_num++;
			}
		}
	}else{
		move(8, pos+2);
		printw("NOT SETUP");
	}
	//print current effect module info
	move(1, pos+2);
	printw(config->effects[sel_i].name);
	move(3, pos+2);
	printw("inputs:     %d", config->effects[sel_i].inp_ports);
	move(4, pos+2);
	printw("input size: %d", config->effects[sel_i].inp_size);
	move(3, pos*2+2);
	printw("outputs:     %d", config->effects[sel_i].out_ports);
	move(4, pos*2+2);
	printw("output size: %d", config->effects[sel_i].out_size);
	move(3, pos*3+2);
	printw("argumerts:   %d", config->effects[sel_i].arg_ports);
	move(4, pos*3+2);
	printw("argumnt size:%d", 1);
	//print connections
	move(6, pos+1);
	printw("-connections");
	for (int i = pos+13; i < COLS; ++i)
	{
		addch('-');
	}
	//print horiz line
	move(LINES - 2, 0);
	for (int i = 0; i < COLS; ++i)
	{
		addch(113 | A_ALTCHARSET);	//dash
	}
	//print vert line
	for (int i = 0; i < LINES - 2; ++i)
	{
		move(i, pos);
		addch(ACS_VLINE);
	}
	//print right pointing tee
	move(6, pos);
	addch(ACS_LTEE);
	//print bottom tee
	move(LINES - 2, pos);
	addch(118 | A_ALTCHARSET); //tee
}

//draws screen and gets input
int mgui_refresh(engine_config* config){
	static int key = 0;
	if(redraw){
		clear();
		//draw screen
		switch(selected_window){
			case DETAIL:
				draw_detailed(config, key);
				break;
			case GRAPH:
				break;
			case ALL:
				break;
			default: break;
		}
		draw_toolbar(stdscr);
		redraw = 0;
	}else{
		//check for resize
		int h, w;
		getmaxyx(stdscr, h, w);
		if(h != term_height || w != term_width){
			term_height = h;
			term_width = w;
			redraw = 1;
		}
	}
	//get input
	key = getch();
	if(key == 'q'){
		return -1;
	}
	if(key > 0){
		redraw = 1;
	}
	return 0;
}

void mgui_select_window(window_t sel){
	selected_window = sel;
}