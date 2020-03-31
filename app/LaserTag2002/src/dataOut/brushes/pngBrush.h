#ifndef _PNG_BRUSH_H
#define _PNG_BRUSH_H

#include "ofMain.h"
#include "ofAddons.h"

#include "drips.h"
#include "baseBrush.h"

//for small speed improvement?
#define ONE_OVER_255 0.00392157

class pngBrush : public baseBrush{
	
	public: 
					
		//stuff we have overidden from baseBrush
		
		void setupCustom();
		void clear();

		void addPoint(float _x, float _y, bool newStroke);
		void update();
		
		unsigned char * getImageAsPixels();
		void drawTool(int x, int y, int w, int h);		
	
		void dripsSettings(bool doDrip, int dripFreq, float speed, int direction);
		void setBrushColor(unsigned char r, unsigned char g, unsigned b);
		void setBrushWidth(int width);
		
	protected:	
				
		int  loadbrushes(string brushDir);
		void setBrushNumber(int _num);
		void updateTmpImage(float _brushWidth);
		string getbrushName();
		void drawBrushColor(float x, float y, int w, int h);
		
	
		ofDirList DL;
		drips DRIPS;
	
		ofImage IMG;
		ofImage TMP;
					
		unsigned char * pixels;
	
		int oldX, oldY, numBrushes, imageNumBytes;

};



#endif
