#ifndef _SWIM_STROKE
#define _SWIM_STROKE


// this brush is from "gesture machines" by zl
// http://www.theremediproject.com/projects/issue12/systemisgesture/
// enjoy !

#include "angleStroke.h"
#include "strokeRenderer.h"


class swimStroke : public angleStroke {
	
	public:
	
		swimStroke();
		
		
		float 		* originalAngles;
		float 		originalLength;
		bool		bUncurling;
		
		void 		update();
		void 		start();
		void 		draw();
		void 		clear();
		bool 		bSwim;
		
		float 		lengthSectionTarget;
		
		ofVec3f 	centroid;
		ofVec3f 	prevCentroid;
		bool 		bFirstFrame;
		
		ofVec3f 	boundingBoxBottom;
		ofVec3f 	boundingBoxTop;
		
		
		float 			overallTargetLength;
		
};

#endif
