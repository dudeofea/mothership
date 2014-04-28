#include "motherGUI.h"

#define COLOR_FRONT1	1
#define COLOR_BACK1		0

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
}

void mgui_exit(){
	getch();
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

void draw_detailed(engine_config* config){
	attron(COLOR_PAIR(1));
	move(1, 0);
	addch(ACS_DIAMOND);
	for (int i = 0; i < config->effects_size; ++i)
	{
		move(i+1, 1);
		printw(config->effects[i].name);
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