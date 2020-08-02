#ifndef _GRAFF_LETTER_H
#define _GRAFF_LETTER_H



// hand crafted on the streets of barcelona

#include "ofMain.h"


//--------------------------
// constants
#include "ofxOpenCv.h"
#include "baseBrush.h"

#define MAX_DRIP_PARTICLES 		150
typedef struct{
	float 	x;
	float 	y;
	float 	vx;
	float 	vy;
	bool 	bOn;
}dripParticle;


class graffLetter : public baseBrush{
	
	public:
		
		//our derrived functions
		void setupCustom();
		void setBrushWidth(int width);
		void addPoint(float x1, float y1, bool newStroke);
		void update();
		void clear();
		unsigned char * getImageAsPixels();		
		
	protected:
		void newLetter();
		void addLine(float x1, float y1, float x2, float y2);
		void setupBrush(ofxCvGrayscaleImage &brush, int width);
		void setupShadowBrush(ofxCvGrayscaleImage &brush, int width);
		void addDrip(float x1, float y1, float x2, float y2);
		void addIntoMe(ofxCvGrayscaleImage& target, ofxCvGrayscaleImage mom, int x, int y);
		void composite(ofxCvGrayscaleImage src, ofxCvGrayscaleImage mom, ofxCvGrayscaleImage momsAlpha);
		dripParticle		particles[MAX_DRIP_PARTICLES];

		int 				borderSize;
		int 				blackSize;
		int 				offsetx;	// 3d yo ! (-15, 15);
		int				 	offsety;	// 3d yo ! (-15, 15);
		int 				oldX;
		int					oldY;
		int 				nDrip;
		bool 				bAddParticles;

		
		ofxCvGrayscaleImage	cvImgWhite;
		ofxCvGrayscaleImage	cvImgBlack;
		ofxCvGrayscaleImage	cvImgBottom;
		ofxCvGrayscaleImage	cvImgTop;
		ofxCvGrayscaleImage	cvImgMix;
		
		
		ofxCvGrayscaleImage	cvWhiteBrush;
		ofxCvGrayscaleImage	cvBlackBrush;
		
		ofxCvGrayscaleImage	cvDripWhiteBrush;
		ofxCvGrayscaleImage	cvDripBlackBrush;
		ofxCvGrayscaleImage	cvDripDropShadowBrush;
		
		ofxCvGrayscaleImage	cvDropShadowBrush;
		
		ofxCvGrayscaleImage	cvTopLayerMask;
		ofxCvGrayscaleImage  cvBottomLayerMask;
				

};

#endif	
