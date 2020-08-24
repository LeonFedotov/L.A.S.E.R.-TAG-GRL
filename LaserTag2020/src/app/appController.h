#ifndef _APP_CONTROLLER_H
#define _APP_CONTROLLER_H

#include "ofMain.h"

//utils for fake horz span 
//on win32
#include "ofxGuiExtended.h"
//our other app objects
#include "laserTracking.h"
#include "laserSending.h"
#include "imageProjection.h"
#include "baseGui.h"
#include "trackPlayer.h"
#include "colorManager.h"

//our brushes
#define NUM_BRUSHES 4

#include "baseBrush.h"  	//our base brush class - you must inhereit this to add your own
#include "basicVectorBrush.h"
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
    void exit();
    void selectPoint(float x, float y);
    void dragPoint(float x, float y);
    void releasePoint();
    void keyPress(int key);
    void keyRelease(int key);
    void drawProjector();
    void drawGUI();
    void setupCamera();
    void setupVideo();
    void positionGui();
protected:
    void setupBrushes(int w, int h);
    void setupListeners();
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
    void drawStatusMessage();
    
    colorManager CM;
    
    ofVideoPlayer VP;
    bool webMovieLoaded;
    
    //an array of our base brush class
    baseBrush * brushes[NUM_BRUSHES];
    
    //other stuff
    laserTracking laserTracking;
    laserSending  laserSending;
    imageProjection imageProjection;
    trackPlayer trackPlayer;
    
    bool toggleGui, full, singleScreenMode, callibration, bInverted;
    int camWidth, camHeight, keyTimer;
    
    ofImage twentyTwentyImg;
    ofImage settingsImg;
    ofImage noticeImg;
    
    ofxGui GUI;
    ofxGuiPanel* brush_panel;
    ofParameterGroup BRUSH_SETTINGS;
    ofParameter<int> BRUSH_MODE;
    ofParameter<int> BRUSH_WIDTH;
    ofParameter<int> BRUSH_NO;
    ofParameter<int> BRUSH_COLOR;
    ofParameter<int> LINE_RES;
    ofParameter<int> PROJ_BRIGHTNESS;
    
    ofxGuiPanel* drip_panel;
    ofParameterGroup DRIPS_SETTINGS;
    ofParameter<bool> DRIPS;
    ofParameter<int> DRIPS_FREQ;
    ofParameter<float> DRIPS_SPEED;
    ofParameter<int> DRIP_DIRECTION;
    ofParameter<float> DRIP_WIDTH;
    
    ofxGuiPanel* tracking_panel;
    ofParameterGroup TRACKING_SETTINGS;
    ofParameter<float> HUE_POINT;
    ofParameter<float> HUE_WIDTH;
    ofParameter<float> SAT_POINT;
    ofParameter<float> VAL_POINT;
    ofParameter<int> MIN_BLOB_SIZE;
    ofParameter<int> ACTIVITY;
    ofParameter<float> JUMP_DIST;
    
    ofxGuiPanel* clear_panel;
    ofParameterGroup CLEAR_ZONE_SETTINGS;
    ofParameter<bool> CLEAR_ZONE;
    ofParameter<int> CLEAR_THRESH;
    ofParameter<int> CLEAR_X;
    ofParameter<int> CLEAR_Y;
    ofParameter<int> CLEAR_W;
    ofParameter<int> CLEAR_H;
    
    ofxGuiPanel* network_panel;
    ofParameterGroup NETWORK_SETTINGS;
    ofParameter<bool> NETWORK_SEND;
    ofParameter<bool> SEND_DATA;
    ofParameter<bool> UDP_OR_TCP;
    ofParameter<int> IP_PT1;
    ofParameter<int> IP_PT2;
    ofParameter<int> IP_PT3;
    ofParameter<int> IP_PT4;
    ofParameter<int> PORT;
    
    ofxGuiPanel* music_panel;
    ofParameterGroup MUSIC_SETTINGS;
    ofParameter<bool> MUSIC;
    ofParameter<int> TRACK;
    ofParameter<int> VOL;
    
    ofxGuiPanel* camera_panel;
    ofParameterGroup CAMERA_SETTINGS;
    ofParameter<bool> USE_CAMERA;
    ofParameter<int> CAM_ID;
    ofParameter<int> CAM_WIDTH;
    ofParameter<int> CAM_HEIGHT;
    ofParameter<int> PROJECTION_W;
    ofParameter<int> PROJECTION_H;
    
    bool bSetupCamera;
    bool bSetupVideo;
    
    void onBrushModeChange(int & i);
    void onEnableNetwork(bool & b);
    void onMusicChange(bool& b);
    void onTrackChange(int & i);
    void onCameraChange(bool & b);
};
#endif
