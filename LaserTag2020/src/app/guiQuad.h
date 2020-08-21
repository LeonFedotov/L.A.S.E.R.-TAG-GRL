#ifndef _GUI_QUAD_
#define _GUI_QUAD_

#include "ofMain.h"

#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"
/////////////////////////
//	REQUIRES ofVec2f to  
//	be defined as so:
/*
	typedef struct {
		float x;
		float y;
	} ofVec2f;
*/
/////////////////////////

#include "baseGui.h"

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
		void setQuadPoints( ofPoint * inPts );
		ofMatrix4x4 getHomography(vector<cv::Point2f> cvSrcPos, vector<cv::Point2f> cvDstPos);
		ofMatrix4x4 getHomography(ofPoint src[4], ofPoint dst[4]);
		bool selectPoint(float x, float y, float offsetX, float offsetY, float width, float height, float hitArea);
		bool updatePoint(float x, float y, float offsetX, float offsetY, float width, float height);
		
		//returns pts to width by height range 
		ofPoint * getScaledQuadPoints(float width, float height);
		//returns pts in 0-1 scale
		ofPoint * getQuadPoints();		
		void saveToFile(string filePath, string newQuadName);
		void saveToFile(string filePath);
		void draw(float x, float y, float width, float height, int red, int green, int blue, int thickness);
		void draw(float x, float y, float width, float height);

		
	protected:
		ofxXmlSettings	xml;
		ofPoint srcZeroToOne[4];
		ofPoint srcScaled[4];
		string quadName;
		int selected;
};

#endif	



