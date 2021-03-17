#ifndef _GUI_QUAD_
#define _GUI_QUAD_

#include "ofMain.h"
#include "ofAddons.h"

#include "ofCvConstants.h"
/////////////////////////
//	REQUIRES ofPoint2f to  
//	be defined as so:
/*
	typedef struct {
		float x;
		float y;
	} ofPoint2f;
*/
/////////////////////////

#include "baseGui.h"
#include "miscUtils.h"

class guiQuad : public baseGui{
	
	public:
		guiQuad();

		void setup(string _quadName);
		void readFromFile(string filePath);
		void loadSettings();
		
		void releaseAllPoints();
		//these should be in the range x(0-maxX) y(0-maxH) 
		//with 	width  = maxW
		//		height = maxH 
		void setQuadPoints( ofPoint2f * inPts );		
		
		bool selectPoint(float x, float y, float offsetX, float offsetY, float width, float height, float hitArea);
		bool updatePoint(float x, float y, float offsetX, float offsetY, float width, float height);
		
		//returns pts to width by height range 
		ofPoint2f * getScaledQuadPoints(float width, float height);
		//returns pts in 0-1 scale
		ofPoint2f * getQuadPoints();		
		void saveToFile(string filePath, string newQuadName);
		void saveToFile(string filePath);
		void draw(float x, float y, float width, float height, int red, int green, int blue, int thickness);
		void draw(float x, float y, float width, float height);

		
	protected:
		ofXML	xml;
		ofPoint2f srcZeroToOne[4];
		ofPoint2f srcScaled[4];
		string quadName;
		int selected;
};

#endif	



