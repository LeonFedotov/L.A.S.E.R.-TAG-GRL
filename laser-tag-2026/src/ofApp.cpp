#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
	//set background to black
//    ofSetLogLevel(OF_LOG_VERBOSE);
	ofBackground(0, 0, 0);
    ofSetWindowTitle("L.A.S.E.R.TAG 2026");
	//for smooth animation
	ofSetVerticalSync(false);
    ofSetFrameRate(60);

    // UI scaling setup
    baseWidth = 1280;
    baseHeight = 800;
    uiScale = 2.0f;

    // Add draw listeners for scaling - wrap everything including GUI
    ofAddListener(ofEvents().draw, this, &ofApp::preDrawScale, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().draw, this, &ofApp::postDrawScale, OF_EVENT_ORDER_AFTER_APP + 100);

    // Add mouse listeners to scale coordinates before GUI processes them
    ofAddListener(ofEvents().mousePressed, this, &ofApp::scaleMouseCoords, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &ofApp::scaleMouseCoords, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseReleased, this, &ofApp::scaleMouseCoords, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseMoved, this, &ofApp::scaleMouseCoords, OF_EVENT_ORDER_BEFORE_APP);

	init();
	appCtrl.setup();
}

void ofApp::setupProjector(){
    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);
    ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	appCtrl.mainLoop();
	ofSoundUpdate();  // Ensure sound system gets updated
}

//--------------------------------------------------------------
void ofApp::draw(){
	appCtrl.drawGUI();
}

void ofApp::exit(){
    appCtrl.exit();
}

void ofApp::windowResized(int w, int h){
    // Calculate scale based on window size vs base size
    float scaleX = (float)w / baseWidth;
    float scaleY = (float)h / baseHeight;
    uiScale = std::min(scaleX, scaleY);
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
	appCtrl.selectPoint(mouse.x, mouse.y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(ofMouseEventArgs& mouse) {
	appCtrl.releasePoint();
}

//--------------------------------------------------------------
void ofApp::keyPressedProjector(ofKeyEventArgs& key) {
    appCtrl.keyPressProjector(key.key);
}

//--------------------------------------------------------------
void ofApp::mouseDraggedProjector(ofMouseEventArgs& mouse) {
    appCtrl.dragPointProjector(mouse.x, mouse.y, ofGetWindowWidth(), ofGetWindowHeight());
}

//--------------------------------------------------------------
void ofApp::mousePressedProjector(ofMouseEventArgs& mouse) {
    appCtrl.selectPointProjector(mouse.x, mouse.y, ofGetWindowWidth(), ofGetWindowHeight());
}

//--------------------------------------------------------------
void ofApp::mouseReleasedProjector(ofMouseEventArgs& mouse) {
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

void ofApp::preDrawScale(ofEventArgs& args){
    ofPushView();
    ofViewport(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    ofSetupScreenOrtho(baseWidth, baseHeight, -1, 1);
}

void ofApp::postDrawScale(ofEventArgs& args){
    ofPopView();
}

void ofApp::scaleMouseCoords(ofMouseEventArgs& mouse){
    // Transform window coordinates to logical coordinates
    mouse.x = mouse.x * baseWidth / ofGetWindowWidth();
    mouse.y = mouse.y * baseHeight / ofGetWindowHeight();
}
