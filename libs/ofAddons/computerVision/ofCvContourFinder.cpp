#include "ofCvContourFinder.h"
#include "cv.h"


//--------------------------------------------------------------------------
ofCvContourFinder::ofCvContourFinder(){
	myMoments 		= (CvMoments*) malloc(sizeof(CvMoments));
	blobs			= new ofCvBlob[MAX_NUM_CONTOURS_TO_FIND];
	reset();
	
}
 
//--------------------------------------------------------------------------
void ofCvContourFinder::reset(){
	nBlobs = 0;
	memset(blobs, 0, sizeof(ofCvBlob) * MAX_NUM_CONTOURS_TO_FIND);
}

//--------------------------------------------------------------------------
void ofCvContourFinder::draw(){
	
	
	ofNoFill();
	for (int i = 0; i < nBlobs; i++){
		ofRect(blobs[i].boundingRect.x, blobs[i].boundingRect.y, blobs[i].boundingRect.width, blobs[i].boundingRect.height);
	}
	
	for (int i = 0; i < nBlobs; i++){
		glBegin(GL_LINE_LOOP);
		for (int j = 0; j < blobs[i].nPts; j++){
			glVertex2f(blobs[i].pts[j].x, blobs[i].pts[j].y);
		}
		glEnd();
	}
	
}


static int qsort_carea_compare ( const void* _a, const void* _b) {
	int out = 0;
	// pointers, ugh.... sorry about this
	CvSeq* a = *((CvSeq **)_a);
	CvSeq* b = *((CvSeq **)_b);
	// use opencv to calc size, then sort based on size
	float areaa = fabs(cvContourArea(a, CV_WHOLE_SEQ));
	float areab = fabs(cvContourArea(b, CV_WHOLE_SEQ));
	// note, based on the -1 / 1 flip
	// we sort biggest to smallest, not smallest to biggest
	if (areaa > areab) { out = -1; }
	else {               out =  1; }
	return out;
}


//--------------------------------------------------------------------------
void ofCvContourFinder::findContours (		ofCvGrayscaleImage 	&input,
											int 				minArea,
											int 				maxArea,
											int 				nConsidered,
											bool 				bFindHoles){
	
	
	reset();

	// opencv will clober the image it detects contours on, so we want to copy it
	// into a copy before we detect contours.  That copy is allocated if necessary
	// (necessary = (a) not allocated or (b) wrong size)
	// so be careful if you pass in different sized images to "findContours"
	// there is a performance penalty, but we think there is not a memory leak to worry about 
	// better to create mutiple contour finders for different sizes, ie, 
	// if you are finding contours in a 640x480 image but also a 320x240 image
	// better to make two ofCvContourFinder objects then to use one, because 
	// you will get penalized less.

	if (inputCopy.width == 0){	
		inputCopy.allocate(input.width, input.height);
		inputCopy = input;
	} else {
		if (inputCopy.width == input.width && inputCopy.height == input.height){
			inputCopy = input;
		} else {
			// we are allocated, but to the wrong size -- 
			// been checked for memory leaks, but a warning:
			// be careful if you call this function with alot of different
			// sized "input" images!, it does allocation every time
			// a new size is passed in....
			inputCopy.clear();
			inputCopy.allocate(input.width, input.height);
			inputCopy = input;
		}
	}


	CvSeq* contour_list 				= NULL;
	contour_storage 					= cvCreateMemStorage(1000);
	storage 							= cvCreateMemStorage(1000);

	CvContourRetrievalMode retrieve_mode = (bFindHoles) ? CV_RETR_LIST : CV_RETR_EXTERNAL;
	cvFindContours (inputCopy.getCvImage(), contour_storage, &contour_list, sizeof(CvContour), retrieve_mode, CV_CHAIN_APPROX_SIMPLE);
	CvSeq* contour_ptr = contour_list;
		
	nCvSeqsFound = 0;
	//-------------------------------------
	// put the contours from the linked list, into an array for sorting
	while ((contour_ptr != NULL)){

		float area = fabs(cvContourArea(contour_ptr, CV_WHOLE_SEQ));
		
		if ((area > minArea) && (area < maxArea)) {
				if (nCvSeqsFound < MAX_NUM_CONTOURS_TO_FIND){
					cvSeqBlobs[nCvSeqsFound] = contour_ptr;		// copy the pointer
					nCvSeqsFound++;
				}
		}

		contour_ptr = contour_ptr->h_next;
	}
	//------------------------------------- sort the pointers based on size...
	if (nCvSeqsFound > 0){
		qsort( cvSeqBlobs, nCvSeqsFound, sizeof(CvSeq*), qsort_carea_compare);
	}

	// now, we have nCvSeqsFound contours, sorted by size in the array cvSeqBlobs
	// let's get the data out and into our structures that we like....
	
	nBlobs = MIN(nConsidered, nCvSeqsFound);		// take the minumum!

	for (int i = 0; i < nBlobs; i++){
		
		float area 			= cvContourArea(cvSeqBlobs[i], CV_WHOLE_SEQ);
		CvRect rect 		= cvBoundingRect(cvSeqBlobs[i], 0);
		cvMoments(cvSeqBlobs[i], myMoments);
		
		blobs[i].area 					= fabs(area);
		blobs[i].hole 					= area < 0 ? true : false;
		blobs[i].length 				= cvArcLength(cvSeqBlobs[i]);
		blobs[i].boundingRect.x 		= rect.x;
		blobs[i].boundingRect.y 		= rect.y;
		blobs[i].boundingRect.width 	= rect.width;
		blobs[i].boundingRect.height	= rect.height;
		blobs[i].centroid.x 			= (int) (myMoments->m10 / myMoments->m00);
		blobs[i].centroid.y 			= (int) (myMoments->m01 / myMoments->m00);
		
		// get the points for the blob:	
		blobs[i].nPts 					= MIN(MAX_CONTOUR_LENGTH, cvSeqBlobs[i]->total);
		
		CvPoint pt;	
		CvSeqReader reader;
		cvStartReadSeq (cvSeqBlobs[i], &reader, 0);

		for (int j=0; j < blobs[i].nPts; j++){
			CV_READ_SEQ_ELEM(pt, reader);
			blobs[i].pts[j].x = (float)pt.x;
			blobs[i].pts[j].y = (float)pt.y;
		}

	}
	//-----------------------------------------------------
	// Free the storage memory.
	// Warning: do this inside this function otherwise you get a strange memory leak
	if (contour_storage != NULL) 	{ cvReleaseMemStorage(&contour_storage);}
	if (storage != NULL) 	{ cvReleaseMemStorage(&storage);}

}

