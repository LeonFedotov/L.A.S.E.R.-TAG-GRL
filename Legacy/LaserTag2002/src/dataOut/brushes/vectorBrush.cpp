#include "vectorBrush.h"
 
//------------------------
void vectorBrush::setupCustom(){

	//we are a vector brush
	isVector = true;

	//doesn't matter as we are an openGL brush
	isColor	 = false;

	//our name and description
	setName("vectorBrush");
	setShortDescription("openGL based brush - speed effects stroke width");
	
	whichStroke = -1;
	
	diagDist = sqrt((float)(width*width + height*height));
	
	clear();
}

//------------------------
void vectorBrush::update(){
	if(brushNumber > NUM_BRUSH_MODES - 1) brushNumber = NUM_BRUSH_MODES - 1;
}

//------------------------
void vectorBrush::clear(){
	for(int i = 0; i < MAX_NUM_STROKES; i++){
		stroke[i].num = 0;
	}
	whichStroke = 0;
}

//------------------------
void vectorBrush::addPoint(float _x, float _y, bool isNewStroke){

	if(whichStroke >= MAX_NUM_STROKES){
		printf("vectorBrush::addPoint() - maxStrokes reached\n");
		return;
	}

	if(isNewStroke || whichStroke == -1 || stroke[whichStroke].num >= MAX_STROKE_PTS){
		whichStroke++;
		
		if(whichStroke >= MAX_NUM_STROKES){
			printf("vectorBrush::addPoint() - maxStrokes reached\n");
			return;
		}
		
		stroke[whichStroke].num = 0;
		
	}
	
	stroke[whichStroke].r 	= red;
	stroke[whichStroke].g 	= green;
	stroke[whichStroke].b 	= blue;

	//scale up to our dimensions
	_x *= (float)width;
	_y *= (float)height;

	int pos = stroke[whichStroke].num;
	
	stroke[whichStroke].pts[pos].set(_x, _y);
	if(pos > 0){
		
		//calculate our speed
		float dx	= _x - stroke[whichStroke].pts[pos-1].x;
		float dy	= _y - stroke[whichStroke].pts[pos-1].y;
		
		ofVec2f nrml(dx, dy);
		float dist = nrml.length();
		
		//our max speed is when they have moved more than 1/16
		//of the diagonal of the projected dimension
		
		dist /= (diagDist * 0.0625); //get to a zero to one range
		dist = 1.0 - MIN(dist, 1);
		
		//get to -1 to 1 range
		dist *= 2;
		dist -= 1;
		
		float amnt = dist * (float)brushWidth * 0.125;
	
		//blur our width a little
		stroke[whichStroke].width[pos] = stroke[whichStroke].width[pos] * 0.3;
		stroke[whichStroke].width[pos] += ( ((float)brushWidth * 0.5 ) + amnt) * 0.7;

		stroke[whichStroke].width[pos] =  (float)brushWidth * 0.5;
		
		nrml.normalize();		
		stroke[whichStroke].nrm[pos] = nrml * 0.7 + stroke[whichStroke].nrm[pos-1] * 0.3;
		stroke[whichStroke].vec = stroke[whichStroke].nrm[pos];
		
	}else stroke[whichStroke].width[pos] =  brushWidth / 2;
	stroke[whichStroke].num++;
	
}	

//------------------------
void vectorBrush::draw(int x, int y, int w, int h){
	if(whichStroke < 0)return;
	
	int numStrokes = whichStroke;
	if(numStrokes >= MAX_NUM_STROKES)numStrokes = MAX_NUM_STROKES-1;
	
	float scaleX =  (float)w / (float)width;
	float scaleY =  (float)h / (float)height;

	glPushMatrix();
		glTranslatef(x, y, 0);
		glScalef(scaleX, scaleY, 1);
		
		if(brushNumber == 0)drawArrow(numStrokes,  8);
		else 
		if(brushNumber == 1)drawBasic(numStrokes, 8);
		else
		if(brushNumber == 2)drawDope(numStrokes, 8);
		else
		if(brushNumber == 3)drawArrowFAT(numStrokes, 8);
						
	glPopMatrix();
	
}
	
//------------------------
void vectorBrush::drawBasic(int numStrokes, float offset){

	float halfBrush;
	float pct = (float)brushBrightness * 0.01;


	ofEnableAlphaBlending();
					
	for(int i = 0; i <= numStrokes; i++){
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(0,0,0, 160);
							
		int numPts = stroke[i].num;		
					
		for(int j = 0; j < numPts; j++){
					
			halfBrush = stroke[i].width[j];
		
			glVertex2f(-offset + stroke[i].pts[j].x + halfBrush, offset + stroke[i].pts[j].y + halfBrush);
			glVertex2f(-offset + stroke[i].pts[j].x - halfBrush, offset + stroke[i].pts[j].y - halfBrush);
		}
		glEnd();	
		
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(stroke[i].r * pct, stroke[i].g * pct, stroke[i].b * pct);
										
		for(int j = 0; j < numPts; j++){
				
			halfBrush = stroke[i].width[j];
		
			glVertex2f(stroke[i].pts[j].x + halfBrush, stroke[i].pts[j].y + halfBrush);
			glVertex2f(stroke[i].pts[j].x - halfBrush, stroke[i].pts[j].y - halfBrush);
		}
		glEnd();		
	}
	
	ofDisableAlphaBlending();
}

//------------------------
void vectorBrush::drawDope(int numStrokes, float offset){

	float nrm_x;
	float nrm_y;
	float halfBrush;
	float pct = (float)brushBrightness * 0.01;	
	
	ofEnableAlphaBlending();
					
	for(int i = 0; i <= numStrokes; i++){
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(0,0,0, 160);
							
		int numPts = stroke[i].num;				
			
		for(int j = 0; j < numPts; j++){
					
			halfBrush 	= stroke[i].width[j];
			nrm_x		= stroke[i].nrm[j].x * halfBrush;
			nrm_y		= stroke[i].nrm[j].y * halfBrush;
		
			glVertex2f(-offset + stroke[i].pts[j].x + nrm_y, offset + stroke[i].pts[j].y - nrm_x);
			glVertex2f(-offset + stroke[i].pts[j].x - nrm_y, offset + stroke[i].pts[j].y + nrm_x);
		}
		glEnd();	
		
		
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(stroke[i].r * pct, stroke[i].g  * pct, stroke[i].b * pct);
							
		for(int j = 0; j < numPts; j++){
		
			halfBrush 	= stroke[i].width[j];
			nrm_x		= stroke[i].nrm[j].x * halfBrush;
			nrm_y		= stroke[i].nrm[j].y * halfBrush;
			
			glVertex2f( stroke[i].pts[j].x + nrm_y, stroke[i].pts[j].y - nrm_x);
			glVertex2f(stroke[i].pts[j].x - nrm_y,  stroke[i].pts[j].y + nrm_x);
		}
		glEnd();
									
	}
	
	ofDisableAlphaBlending();
}


//------------------------
void vectorBrush::drawArrow(int numStrokes, float offset){

	float nrm_x;
	float nrm_y;
	float halfBrush;
	float pct = (float)brushBrightness * 0.01;	

	ofEnableAlphaBlending();
					
	for(int i = 0; i <= numStrokes; i++){
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(0,0,0,160);
							
		int numPts = stroke[i].num;				
			
		for(int j = 0; j < numPts; j++){
			
			halfBrush 	= stroke[i].width[j];
			nrm_x		= stroke[i].nrm[j].x * halfBrush;
			nrm_y		= stroke[i].nrm[j].y * halfBrush;
		
			glVertex2f(-offset + stroke[i].pts[j].x + nrm_y, offset + stroke[i].pts[j].y - nrm_x);
			glVertex2f(-offset + stroke[i].pts[j].x - nrm_y, offset + stroke[i].pts[j].y + nrm_x);
		}
		glEnd();	
		
		
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(stroke[i].r * pct, stroke[i].g * pct, stroke[i].b * pct);
							
		for(int j = 0; j < numPts; j++){
		
			halfBrush 	= stroke[i].width[j];
			nrm_x		= stroke[i].nrm[j].x * halfBrush;
			nrm_y		= stroke[i].nrm[j].y * halfBrush;
			
			glVertex2f( stroke[i].pts[j].x + nrm_y, stroke[i].pts[j].y - nrm_x);
			glVertex2f(stroke[i].pts[j].x  - nrm_y,  stroke[i].pts[j].y + nrm_x);
		}
		if( numPts > 0 ){
			nrm_x = stroke[i].vec.x  * halfBrush;
			nrm_y = stroke[i].vec.y  * halfBrush;
							
			float px = stroke[i].pts[numPts-1].x + nrm_x*2;
			float py = stroke[i].pts[numPts-1].y + nrm_y*2;
		
			glVertex2f( stroke[i].pts[numPts-1].x + nrm_y*3, stroke[i].pts[numPts-1].y - nrm_x*3);
			glVertex2f( stroke[i].pts[numPts-1].x - nrm_y*3, stroke[i].pts[numPts-1].y + nrm_x*3);
			
			glVertex2f(px, py);
			glVertex2f(px, py);
		}
				
		glEnd();
											
	}
	
	ofDisableAlphaBlending();
}

//------------------------
void vectorBrush::drawArrowFAT(int numStrokes, float offset){

	float nrm_x;
	float nrm_y;
	float halfBrush;
	float pct = (float)brushBrightness * 0.01;	

	ofEnableAlphaBlending();
					
	for(int i = 0; i <= numStrokes; i++){
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(0xFF0AC2);
							
		int numPts = stroke[i].num;				
			
		for(int j = 0; j < numPts; j++){
			
			halfBrush 	= stroke[i].width[j];
			nrm_x		= stroke[i].nrm[j].x * halfBrush;
			nrm_y		= stroke[i].nrm[j].y * halfBrush;
		
			glVertex2f(-offset + stroke[i].pts[j].x + nrm_y, offset + stroke[i].pts[j].y - nrm_x);
			glVertex2f(-offset + stroke[i].pts[j].x - nrm_y, offset + stroke[i].pts[j].y + nrm_x);
		}
		glEnd();	
		
		
		glBegin(GL_QUAD_STRIP);		
		ofSetColor(stroke[i].r * pct, stroke[i].g * pct, stroke[i].b * pct);
							
		for(int j = 0; j < numPts; j++){
		
			halfBrush 	= stroke[i].width[j];
			nrm_x		= stroke[i].nrm[j].x * halfBrush;
			nrm_y		= stroke[i].nrm[j].y * halfBrush;
			
			glVertex2f( stroke[i].pts[j].x + nrm_y, stroke[i].pts[j].y - nrm_x);
			glVertex2f(stroke[i].pts[j].x  - nrm_y,  stroke[i].pts[j].y + nrm_x);
		}
		if( numPts > 0 ){
			nrm_x = stroke[i].vec.x  * halfBrush;
			nrm_y = stroke[i].vec.y  * halfBrush;
			
			float px = stroke[i].pts[numPts-1].x + nrm_x*2;
			float py = stroke[i].pts[numPts-1].y + nrm_y*2;
		
			glVertex2f( stroke[i].pts[numPts-1].x + nrm_y*3, stroke[i].pts[numPts-1].y - nrm_x*3);
			glVertex2f( stroke[i].pts[numPts-1].x - nrm_y*3, stroke[i].pts[numPts-1].y + nrm_x*3);
			
			glVertex2f(px, py);
			glVertex2f(px, py);
		}
		glEnd();
											
	}
	
	ofDisableAlphaBlending();
}
