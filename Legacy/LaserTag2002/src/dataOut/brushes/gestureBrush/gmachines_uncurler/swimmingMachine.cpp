#include "swimmingMachine.h"
#include "strokeRenderer.h"

//-------------------------------------------------------------------------
swimmingMachine::swimmingMachine(){

	maxNumStrokes 	= 7;
	nStrokes		= 0;
	angle = new swimStroke[	maxNumStrokes ];
	
}


//-------------------------------------------------------------------------
void swimmingMachine::setup(){
}


//-------------------------------------------------------------------------
void swimmingMachine::update(){
	if (stroke.nPts > 3){
		angle[nStrokes].fitToVec3fs (stroke.resampled, stroke.nMaximumPts, 500);
	}
	
	for (int i =0; i < maxNumStrokes; i++){
		angle[i].update();
	}
}


//-------------------------------------------------------------------------
void swimmingMachine::draw(){	
	for (int i =0; i < maxNumStrokes; i++){
		angle[i].draw();
	}
	
	stroke.draw();
}


//-------------------------------------------------------------------------
void swimmingMachine::start(){
}

void swimmingMachine::clear(){
	for (int i =0; i < maxNumStrokes; i++){
		angle[i].clear();
	}
}


//-------------------------------------------------------------------------
void swimmingMachine::end(){
		for (int i =0; i < maxNumStrokes; i++){
		}
}

//--------------------------------------------------------------
void swimmingMachine::mouseDragged(int x, int y, float button){
	stroke.addPoint(ofVec3f(x,y,button));
}

//--------------------------------------------------------------
void swimmingMachine::mousePressed(int x, int y, float button){
	stroke.nPts = 0;
	angle[nStrokes].bSwim = false;
}

//--------------------------------------------------------------
void swimmingMachine::mouseReleased(){
	angle[nStrokes].start();
	nStrokes++;
	nStrokes %= 7;
	stroke.nPts = 0;
}