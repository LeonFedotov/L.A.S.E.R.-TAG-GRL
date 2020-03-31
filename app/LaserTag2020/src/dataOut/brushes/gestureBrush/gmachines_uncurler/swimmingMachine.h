#ifndef _SWIMMING_MACHINE
#define _SWIMMING_MACHINE

// this brush is from "gesture machines" by zl
// http://www.theremediproject.com/projects/issue12/systemisgesture/
// enjoy !

#include "ofMain.h"
#include "swimStroke.h"
#include "maxStroke.h"

class swimmingMachine {
	
	public:
		
		swimmingMachine();
		
		void setup();
		void update();
		void draw();
		void start();
		void end();
		void clear();
		
		void mouseDragged(int x, int y, float button);
		void mousePressed(int x, int y, float button);
		void mouseReleased();
		
		int 		maxNumStrokes;
		int 		nStrokes;
		swimStroke	* angle;
		maxStroke 	stroke;
		
};

#endif	
