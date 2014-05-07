#ifndef _MOTHERGUI_H_
#define _MOTHERGUI_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include "engine.h"

//selected window
typedef enum {DETAIL, GRAPH, ALL} window_t;

void mgui_init(void);
void mgui_exit(void);
int mgui_refresh(engine_config* config);
void mgui_select_window(window_t sel);

#endif