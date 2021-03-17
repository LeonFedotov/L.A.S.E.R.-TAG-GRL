#include "gestureBrush.h"


//----------------------------------------------
void gestureBrush::setupCustom(){
	//we are a vector brush
	isVector = true;

	//doesn't matter as we are an openGL brush
	isColor	 = false;

	//our name and description
	setName("gestureBrush");
	setShortDescription("an animated brush that plays with your gestures");
	
	oldX = 0;
	oldY = 0;
	
	sm.setup();
	
	nFramesSinceAdded = 0;
	bNew = false;
	nFramesSinceAdded = 0;
	velSmooth = 4;
}


//----------------------------------------------
void gestureBrush::update(){
	
	sm.update();
	nFramesSinceAdded++;

	if (nFramesSinceAdded == 10){
		// ok we've been a while, kickoff...
		sm.mouseReleased();
	}
	
}

//----------------------------------------------
void gestureBrush::newLetter(){
	
	//sm.mouseReleased(); // ok?
	sm.mousePressed(0,0, 0);
	bNew = true;
	//void mouseMoved(int x, int y );
	velSmooth = 10;
	
}

//----------------------------------------------
void gestureBrush::draw(int x, int y, int w, int h){
		
	float scaleX =  (float)w / (float)width;
	float scaleY =  (float)h / (float)height;

	float pct = (float)brushBrightness * 0.01;

	ofSetColor( (float)red * pct, (float)green  * pct, (float)blue * pct);	
	glPushMatrix();
		glTranslatef(x, y, 0);
		glScalef(scaleX, scaleY, 1);
		sm.draw();	
	glPopMatrix();
		
}

//----------------------------------------------
void gestureBrush::clear(){
	sm.clear();
}

		
//-----------------------------------------------
void gestureBrush::addPoint(float x1, float y1, bool newStroke){
		
	x1 *= (float)width;
	y1 *= (float)height;
		
	if(newStroke){
		newLetter();
		oldX = x1;
		oldY = y1;
		xSmooth = x1;
		ySmooth = y1; 
	} else {
		xSmooth = xSmooth * 0.3f + 0.7f * x1;
		ySmooth = ySmooth * 0.3f + 0.7f * y1;
	}
	
	addLine(xSmooth, ySmooth, oldX, oldY);
		
	oldX = xSmooth;
	oldY = ySmooth;
	
}

//----------------------------------------------
void gestureBrush::addLine(float x1, float y1, float x2, float y2){
	
	float vel = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	velSmooth = 0.95f * velSmooth + (0.05)*(1 + vel);
	
	if (bNew == true){
		sm.mouseDragged(x1, y1, velSmooth);
		
		bNew = false;
	}
	
	sm.mouseDragged(x2, y2, velSmooth);
	nFramesSinceAdded = 0;
		
}



