#include "ofCvColorImage.h"

//-------------------------------------------------------------------------------------
ofCvColorImage::ofCvColorImage(){
	width = height = 0;
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::allocate(int _w, int _h){
	cvImage 			= cvCreateImage(cvSize(_w,_h), IPL_DEPTH_8U,3);
	cvImageTemp			= cvCreateImage(cvSize(_w,_h), IPL_DEPTH_8U,3);
	pixels 				= new unsigned char[_w*_h * 3];
	width 				= _w;
	height 				= _h;
	tex.allocate(width, height, GL_RGB);
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::swapTemp(){
	IplImage * temp;
	temp 			= cvImage;
	cvImage 		= cvImageTemp;
	cvImageTemp 	= temp;
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::clear(){
	cvReleaseImage(&cvImage);
	cvReleaseImage(&cvImageTemp);
	width = height = 0;;
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::setFromPixels(unsigned char * _pixels, int w, int h){
	cvSetImageData(cvImage,  _pixels, w*3);
	int stride = w*3;
	for (int i = 0; i < h; i++){
		memcpy(pixels + (i * stride), cvImage->imageData + (i * cvImage->widthStep), stride);
	} 
}

//-------------------------------------------------------------------------------------
unsigned char * ofCvColorImage::getPixels(){
	// copy each line of pixels:
	for (int i = 0; i < height; i++){
		memcpy(pixels + (i * width * 3), cvImage->imageData + (i * cvImage->widthStep), width*3);
	}
	return pixels;
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::warpIntoMe(ofCvColorImage &mom, ofPoint2f src[4], ofPoint2f dst[4]){
	// compute matrix for perspectival warping (homography)
	CvPoint2D32f cvsrc[4];
	CvPoint2D32f cvdst[4];
	CvMat *translate = cvCreateMat(3,3,CV_32FC1);
	cvSetZero(translate);
	for (int i = 0; i < 4; i++){
		cvsrc[i].x = src[i].x;
		cvsrc[i].y = src[i].y;
		cvdst[i].x = dst[i].x;
		cvdst[i].y = dst[i].y;
	}
	cvWarpPerspectiveQMatrix(cvsrc, cvdst, translate);  // calculate homography
	cvWarpPerspective( mom.getCvImage(), cvImage, translate);
	cvReleaseMat(&translate);
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::rgbToHsv(){

	cvCvtColor(cvImage, cvImageTemp, CV_RGB2HSV);
	swapTemp();
			
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::hsvToRgb(){

	cvCvtColor(cvImage, cvImageTemp, CV_HSV2RGB );
	swapTemp();
		
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::draw(float x, float y){

	draw(x, y, width, height);
}

//-------------------------------------------------------------------------------------
void ofCvColorImage::draw(float x, float y, float w, float h){

	//stepped-width image upload:
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
	tex.loadData( (unsigned char *)cvImage->imageData, width, height, GL_RGB);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	
	tex.draw(x,y,w,h);	
	
}
