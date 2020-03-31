#ifndef _GESTURE_BRUSH_H
#define _GESTURE_BRUSH_H



// this brush is from "gesture machines" by zl
// http://www.theremediproject.com/projects/issue12/systemisgesture/
// enjoy !

#include "ofMain.h"
#include "ofAddons.h"
#include "ofVec3f.h"

//--------------------------
// constants

#include "swimmingMachine.h"
#include "baseBrush.h"

class gestureBrush : public baseBrush{
	
	public:
			
		void setupCustom();
		void clear();
		void update();

		void addPoint(float x1, float y1, bool newStroke);		
		void draw(int x, int y, int w, int h);

	protected:	
		
		void newLetter();
		void addLine(float x1, float y1, float x2, float y2);
		
		int 				oldX;
		int					oldY;
		
		swimmingMachine		sm;		
		int					nFramesSinceAdded;
		bool 				bNew;
		float 				velSmooth;
		float 				xSmooth;
		float 				ySmooth;

};

#endif	
