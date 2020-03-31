#ifndef _VECTOR_BRUSH_H
#define _VECTOR_BRUSH_H

#include "ofMain.h"
#include "ofAddons.h"

#include "baseBrush.h"

#define MAX_NUM_STROKES 256
#define MAX_STROKE_PTS 512

#define NUM_BRUSH_MODES 4

typedef struct{
	ofVec2f pts[MAX_STROKE_PTS];
	ofVec2f nrm[MAX_STROKE_PTS];
	float width[MAX_STROKE_PTS];
	ofVec2f vec;
	int num;
	int r;
	int g; 
	int b;
}singleStroke;

//true vector brush - uses openGL shapes
class vectorBrush : public baseBrush{
	
	public: 
		
		void setupCustom();
		void clear();
		void update();
		void addPoint(float _x, float _y, bool isNewStroke);
		void draw(int x, int y, int w, int h);
		
	protected:
		void drawBasic(int numStrokes, float offset);
		void drawDope(int numStrokes, float offset);
		void drawArrow(int numStrokes, float offset);
		void drawArrowFAT(int numStrokes, float offset);
		
		singleStroke stroke[MAX_NUM_STROKES]; 
		int whichStroke;
		float diagDist;
		
};

#endif
