#include "automaton.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

int main()
{
	const unsigned int width = 80, height = 2;
	Automaton *automat = new Automaton(width, height, 0.5, 0.995, 5.0);
	
	unsigned int oscX = 20, oscY = 10;
	double oscPhase[2] = { 0., 0.}, oscStep = 0.01*M_PI;
	unsigned int oscCount = 2;
	bool oscillating = true;
	
	while (1) {
		usleep(100000);
		//automat->cellAt(oscX, oscY)->airPressure = sin(oscPhase) + 1.0;
		//automat->cellAt(oscX, oscY)->flow[0] = -1.0;
		//automat->cellAt(oscX, oscY)->flow[1] = 0.;
		
		system("tput reset");
		for (unsigned int i = 0; i < 25; i++) {
			if (oscillating) {
				oscPhase[0] += oscStep;
				oscPhase[1] += oscStep*1.5;
				for (unsigned int oscI = 0; oscI < oscCount; oscI++) {
					if (oscPhase[oscI] > 2*M_PI)
					{
						oscPhase[oscI] -= 2*M_PI;
					}
				}
				
				// automat->cellAt((int)(sin(oscPhase) * oscX + width/2), (int)(cos(oscPhase) * oscY + height/2))->airPressure = 10.0;
				//automat->cellAt(width / 2, height / 2)->airPressure = ;
			}
			/*for (unsigned int x = 0; x < width; x++) {
				automat->cellAt(x, 0)->airPressure = 5.;
				automat->cellAt(x, height-1)->airPressure = 5.;
			}*/
			for (unsigned int y = 0; y < height; y++) {
				const double phase = (y%2 == 0) ? (oscPhase[1]) : (oscPhase[0]);
				//const double phase = oscPhase[0];
				automat->cellAt(0, y)->airPressure = 1.0 * sin(phase) + 5.0;
				//automat->cellAt(width - 1, y)->airPressure = 5.;
			}
			automat->updateCells();
		}
		automat->printFlow();
		automat->printCells256(4.5, 5.5);
	}
	delete automat;
	
	return 0;
}
