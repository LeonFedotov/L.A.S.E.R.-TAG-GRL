#ifndef _COORD_WARPING_H
#define _COORD_WARPING_H

#include "ofMain.h"
//we use openCV to calculate our transform matrix
#include "ofxOpenCv.h"


class coordWarping{


	public:
	
		//---------------------------
		coordWarping();

		//first calculate the matrix
		//do this only when it changes - saves cpu!
		void calculateMatrix(ofPoint src[4], ofPoint dst[4]);

		//then when ever you need to warp the coords call this
		ofPoint transform(float xIn, float yIn);
	
	protected:
		
		CvPoint2D32f cvsrc[4];
		CvPoint2D32f cvdst[4];
		CvMat *translate;
				
};

#endif
