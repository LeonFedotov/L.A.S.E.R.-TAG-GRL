#ifndef _CV_CONSTANTS_H
#define _CV_CONSTANTS_H

#include "cv.h"


#define 	MAX_NUM_CONTOURS_TO_FIND		128				// alther this if you think you will be looking for more....
#define 	MAX_CONTOUR_LENGTH				1024			// alther this if you think your contours will be longer than this

//------------------------------
typedef struct {
	float x;
	float y;
} ofPoint2f;

//------------------------------
typedef struct {
	float x;
	float y;
	float width;
	float height;
} ofRectangle;


//------------------------------
typedef struct {
	float				area;			// area of contour
	float				length;			// length of contour
	ofRectangle			boundingRect;	// bounding box
	ofPoint2f			centroid;		// centroid
	bool				hole;			// am I a hole?
	int					nPts;
	ofPoint2f			pts[MAX_CONTOUR_LENGTH];
} ofCvBlob;


#endif
