 #ifndef _CV_CONTOUR_FINDER
#define _CV_CONTOUR_FINDER


#include "ofMain.h"
#include "ofCvConstants.h"
#include "ofCvGrayscaleImage.h"
//--------------------------------------------------------------------
class ofCvContourFinder {

	public:
		
		ofCvContourFinder();

		void  		findContours (		ofCvGrayscaleImage	 	&input,
										int 					minArea,
										int 					maxArea,
										int 					nConsidered,
										bool 					bFindHoles);

		void 		reset();
		void 		draw();

		ofCvGrayscaleImage		inputCopy;
		CvMemStorage			*contour_storage;
		CvMemStorage			*storage;
		CvMoments				*myMoments;

		//----------------------------------- an array of cvSequences
		int 					nCvSeqsFound;
		CvSeq* 					cvSeqBlobs[MAX_NUM_CONTOURS_TO_FIND];
		
		int 					nBlobs;
		ofCvBlob				* blobs; 
		
		

};

#endif
