#include "ofCvGrayscaleImage.h"
#include "ofCvColorImage.h"

//------------------------------------------------------------------------------------
ofCvGrayscaleImage::ofCvGrayscaleImage(){
	width = height = 0;;
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::allocate(int _w, int _h){
	cvImage 			= cvCreateImage(cvSize(_w,_h), IPL_DEPTH_8U,1);
	cvImageTemp			= cvCreateImage(cvSize(_w,_h), IPL_DEPTH_8U,1);
	pixels 				= new unsigned char[_w*_h];
	width 				= _w;
	height 				= _h;
	tex.allocate(width, height, GL_LUMINANCE);
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::clear(){
	cvReleaseImage(&cvImage);
	cvReleaseImage(&cvImageTemp);
	delete pixels;
	tex.clear();
	width = height = 0;;
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::set(int value){
	//cvSet(cvImage, cvScalar(value));
	
	for (int i = 0; i < height; i++){
		memset(cvImage->imageData + (i * cvImage->widthStep), value, width);
	}
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::setFromColorImage(ofCvColorImage mom){
	cvCvtColor( mom.getCvImage() ,cvImage, CV_RGB2GRAY); 
}



//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::setFromPixels(unsigned char * _pixels, int w, int h){
	//cvSetImageData(cvImage,  _pixels, w);
	
	for (int i = 0; i < height; i++){
		memcpy(cvImage->imageData + (i * cvImage->widthStep), _pixels + (i * width), width);
	}
	
	for (int i = 0; i < h; i++){
		memcpy(pixels + (i * w), cvImage->imageData + (i * cvImage->widthStep), w);
	}
}

//-------------------------------------------------------------------------------------
unsigned char * ofCvGrayscaleImage::getPixels(){
	// copy each line of pixels:
	for (int i = 0; i < height; i++){
		memcpy(pixels + (i * width), cvImage->imageData + (i * cvImage->widthStep), width);
	}
	return pixels;
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::draw(float x, float y){
	
	draw(x, y, width, height);

}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::draw(float x, float y, float w, float h){

	//stepped-width image upload:
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
	tex.loadData((unsigned char *)cvImage->imageData, width, height, GL_LUMINANCE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	
	tex.draw(x,y,w, h);
	
}


//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::swapTemp(){
	IplImage * temp;
	temp 			= cvImage;
	cvImage 		= cvImageTemp;
	cvImageTemp 	= temp;
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::contrastStretch(){
	double minVal, maxVal;
	cvMinMaxLoc(cvImage, &minVal, &maxVal, NULL, NULL, 0);
	double scale=1.0f;
	double shift=0;
	if ((maxVal-minVal) != 0){
		scale=255.0/(maxVal-minVal);
    	shift=-minVal*scale;
	}
	cvConvertScale(cvImage, cvImageTemp, scale, shift);
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::threshold(int value){
	//http://lush.sourceforge.net/lush-manual/01a8321b.html
	cvThreshold(cvImage,cvImageTemp,value,255,CV_THRESH_BINARY);
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::dilate_3x3(){
	cvDilate( cvImage, cvImageTemp, 0, 1 );
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::erode_3x3(){
	cvErode( cvImage, cvImageTemp, 0, 1 );
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::blur(){
	cvSmooth(cvImage, cvImageTemp, CV_GAUSSIAN , 3);
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::blurHeavily(){
	cvSmooth(cvImage, cvImageTemp, CV_GAUSSIAN , 33);
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::absDiff(ofCvGrayscaleImage mom, ofCvGrayscaleImage dad){
	cvAbsDiff(mom.getCvImage(),dad.getCvImage(),cvImage);
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::mirror(bool bFlipVertically, bool bFlipHorizontally){
	int flipMode = 0;
	//---------------------------------
	if (bFlipVertically && !bFlipHorizontally) flipMode = 0;
	else if (!bFlipVertically && bFlipHorizontally) flipMode = 1;
	else if (bFlipVertically && bFlipHorizontally) flipMode = -1;
	else return;
	//---------------------------------
	cvFlip(cvImage, cvImageTemp, flipMode);
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::warpIntoMe(ofCvGrayscaleImage mom, ofPoint2f src[4], ofPoint2f dst[4]){
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

// --------------------
// blend into me
//
void ofCvGrayscaleImage::composite(ofCvGrayscaleImage mom,ofCvGrayscaleImage momsAlpha){
	
	// for now just mode ATOP...
	// but we should add, over, in etc...
	// http://www.gamedev.net/reference/articles/article320.asp
	
	if (mom.width == width	&& 	mom.height == height){
		if (momsAlpha.width == width 	&& 	momsAlpha.height == height){
			 int pos;
			register unsigned char alphaVal;
			register unsigned char  val;
			unsigned char * momData = (unsigned char *)mom.getCvImage()->imageData;
			unsigned char * momAlphaData = (unsigned char *)momsAlpha.getCvImage()->imageData;
			unsigned char * meData = (unsigned char *)cvImage->imageData;
			
			int result 	= 0;
			pos = 0;
			for (int i = 0; i < height; i++){
				for (int j = 0; j < width; j++){
					alphaVal 			= 	*(momAlphaData + pos);
					*(meData + pos) 	= ((*(meData + pos)*(255-alphaVal)) + 
											(*(momData + pos) * alphaVal)) >> 8; 
					pos++;
				}
				pos = i * cvImage->widthStep;
			}
		}
	}
}
// --------------------


//-------------------------------------------------------------------------------------
int ofCvGrayscaleImage::countNonZeroInRegion(int x, int y, int w, int h){

	int count = 0;
	cvSetImageROI(cvImage, cvRect(x,y,w,h));
	count = cvCountNonZero(cvImage);
	cvResetImageROI( cvImage );
	return count;
}


//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::operator =	( ofCvGrayscaleImage & mom ){
	if (mom.width == width && mom.height == height){
		cvCopy(mom.getCvImage(), cvImage, 0);
	} else {
		// what do we do?
		// cvResize?
		// cvClone?
		printf("error in =, images are different sizes\n");
	}
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::operator -=	(  ofCvGrayscaleImage & mom ){
	if (mom.width == width && mom.height == height){
		cvSub(cvImage, mom.getCvImage(), cvImageTemp);
		swapTemp();
	} else {
		printf("error in -=, images are different sizes\n");
	}
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::operator +=	(  ofCvGrayscaleImage & mom ){
	if (mom.width == width && mom.height == height){
		cvAdd(cvImage, mom.getCvImage(), cvImageTemp);
		swapTemp();
	} else {
		// what do we do?
		// cvResize?
		// cvClone?
		printf("error in =, images are different sizes\n");
	}
}

//------------------------------------------------
void ofCvGrayscaleImage::addIntoMe(ofCvGrayscaleImage mom, int x, int y){
	int ymin = MAX(0, y); 
	int ymax = MIN(0 + height, y + mom.height); 
	if (ymin >= ymax) {
		return;
	}                                 	
	int xmax = MIN(0 + width, x + mom.width);
	int xmin = MAX(0, x);
	if (xmin >= xmax) {
		return;
	}
    
        
	// there is an intersection which is the rectangle  { xmin, ymin, xmax, ymax } 	
	//value->setValues(xmin,ymin,xmax-xmin,ymax-ymin);  
	//printf("rectangle: %i %i %i %i \n", xmin,ymin,xmax-xmin,ymax-ymin);
	
	// roi for ME = this  xmin,ymin,xmax-xmin,ymax-ymin
	// pos for ME = xmin, ymin
	
	// for them...
	
	//if (x > 0)
	int posThemX = x >= 0 ? 0 : abs(x);
	int posThemY = y >= 0 ? 0 : abs(y);
	int widthThem = xmax-xmin;
	int heightThem = ymax-ymin;
	//printf("rectangle them: %i %i %i %i \n", posThemX, posThemY, widthThem, heightThem);
	cvSetImageROI( getCvImage(), cvRect( xmin, ymin, xmax-xmin, ymax-ymin ));
	cvSetImageROI( cvImageTemp, cvRect( xmin, ymin, xmax-xmin, ymax-ymin ));
	cvSetImageROI( mom.getCvImage(), cvRect( posThemX, posThemY, widthThem, heightThem ));
	
	cvAdd(cvImage, mom.getCvImage(), cvImageTemp);
	cvCopy(cvImageTemp, cvImage, 0);
	
	cvSetImageROI( getCvImage(), cvRect(0,0,width, height));
	cvSetImageROI( cvImageTemp, cvRect(0,0,width, height));
	cvSetImageROI( mom.getCvImage(), cvRect(0,0,width, height));
	
	
	//swapTemp();
}


//------------------------------------------------
void ofCvGrayscaleImage::subIntoMe(ofCvGrayscaleImage mom, int x, int y){
	int ymin = MAX(0, y); 
	int ymax = MIN(0 + height, y + mom.height); 
	if (ymin >= ymax) {
		return;
	}                                 	
	int xmax = MIN(0 + width, x + mom.width);
	int xmin = MAX(0, x);
	if (xmin >= xmax) {
		return;
	}
    
	//if (x > 0)
	int posThemX = x >= 0 ? 0 : abs(x);
	int posThemY = y >= 0 ? 0 : abs(y);
	int widthThem = xmax-xmin;
	int heightThem = ymax-ymin;
	//printf("rectangle them: %i %i %i %i \n", posThemX, posThemY, widthThem, heightThem);
	cvSetImageROI( getCvImage(), cvRect( xmin, ymin, xmax-xmin, ymax-ymin ));
	cvSetImageROI( cvImageTemp, cvRect( xmin, ymin, xmax-xmin, ymax-ymin ));
	cvSetImageROI( mom.getCvImage(), cvRect( posThemX, posThemY, widthThem, heightThem ));
	
	cvSub(cvImage, mom.getCvImage(), cvImageTemp);
	cvCopy(cvImageTemp, cvImage, 0);
	
	cvSetImageROI( getCvImage(), cvRect(0,0,width, height));
	cvSetImageROI( cvImageTemp, cvRect(0,0,width, height));
	cvSetImageROI( mom.getCvImage(), cvRect(0,0,width, height));
	
}

		

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::operator *=	(  ofCvGrayscaleImage & mom ){
	if (mom.width == width && mom.height == height){
		float scalef = 1.0f / 255.0f;
		cvMul(cvImage, mom.getCvImage(), cvImageTemp, scalef);
		swapTemp();
	} else {
		// what do we do?
		// cvResize?
		// cvClone?
		printf("error in =, images are different sizes\n");
	}
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::operator -=	( float scalar ){
	cvSubS(cvImage, cvScalar(scalar), cvImageTemp);
	swapTemp();
}

//-------------------------------------------------------------------------------------
void ofCvGrayscaleImage::operator +=	( float scalar ){
	cvAddS(cvImage, cvScalar(scalar), cvImageTemp);
	swapTemp();
}
