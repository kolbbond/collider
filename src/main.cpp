// Main Driver for Collider V2
//
// Will run actual chess engine and interface with a graphics driver for 
// better object oriented approach.
// October 12, 2017
// Dylan James Kolb-Bond


#include <iostream>
#include "collider.h"
#include "display.h"
#include "sdlp.h"

double life;

int main(int argc, char *args[]) {
	(void)argc;
	(void)args;

	/* Main game loop */
	
	bool stop = false;
	collider();
	while (!stop) {
		SDL_WaitEvent(NULL);
		displayAll();
	}
	closeSDL();
	return 0;

}