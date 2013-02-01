
#include "iduino.h"

static char initialized = 0;

void
initialize(void)
{
  if (initialized)
    return;
  initialized = 1;
  watchdog_init();
  watchdog_start();
  clock_init();

  process_init();
  process_start(&etimer_process, NULL);
}
int
main(void)
{
  setup();
  initialize();
 while(1) {
    process_run();
    watchdog_periodic();
    loop();
  }
  return 0;
}
