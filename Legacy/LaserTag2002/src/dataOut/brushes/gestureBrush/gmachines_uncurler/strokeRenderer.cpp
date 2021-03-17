#include "strokeRenderer.h"




void renderStroke(ofVec3f * pts, int nPts, bool bUseZasWidth, float width){


	if (nPts < 3) return;
	
	
	float 		angleSmooth;
	float 		prevAngle;
	
	ofVec3f 	tangent;
	
	ofVec3f 	pta, ptb;
	
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < nPts-1; i++){
		float angle 	= atan2(pts[i+1].y - pts[i].y, pts[i+1].x - pts[i].x);
		if (i == 0){
			angleSmooth = angle;
		} else {
			float diff = (angle - angleSmooth);
			while (diff < -PI) diff += TWO_PI;
			while (diff > PI) diff -= TWO_PI;
			
			angleSmooth = 0.5f * angleSmooth + 
						  0.5f * (angleSmooth + diff);
		}
		//prevAngle = angle;
		
		tangent.set(-sin(angleSmooth), cos(angleSmooth),0);
		pta = pts[i] + tangent * 10.0f;
		ptb = pts[i] - tangent * 10.0f;
		glVertex2f(pta.x, pta.y);
		glVertex2f(ptb.x, ptb.y);
		//ofLine(pta.x, pta.y, ptb.x, ptb.y);
	}
	glEnd();

}






void renderStroke(ofVec3f ** pts, int nPts, bool bUseZasWidth, float width){


	if (nPts < 3) return;
	
	
	float 		angleSmooth;
	float 		prevAngle;
	
	ofVec3f 	tangent;
	
	ofVec3f 	pta, ptb;
	
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < nPts; i++){
		int posa = i;
		int posb = i+1; if (posb == nPts){posb = nPts - 1; posa--;
		}
		float angle 	= atan2(pts[posb]->y - pts[posa]->y, pts[posb]->x - pts[posa]->x);
		if (i == 0){
			angleSmooth = angle;
		} else {
			float diff = (angle - angleSmooth);
			while (diff < -PI) diff += TWO_PI;
			while (diff > PI) diff -= TWO_PI;
			
			angleSmooth = 0.8f * angleSmooth + 
						  0.2f * (angleSmooth + diff);
		}
		//prevAngle = angle;
		
		tangent.set(-sin(angleSmooth), cos(angleSmooth),0);
		pta = *pts[i] + tangent * pts[i]->z/2.0f;
		ptb = *pts[i] - tangent * pts[i]->z/2.0f;
		glVertex2f(pta.x, pta.y);
		glVertex2f(ptb.x, ptb.y);
		//ofLine(pta.x, pta.y, ptb.x, ptb.y);
	}
	glEnd();

}



void renderStroke(float * angles, float * widths, int nPts, ofVec3f startPos, float sectionLength){
	
	ofVec3f pt = startPos;
	ofVec3f tangent;
	ofVec3f pta, ptb;
	float 		angleSmooth;
	
	
	glBegin(GL_QUAD_STRIP);
	//glVertex2f(pt.x, pt.y);
	float angle = 0;
	for (int i = 0; i < nPts; i++){
		if (i == 0) angle = angles[i];
		else (angle += angles[i]);
		
		tangent.set(-sin(angle), cos(angle),0);
		pt.x += sectionLength * cos(angle);
		pt.y += sectionLength * sin(angle);
		//if (i % 4 == 0) glVertex2f(pt.x, pt.y);
		float w = widths[i] / 2.0f;
		w = MAX(w, 1.5f);
		pta = pt + tangent * widths[i]/2.0f;
		ptb = pt - tangent * widths[i]/2.0f;
		glVertex2f(pta.x, pta.y);
		glVertex2f(ptb.x, ptb.y);
		//ofLine(pta.x, pta.y, ptb.x
	}
	glEnd();


}