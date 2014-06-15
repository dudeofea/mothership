#include <stdio.h>
#include "engine.h"
#include "motherGUI.h"
#include "effects.h"

int main(int argc, char const *argv[])
{
	mgui_init();
	engine_config config = ms_init();
	init_effects(&config);
	//run main gui refresh loop
	while(mgui_refresh(&config) >= 0){ ; }
	ms_exit(&config);
	mgui_exit();
	return 0;
}