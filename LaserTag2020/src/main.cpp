#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"
//========================================================================
int main( ){
	ofGLFWWindowSettings settings;
	settings.setSize(1920, 1080);
	settings.resizable = true;
	shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);

	settings.setSize(1920, 1080);
	settings.resizable = true;
	// uncomment next line to share main's OpenGL resources with gui
	settings.shareContextWith = mainWindow;	
	shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
	guiWindow->setVerticalSync(false);

	shared_ptr<ofApp> mainApp(new ofApp);
	ofAddListener(guiWindow->events().draw, mainApp.get(), &ofApp::drawProjector);
	ofAddListener(guiWindow->events().keyPressed, mainApp.get(), &ofApp::keyPressed);
	ofAddListener(guiWindow->events().keyReleased, mainApp.get(), &ofApp::keyReleased);
	ofAddListener(guiWindow->events().mouseDragged, mainApp.get(), &ofApp::mouseDragged);
	ofAddListener(guiWindow->events().mousePressed, mainApp.get(), &ofApp::mousePressed);
	ofAddListener(guiWindow->events().mouseReleased, mainApp.get(), &ofApp::mouseReleased);

	ofRunApp(mainWindow, mainApp);
	ofRunMainLoop();
}
