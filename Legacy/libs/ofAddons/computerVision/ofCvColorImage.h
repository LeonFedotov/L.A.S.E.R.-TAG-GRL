#ifndef _CV_COLOR_IMAGE_H
#define _CV_COLOR_IMAGE_H


//----------------
#include "ofMain.h"
#include "ofCvConstants.h"



//------------------------------------
class ofCvColorImage {

	public:

		ofCvColorImage();
		void allocate(int _w, int _h);
		void clear();
		void setFromPixels(unsigned char * _pixels, int w, int h);
		void warpIntoMe(ofCvColorImage &mom, ofPoint2f src[4], ofPoint2f dst[4]);
		
		unsigned char 	* getPixels();
		IplImage 		* getCvImage(){ return cvImage;};

		void rgbToHsv();
		void hsvToRgb();

		void 			draw(float x, float y);
		void 			draw(float x, float y, float w, float h);
		int 			width, height;

	private:

		//----------------------------
		// internal functionality
		// we will use two iplImages internally, since many of the
		// operations expect a "src" and a "swap"
		//----------------------------

		IplImage		* 	cvImage;
		IplImage		* 	cvImageTemp;
		void 			swapTemp();		// make the "dst" image the current "src" image

		unsigned char 	* 	pixels;		// not width stepped
		ofTexture 			tex;		// internal tex

};



#endif
