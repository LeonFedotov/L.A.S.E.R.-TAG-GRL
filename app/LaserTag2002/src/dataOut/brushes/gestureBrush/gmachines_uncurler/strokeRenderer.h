#ifndef _STROKE_RENDERER
#define _STROKE_RENDERER

#include "ofMain.h"


void renderStroke(ofVec3f * pts, int nPts, bool bUseZasWidth, float width);
void renderStroke(ofVec3f ** pts, int nPts, bool bUseZasWidth, float width);
void renderStroke(float * angles, float * widths, int nPts, ofVec3f startPos, float sectionLength);

#endif 
