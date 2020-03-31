#include "ofMain.h"
#include "testApp.h"

//========================================================================
int main( ){

	
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofSetupOpenGL(1024, 768, OF_WINDOW);			// <-------- setup the GL context


	ofRunApp(new testApp());
	
}