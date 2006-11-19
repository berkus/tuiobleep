#include "TuioApp.h"
#include <time.h>

static const timespec main_rate = { 0, 500000000 }; // 1/2 second
volatile bool quit_flag = false;

int main(int, char **)
{
	TuioApp app;

	while(!quit_flag)
	{
		nanosleep(&main_rate, NULL);
	}

	return 0;
}
