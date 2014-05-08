#include "motherGUI.h"

#define COLOR_FRONT1	1
#define COLOR_BACK1		0

#define UP_ARROW		65
#define DOWN_ARROW		66
#define LEFT_ARROW		68
#define RIGHT_ARROW		67

#define KEY_Q			113

window_t selected_window = DETAIL;

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
	//print wire connection indicators
	int w_i = ms_get_assoc_wire_index(sel_i, config);
	if(w_i >= 0){
		//inputs
		for (int i = 0; i < config->effects[sel_i].inp_ports; ++i)
		{
			if(config->run_order[w_i].inp[i] >= 0){
				move(i+1, 0);
				addch(ACS_DIAMOND);
			}else if(config->run_order[w_i].inp[i] == JACKD_INPUT){
				move(0, 0);
				addch(ACS_DIAMOND);
			}else if(config->run_order[w_i].inp[i] == JACKD_OUTPUT){
				move(0, 30);
				addch(ACS_DIAMOND);
			}
		}
		//outputs

	}
	int pos = COLS / 4;
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
	move(LINES - 2, pos);
	addch(118 | A_ALTCHARSET); //tee
}

//draws screen and gets input
int mgui_refresh(engine_config* config){
	clear();
	//draw screen
	static int key = 0;
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
	//get input
	key = getch();
	if(key == 'q'){
		return -1;
	}
	return 0;
}

void mgui_select_window(window_t sel){
	selected_window = sel;
}