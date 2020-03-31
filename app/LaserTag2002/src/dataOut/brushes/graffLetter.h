#ifndef _GRAFF_LETTER_H
#define _GRAFF_LETTER_H



// hand crafted on the streets of barcelona

#include "ofMain.h"
#include "ofAddons.h"
#include "ofVec3f.h"

//--------------------------
// constants
#include "ofCvConstants.h"
#include "ofCvGrayscaleImage.h"
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
		void setupBrush(ofCvGrayscaleImage &brush, int width);
		void setupShadowBrush(ofCvGrayscaleImage &brush, int width);
		void addDrip(float x1, float y1, float x2, float y2);
		
		dripParticle		particles[MAX_DRIP_PARTICLES];

		int 				borderSize;
		int 				blackSize;
		int 				offsetx;	// 3d yo ! (-15, 15);
		int				 	offsety;	// 3d yo ! (-15, 15);
		int 				oldX;
		int					oldY;
		int 				nDrip;
		bool 				bAddParticles;

		
		ofCvGrayscaleImage	cvImgWhite;
		ofCvGrayscaleImage	cvImgBlack;
		ofCvGrayscaleImage	cvImgBottom;
		ofCvGrayscaleImage	cvImgTop;
		ofCvGrayscaleImage	cvImgMix;
		
		
		ofCvGrayscaleImage	cvWhiteBrush;
		ofCvGrayscaleImage	cvBlackBrush;
		
		ofCvGrayscaleImage	cvDripWhiteBrush;
		ofCvGrayscaleImage	cvDripBlackBrush;
		ofCvGrayscaleImage	cvDripDropShadowBrush;
		
		ofCvGrayscaleImage	cvDropShadowBrush;
		
		ofCvGrayscaleImage	cvTopLayerMask;
		ofCvGrayscaleImage  cvBottomLayerMask;
				

};

#endif	
