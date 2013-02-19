
#include "contiki.h"
static char initialized = 0;
void
initialize(void)
{

	if (initialized)
		return;
	//init(); //arduino init
	initialized = 1;
	watchdog_init();
	watchdog_start();
	clock_init();

	process_init();
	process_start(&etimer_process, NULL);
	//ctimer_init();
}
int
main(void)
{


	initialize();
	setup();
	while(1) {
		process_run();
		watchdog_periodic();
		loop();
	}
	return 0;
}
