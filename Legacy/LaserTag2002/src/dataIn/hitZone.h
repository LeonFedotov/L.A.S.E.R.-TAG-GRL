#ifndef _HIT_ZONE_H
#define _HIT_ZONE_H

#include "ofMain.h"
#include "ofAddons.h"

class hitZone{

	public:
	
		hitZone();
		void setActive(bool active);
		bool getActive();
		bool isHit(float x, float y);
		void setPosition(float x, float y);
		void setDimensions(float w, float h);
		void updatePoints();
		
		ofVec2f zoneDimensions[2];
		ofVec2f points[4];
		bool bActive;
		
};

#endif