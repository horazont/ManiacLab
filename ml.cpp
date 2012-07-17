#include "automaton.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

int main()
{
	const int width = 100, height = 40;
	Automaton *automat = new Automaton(width, height, 1.);
	
	unsigned int oscX = 20, oscY = 10;
	double oscPhase = 0., oscStep = 0.02*M_PI;
	
	automat->printCells();
	
	while (1) {
		usleep(100000);
		oscPhase += oscStep;
		if (oscPhase > 2*M_PI)
		{
			oscPhase -= 2*M_PI;
		}
		for (unsigned int x = 0; x < width; x++) {
			automat->cellAt(x, 0)->airPressure = 1.;
			automat->cellAt(x, height-1)->airPressure = 1.;
		}
		for (unsigned int y = 0; y < height; y++) {
			automat->cellAt(0, y)->airPressure = 1.;
			automat->cellAt(width-1, y)->airPressure = 1.;
		}
		//automat->cellAt(oscX, oscY)->airPressure = sin(oscPhase) + 1.0;
		automat->cellAt((int)(sin(oscPhase) * oscX + width/2), (int)(cos(oscPhase) * oscY + height/2))->airPressure = 4.0;
		system("tput reset");
		for (unsigned int i = 0; i < 10; i++) {
			automat->updateCells();
		}
		automat->printCells();		
	}
	delete automat;
	
	return 0;
}
