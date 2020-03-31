#ifndef _COORD_WARPING_H
#define _COORD_WARPING_H

#include "ofMain.h"
#include "ofAddons.h"

//we use openCV to calculate our transform matrix
#include "ofCvConstants.h"
#include "ofCvContourFinder.h"

class coordWarping{


	public:
	
		//---------------------------
		coordWarping();

		//first calculate the matrix
		//do this only when it changes - saves cpu!
		void calculateMatrix(ofPoint2f src[4], ofPoint2f dst[4]);

		//then when ever you need to warp the coords call this
		ofPoint2f transform(float xIn, float yIn);
	
	protected:
		
		CvPoint2D32f cvsrc[4];
		CvPoint2D32f cvdst[4];
		CvMat *translate;
				
};

#endif