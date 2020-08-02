#include "coordWarping.h"

//---------------------------
coordWarping::coordWarping(){
	translate = cvCreateMat(3,3,CV_32FC1);
}

//---------------------------
void coordWarping::calculateMatrix(ofPoint src[4], ofPoint dst[4]){

	cvSetZero(translate);
	for (int i = 0; i < 4; i++){
		cvsrc[i].x = src[i].x;
		cvsrc[i].y = src[i].y;
		cvdst[i].x = dst[i].x;
		cvdst[i].y = dst[i].y;

	}

	cvGetPerspectiveTransform(cvsrc, cvdst, translate);  // calculate homography

	int n       = translate->cols;
	float *data = translate->data.fl;
}

//---------------------------		
ofPoint coordWarping::transform(float xIn, float yIn){

	ofPoint out;

	float *data = translate->data.fl;
	
	float a = data[0];
	float b = data[1];
	float c = data[2];
	float d = data[3];
	
	float e = data[4];
	float f = data[5];
	float i = data[6];
	float j = data[7];
	
	//from Myler & Weeks - so fingers crossed!
	out.x = ((a*xIn + b*yIn + c) / (i*xIn + j*yIn + 1));
	out.y = ((d*xIn + e*yIn + f) / (i*xIn + j*yIn + 1));
	
	return out;
}		
