#ifndef _CV_GRAYSCALE_IMAGE_H
#define _CV_GRAYSCALE_IMAGE_H


#include "ofMain.h"
#include "ofCvConstants.h"


class ofCvColorImage;			// forward declaration

//------------------------------------
class ofCvGrayscaleImage {

	public:

		ofCvGrayscaleImage();
		void allocate(int _w, int _h);
		void clear();
		void setFromPixels(unsigned char * _pixels, int w, int h);
		void setFromColorImage(ofCvColorImage mom);

		// singular (inernal) operations:
		void set(int value);
		
		void contrastStretch();				// attempts to remap pixels to 0-255
		void threshold(int value);			// binary threshold on value
		void erode_3x3();
		void dilate_3x3();
		void mirror(bool bFlipVertically, bool bFlipHorizontally);
		void blur();
		void blurHeavily();

		// 1->1 (one to one) operations
		void warpIntoMe(ofCvGrayscaleImage mom, ofPoint2f src[4], ofPoint2f dst[4]);

		// 2->1 (two into one) operations
		void absDiff(ofCvGrayscaleImage mom, ofCvGrayscaleImage dad);

		void composite(ofCvGrayscaleImage mom,ofCvGrayscaleImage momsAlpha);
		


		void addIntoMe(ofCvGrayscaleImage mom, int x, int y);
		void subIntoMe(ofCvGrayscaleImage mom, int x, int y);
		
		// operators ------------------------------
		void operator =		( ofCvGrayscaleImage & mom );
		void operator -=	( ofCvGrayscaleImage & mom );
		void operator +=	( ofCvGrayscaleImage & mom );
		void operator *=	( ofCvGrayscaleImage & mom );

		void operator -=	( float scalar );
		void operator +=	( float scalar );
		// ----------------------------------------

		// statistics:
		int countNonZeroInRegion(int x, int y, int w, int h);

		unsigned char 	* getPixels();
		IplImage 		* getCvImage(){ return cvImage;};

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
