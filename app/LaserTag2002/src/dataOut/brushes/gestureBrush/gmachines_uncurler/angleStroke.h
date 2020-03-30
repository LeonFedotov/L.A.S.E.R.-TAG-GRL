#ifndef _ANGLE_STROKE
#define _ANGLE_STROKE


#include "ofMain.h"
#include "maxStroke.h"

class angleStroke {
	
	public:
	
		angleStroke();
		
		void setup();
		virtual void draw(){};
		virtual void start(){};
		virtual void update();
		void addPoint(ofVec3f position);
		void start (ofVec3f position);
		
		void fitToVec3fs (ofVec3f * pts, int nPts, int nTargetAngles);
		ofVec3f inline findPtForPct(float pct, ofVec3f * pts, int nInputPts, float length);
		
		bool 		bFitted;
		
		int 		nPts;
		float 		* angles;
		float 		* widths;
		int 		maxNumPts;
		float 		lengthSection;
		ofVec3f 	startPosition;
		ofVec3f 	currentPosition;
		float 		currentAngle;
		
		inline double returnAngle(ofVec3f pointdata, ofVec3f pointdata1){
			return (double)atan2((pointdata1.y - pointdata.y), pointdata1.x - pointdata.x);
		}
		
		
};

#endif
