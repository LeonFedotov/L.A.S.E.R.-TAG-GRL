#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"
//========================================================================
int main( ){
	ofGLFWWindowSettings settings;
	settings.setSize(1280, 800);
	settings.resizable = false;
	shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);

	settings.setSize(1280, 720);
    glm::vec2 screenSize = mainWindow.get()->getScreenSize();
    settings.setPosition(glm::vec2(screenSize.x, screenSize.y/2-720/2));
	settings.resizable = true;
	settings.shareContextWith = mainWindow;
    
	shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
	guiWindow->setVerticalSync(false);

	shared_ptr<ofApp> mainApp(new ofApp);
    mainApp->setupProjector();
	ofAddListener(guiWindow->events().draw, mainApp.get(), &ofApp::drawProjector);
	ofAddListener(guiWindow->events().keyPressed, mainApp.get(), &ofApp::keyPressedProjector);
	ofAddListener(guiWindow->events().keyReleased, mainApp.get(), &ofApp::keyReleased);
	ofAddListener(guiWindow->events().mouseDragged, mainApp.get(), &ofApp::mouseDraggedProjector);
	ofAddListener(guiWindow->events().mousePressed, mainApp.get(), &ofApp::mousePressedProjector);
	ofAddListener(guiWindow->events().mouseReleased, mainApp.get(), &ofApp::mouseReleasedProjector);

	ofRunApp(mainWindow, mainApp);
	ofRunMainLoop();
}
