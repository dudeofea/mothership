#include "motherGUI.h"

#define COLOR_FRONT1	1
#define COLOR_BACK1		0

#define UP_ARROW		65
#define DOWN_ARROW		66
#define LEFT_ARROW		68
#define RIGHT_ARROW		67

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
	noecho();
}

void mgui_exit(){
	getch();
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

void draw_detailed(engine_config* config){
	static int sel_i = 0;
	//get input
	int key = getch();
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
	move(1, 0);
	addch(ACS_DIAMOND);
	for (int i = 0; i < config->effects_size; ++i)
	{
		if(i == sel_i)
			attron(COLOR_PAIR(2));
		move(i+1, 1);
		print_fixed_string(config->effects[i].name, 17);
		attron(COLOR_PAIR(1));
	}
	//TODO: add wire connection indicators
	move(LINES - 2, 0);
	for (int i = 0; i < COLS; ++i)
	{
		addch(113 | A_ALTCHARSET);	//dash
	}
	int pos = COLS / 4;
	for (int i = 0; i < LINES - 2; ++i)
	{
		move(i, pos);
		addch(ACS_VLINE);
	}
	move(LINES - 2, pos);
	addch(118 | A_ALTCHARSET); //tee
}

void mgui_refresh(engine_config* config){
	//draw screen
	switch(selected_window){
		case DETAIL:
			draw_detailed(config);
			break;
		case GRAPH:
			break;
		case ALL:
			break;
		default: break;
	}
	draw_toolbar(stdscr);
}

void mgui_select_window(window_t sel){
	selected_window = sel;
}