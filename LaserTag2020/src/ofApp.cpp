#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
	
	//set background to black
	ofBackground(0, 0, 0);	
	
	//for smooth animation
	ofSetVerticalSync(true);
	
	init();
	appCtrl.setup();

}

//--------------------------------------------------------------
void ofApp::update(){
	appCtrl.mainLoop();
}

//--------------------------------------------------------------
void ofApp::draw(){
	appCtrl.draw();
	ofDrawBitmapString(ofGetFrameRate(), 10, 10);
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	appCtrl.keyPress(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased (int key){

	appCtrl.keyRelease(key);
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	appCtrl.dragPoint(x,y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	appCtrl.selectPoint(x,y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(){
	appCtrl.releasePoint();

}

//this is a little hack to make sure people don't
//remove our disclaimer.
void ofApp::init(){

	ofImage ck;
	ck.load("sys/criticalDontEditOrDelete.png");
	
	unsigned char * pix = ck.getPixels().getData();
	
	int totalPixels = ck.getWidth()*ck.getHeight()*3;
	
	int bCount = 0;
	
	for(int i = 0; i < totalPixels; i+=3){
		if(pix[i] + pix[i+1] + pix[i+2] == 0){
			bCount++;
		}
	}
	
	if(bCount != 68669){
		printf("You have attempted to modify or remove our notice - app exiting\n");
		OF_EXIT_APP(0);
	}
	//printf("bCount is %i\n", bCount);
	
}
