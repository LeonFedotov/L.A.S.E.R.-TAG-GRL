#ifndef _APP_CONTROLLER_H
#define _APP_CONTROLLER_H

#include "ofMain.h"
#include "ofAddons.h"

//we actually project at 1024 by 768
//but we stretch our texture up from 640 by 480
#define PROJECTION_W 640
#define PROJECTION_H 480

//utils for fake horz span 
//on win32
#include "winUtils.h"

//our other app objects
#include "guiSettingsManager.h"
#include "laserTracking.h"
#include "laserSending.h"
#include "imageProjection.h"
#include "baseGui.h"
#include "trackPlayer.h"
#include "colorManager.h"

//our brushes
#define NUM_BRUSHES 4

#include "baseBrush.h"  	//our base brush class - you must inhereit this to add your own
#include "pngBrush.h"  		//the main png brush - allows you to design your own letters - by Theodore Watson
#include "graffLetter.h" 	//3D Looking bubble letters - by Zachary Lieberman
#include "vectorBrush.h" 	//openGL brush - different drawing modes - by Theodore Watson
#include "gestureBrush.h" 	//gesture machine brush - by Zachary Lieberman

#define STATUS_SHOW_TIME 3000  //in ms - this sets the fade time for the status text 

class appController : public baseGui{

	public:
	
		//---------------------------------------------------
		appController();
		void setup();
		void mainLoop();
		void selectPoint(float x, float y);
		void dragPoint(float x, float y);
		void releasePoint();
		void keyPress(int key);
		void keyRelease(int key);
		void draw();
		
	protected:
		void setupBrushes(int w, int h);
		void loadSettings();
		void saveSettings();
		void reloadSettings();
		void clearProjectedImage();
			
		void setupNetwork();
		void handleNetworkSending();
		void trackLaser();
		void manageMusic();
		void updateBrushSettings(bool first);
		void managePainting();		
		
		void drawGUI();		
		void drawStatusMessage();
		
		colorManager CM;
		
		ofVideoPlayer VP;
		bool webMovieLoaded;

		//an array of our base brush class
		baseBrush * brushes[NUM_BRUSHES];
				
		//other stuff	
		laserTracking LT;
		laserSending  LS;
		guiSettingsManager GUI;
		imageProjection IP;
		trackPlayer TP;
		
		bool toggleGui, full, singleScreenMode, callibration, bInverted;
		int camWidth, camHeight, keyTimer;
		int brushMode;
		
		ofImage settingsImg;
		ofImage noticeImg;
			
};

#endif