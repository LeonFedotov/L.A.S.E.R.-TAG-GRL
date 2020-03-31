#ifndef _MAX_STROKE
#define _MAX_STROKE


#include "ofMain.h"
#include "ofVec3f.h"


class maxStroke {
	
	public:
	
		maxStroke();
		void setup();
		void draw();
		void update();
		void addPoint(ofVec3f pt);
	
		void resamplePts(	ofVec3f *input,  		int inNum,
						    ofVec3f *output, 		int outNum);
		

		
				    
		int nMaximumPts;
		int nPts;
		ofVec3f * input;
		ofVec3f * resampled;
		
};

#endif
