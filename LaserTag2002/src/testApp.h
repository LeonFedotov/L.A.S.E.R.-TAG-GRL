#ifndef _TEST_APP
#define _TEST_APP


#include "ofCore.h"
#include "appController.h"

class testApp : public ofSimpleApp{
	
	public:
		
		void setup();
		void update();
		void draw();
		void init();
		
		void keyPressed  (int key);
		void keyReleased (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased();

		float 	counter;
		float	spin;
		float	spinPct;
		int		prevMX;
		int		prevMY;
		bool 	bFirstMouseMove;
		
		appController appCtrl; 	
		
};

#endif	
