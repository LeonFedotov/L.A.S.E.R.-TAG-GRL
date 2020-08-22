#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
	//set background to black
	ofBackground(0, 0, 0);	
	//for smooth animation
	ofSetVerticalSync(true);
    ofSetFrameRate(60);
	init();
	appCtrl.setup();
}

//--------------------------------------------------------------
void ofApp::update(){
	appCtrl.mainLoop();
}

//--------------------------------------------------------------
void ofApp::draw(){
	appCtrl.drawGUI();
}

void ofApp::exit(){
    appCtrl.exit();
}

void ofApp::drawProjector(ofEventArgs& args) {
	appCtrl.drawProjector();
}
//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& key) {
	appCtrl.keyPress(key.key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(ofKeyEventArgs& key) {
	appCtrl.keyRelease(key.key);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(ofMouseEventArgs& mouse) {
	appCtrl.dragPoint(mouse.x, mouse.y);
}

//--------------------------------------------------------------
void ofApp::mousePressed(ofMouseEventArgs& mouse) {
	appCtrl.selectPoint(mouse.x,mouse.y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(ofMouseEventArgs& mouse) {
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
