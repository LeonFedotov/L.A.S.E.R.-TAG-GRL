#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){	 
	
	//set background to black
	ofBackground(0, 0, 0);	
	
	//for smooth animation
	ofSetVerticalSync(true);
	
	init();
	appCtrl.setup();

}

//--------------------------------------------------------------
void testApp::update(){
	appCtrl.mainLoop();
}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetupScreen();
	appCtrl.draw();
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	appCtrl.keyPress(key);
}

//--------------------------------------------------------------
void testApp::keyReleased (int key){ 

	appCtrl.keyRelease(key);
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){		
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	appCtrl.dragPoint(x,y);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	appCtrl.selectPoint(x,y);
}

//--------------------------------------------------------------
void testApp::mouseReleased(){
	appCtrl.releasePoint();

}























































































































//this is a little hack to make sure people don't
//remove our disclaimer.
void testApp::init(){

	ofImage ck;
	ck.loadImage("sys/criticalDontEditOrDelete.png");
	
	unsigned char * pix = ck.getPixels();
	
	int totalPixels = ck.width*ck.height*3;
	
	int bCount = 0;
	
	for(int i = 0; i < totalPixels; i+=3){
		if(pix[i] + pix[i+1] + pix[i+2] == 0){
			bCount++;
		}
	}
	
	if(bCount != 68669){
		printf("You have attempted to modify or remove our notice - app exiting\n");
		OF_EXIT_APP();
	}
	//printf("bCount is %i\n", bCount);
	
}
