#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

#ifdef _WIN32
#include "GLFW/glfw3.h"
#endif

//========================================================================
int main( ){
	ofGLFWWindowSettings settings;
	settings.setSize(1280, 800);
	settings.resizable = true;
	shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
	mainWindow->setVerticalSync(false);

	settings.setSize(1280, 720);
#ifdef _WIN32
	// Windows: use fixed position for reliability
	settings.setPosition(glm::vec2(100, 100));
	settings.decorated = true;
#else
	// macOS/Linux: position based on screen size
	glm::vec2 screenSize = mainWindow.get()->getScreenSize();
	if (screenSize.x > 1920) {
		settings.setPosition(glm::vec2(screenSize.x, screenSize.y/2-720/2));
	} else {
		settings.setPosition(glm::vec2(0, 0));
	}
#endif
	settings.resizable = true;
	settings.shareContextWith = mainWindow;

	shared_ptr<ofAppBaseWindow> guiWindow = ofCreateWindow(settings);
	guiWindow->setVerticalSync(false);

#ifdef _WIN32
	// Windows: Force the projector window to be visible
	ofAppGLFWWindow* glfwWin = dynamic_cast<ofAppGLFWWindow*>(guiWindow.get());
	if (glfwWin) {
		GLFWwindow* win = glfwWin->getGLFWWindow();
		glfwShowWindow(win);
		glfwFocusWindow(win);
	}
#endif

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
