#ifndef _DRIP_H
#define _DRIP_H

#include "ofMain.h"
#include "ofAddons.h"
#include "miscUtils.h"

#define MAX_DRIPS 800

class drip{
	
	public: 
	
		drip();
		void reset();
		
		//-----------------------------------	
		void setColor(float red, float green, float blue);
		bool setup(float x, float y, int imageW, int imageH, int _direction, float length, float speed, int dWidth);
		bool update();
		void startDripping();
		void stopDripping();
		bool isDripping();
		
		
		ofVec2f pre;	//previous position
		ofVec2f pos;	//our position
		ofVec2f dst;	//our destination (target)
		ofVec2f vel;	//velocity
				
		int dripWidth;
		int dripLength;
		int direction;
		
		float distance;
		float r, g, b;
		
		bool dripping;
	
};


class drips{
	
	public: 
		
		//------------------------------------
		drips();
		void setup(int w, int h);
		void setColor(unsigned char r, unsigned char g, unsigned char b);
		void clear();
		void setDirection(int dripDirection);
		int getDirection();
		void setSpeed(float dripSpeed);
		float getSpeed();
		void setWidth(int dWidth);
		int getWidth();
		bool addDrip(int x, int y);
		void updateDrips(unsigned char * pix);
		void updateDrips();
		unsigned char * getPixels();
		
	protected:

		void fastVerticalLine(unsigned char * pix, int xPos, int y0, int y1, int lineWidth,  unsigned char r, unsigned char g, unsigned char b);
		void fastHorizontalLine(unsigned char * pix, int yPos, int x0, int x1, int lineWidth,  unsigned char r, unsigned char g, unsigned char b);
		
		bool bSetup;
		int width, height, totalPixels, currentDrip, numDrips;
		unsigned char red, green, blue;
		int direction;
		int dripWidth;
		float speed;

		drip DRIP[MAX_DRIPS];
		unsigned char * pixels;

};

#endif






































