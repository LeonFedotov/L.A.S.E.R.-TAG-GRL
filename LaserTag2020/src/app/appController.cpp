#include "appController.h"

//---------------------------------------------------
appController::appController() {

}

//all our initalizers
//----------------------------------------------------
void appController::setup() {

	//some vars
	full = false;
	singleScreenMode = false;
	toggleGui = false;
	callibration = false;
	keyTimer = 0;
	bInverted = false;
	brushMode = 0;

	//this is for win32 custom fullscreen
	//see winUtils.h - do not rename windowTitle
	windowTitle = "laser tag server";
	ofSetWindowTitle(windowTitle);

	////// BRUSHES and PROJECTION ///
	IP.setup(PROJECTION_W, PROJECTION_H);
	setupBrushes(PROJECTION_W, PROJECTION_H);
	loadSettings();
	CM.loadColorSettings(ofToDataPath("settings/colors.xml"));

	////// XML SETTINGS ////
	IP.loadSettings(ofToDataPath("settings/quadProj.xml"));


	/////// GUI STUFF ////
	settingsImg.load("sys/settings.png");
	noticeImg.load("sys/criticalDontEditOrDelete.png");
	//	useTrueTypeFont("fonts/courbd.ttf", 10);
	LT.useTrueTypeFont("fonts/courbd.ttf", 10);
	GUI.setUseTTF("fonts/courbd.ttf");

	//////// NETWORK SETUP ///
	setupNetwork();

	//////// MUSIC PLAYER ////			
	TP.loadTracks(ofToDataPath("tunes/"));

	//////// VIDEO TRACKING /
	camWidth = CAM_WIDTH;
	camHeight = CAM_HEIGHT;

	//if we have a camera lets see if the requested dimensions
	//is what we asked for and if not lets update our settings
	//with the real dimensions
	if (USE_CAMERA) {
		LT.setupCamera(CAM_ID, camWidth, camHeight);
		if (LT.W != 0 && LT.H != 0) {
			camWidth = LT.W;
			camHeight = LT.H;
			CAM_WIDTH = camWidth;
			CAM_HEIGHT = camHeight;
		}
	}
	else LT.setupVideo("videos/0_lasertag-IR-trackLaser.mov");

	LT.setupCV(ofToDataPath("settings/quad.xml"));

	//lets update our brushes on launch
	updateBrushSettings(true);

	//see if there is a video from GRL to display.
	webMovieLoaded = VP.load("http://graffitiresearchlab.com/lasertag2000/helloFromGRL.php?" + ofToString(ofRandom(10000, 50000)));
	if (webMovieLoaded)VP.play();

}

//-----------------------------------------------------------
void appController::setupBrushes(int w, int h) {

	brushes[0] = new pngBrush();
	brushes[1] = new vectorBrush();
	brushes[2] = new gestureBrush();

	for (int i = 0; i < NUM_BRUSHES; i++) {
		brushes[i]->setup(w, h);
	}

}

void appController::onBrushModeChange(int& i) {
	updateBrushSettings(false);
}

void appController::onEnableNetwork(bool& b) {
	setupNetwork();
}
//lets read some xml!
//----------------------------------------------------
void appController::loadSettings() {



	GUI.setup("Settings");

	BRUSH_SETTINGS.setName("Brush Settings");
	BRUSH_SETTINGS.add(BRUSH_MODE.set("Brush mode", 0, 0,3));
	BRUSH_MODE.addListener(this, &appController::onBrushModeChange);
	BRUSH_SETTINGS.add(BRUSH_WIDTH.set("Brush width", 4, 2, 128));
	BRUSH_WIDTH.addListener(this, &appController::onBrushModeChange);
	BRUSH_SETTINGS.add(BRUSH_NO.set("Brush image/style", 0, 0, 200));
	BRUSH_NO.addListener(this, &appController::onBrushModeChange);
	BRUSH_SETTINGS.add(BRUSH_COLOR.set("Brush color", 0, 0, 8));
	BRUSH_COLOR.addListener(this, &appController::onBrushModeChange);
	BRUSH_SETTINGS.add(LINE_RES.set("Line resolution", 40, 1, 200));
	LINE_RES.addListener(this, &appController::onBrushModeChange);
	BRUSH_SETTINGS.add(PROJ_BRIGHTNESS.set("Proj Brightness", 100, 0, 100));
	GUI.add(BRUSH_SETTINGS);

	DRIPS_SETTINGS.setName("Drip settings");
	DRIPS_SETTINGS.add(DRIPS.set("Drips enabled", true));
	DRIPS_SETTINGS.add(DRIPS_FREQ.set("Drips freq", 11, 1, 120));
	DRIPS_SETTINGS.add(DRIPS_SPEED.set("Drips speed", 0.3, 0.0, 12.0));
	DRIPS_SETTINGS.add(DRIP_DIRECTION.set("Drips direction", 0, 0, 3));
	GUI.add(DRIPS_SETTINGS);

	TRACKING_SETTINGS.setName("Tracking settings");
	TRACKING_SETTINGS.add(HUE_POINT.set("Hue Point", 0.280000001, 0.0f, 1.0f));
	TRACKING_SETTINGS.add(HUE_WIDTH.set("Hue Thresh Width", 0.170000002, 0.0f, 1.0f));
	TRACKING_SETTINGS.add(SAT_POINT.set("Sat Threshold", 0.219999999, 0.0f, 1.0f));
	TRACKING_SETTINGS.add(VAL_POINT.set("Value Threshold", 0.160000995, 0.0f, 1.0f));
	TRACKING_SETTINGS.add(MIN_BLOB_SIZE.set("Min blob size", 8, 1, 100));
	TRACKING_SETTINGS.add(ACTIVITY.set("Activity thresh", 10, 0, 100));
	TRACKING_SETTINGS.add(JUMP_DIST.set("Jump dist", 0.610000014, 0.0f, 1.0f));
	GUI.add(TRACKING_SETTINGS);

	CLEAR_ZONE_SETTINGS.setName("Clear zone settings");
	CLEAR_ZONE_SETTINGS.add(CLEAR_ZONE.set("Use clear zone", 0, 0, 1));
	CLEAR_ZONE_SETTINGS.add(CLEAR_THRESH.set("Clear sensitivty", 1, 1, 9000));
	CLEAR_ZONE_SETTINGS.add(CLEAR_X.set("Clear x pos", 0, 0, 1920));
	CLEAR_ZONE_SETTINGS.add(CLEAR_Y.set("Clear y pos", 1, 0, 1080));
	CLEAR_ZONE_SETTINGS.add(CLEAR_W.set("Clear width", 1, 0, 1920));
	CLEAR_ZONE_SETTINGS.add(CLEAR_H.set("Clear height", 1, 0, 1080));
	GUI.add(CLEAR_ZONE_SETTINGS);

	NETWORK_SETTINGS.setName("Network settinSgs");
	NETWORK_SETTINGS.add(NETWORK_SEND.set("Enable Network", false));
	NETWORK_SEND.addListener(this, &appController::onEnableNetwork);
	NETWORK_SETTINGS.add(SEND_DATA.set("Send data", false));
	NETWORK_SETTINGS.add(UDP_OR_TCP.set("UDP(0) / TCP(", true));
	NETWORK_SETTINGS.add(IP_PT1.set("ip: val.xxx.xxx.xxx", 127, 0, 255));
	NETWORK_SETTINGS.add(IP_PT2.set("ip: xxx.val.xxx.xxx", 0, 0, 255));
	NETWORK_SETTINGS.add(IP_PT3.set("ip: xxx.xxx.val.xxx", 0, 0, 255));
	NETWORK_SETTINGS.add(IP_PT4.set("ip: xxx.xxx.xxx.val", 1, 0, 255));
	NETWORK_SETTINGS.add(PORT.set("port:", 5544, 0, 65000));
	GUI.add(NETWORK_SETTINGS);

	MUSIC_SETTINGS.setName("Music player settings");
	MUSIC_SETTINGS.add(MUSIC.set("Play music", true));
	MUSIC.addListener(this, &appController::onMusicChange);
	MUSIC_SETTINGS.add(TRACK.set("Which track", 0, 0, 999));
	TRACK.addListener(this, &appController::onTrackChange);
	MUSIC_SETTINGS.add(VOL.set("Volume", 80, 0, 100));
	GUI.add(MUSIC_SETTINGS);

	CAMERA_SETTINGS.setName("Camera settings");
	CAMERA_SETTINGS.add(USE_CAMERA.set("Use camera", false));
	CAMERA_SETTINGS.add(CAM_ID.set("Camera #", 0, 0, 900));
	CAMERA_SETTINGS.add(CAM_WIDTH.set("Camera Width", 320, 0, 1920));
	CAMERA_SETTINGS.add(CAM_HEIGHT.set("Camera Height", 240, 0, 1080));
	GUI.add(CAMERA_SETTINGS);

	GUI.loadFromFile(ofToDataPath("settings/settings.xml"));

}


//----------------------------------------------------
void appController::clearProjectedImage() {
	for (int i = 0; i < NUM_BRUSHES; i++) {
		brushes[i]->clear();
	}
}

//----------------------------------------------------
void appController::mainLoop() {

	//we use this everywhere so lets make it a member
	//variable
	brushMode = BRUSH_MODE;

	//lets find dat laser!
	trackLaser();

	//if sending data is enabled
	//then lets send our data!	
	if (SEND_DATA && LT.newData()) {
		handleNetworkSending();
	}
	//this deals with telling our brushes
	//all about the settings that are being
	//changed
	manageMusic();

	//this is where we paint
	managePainting();

	//if you have a crazy bright projector
	//and a weak laser - you might need to dim the 
	//projector - this method is for raster brushes
	//for gl brushes we do it in updateBrushSettings
	IP.setProjectionBrightness(PROJ_BRIGHTNESS);

	//this is for the singlescreen mode
	//it will show the current setting for a few seconds
	if (ofGetElapsedTimeMillis() - keyTimer > STATUS_SHOW_TIME)keyTimer = 0;

	if (webMovieLoaded)VP.update();
}


//----------------------------------------------------
void appController::setupNetwork() {

	if (NETWORK_SEND) {
		string ipStr = ofToString(IP_PT1) + ".";
		ipStr += ofToString(IP_PT2) + ".";
		ipStr += ofToString(IP_PT3) + ".";
		ipStr += ofToString(IP_PT4);

		int port = PORT;

		if (UDP_OR_TCP) {
			string msg = "status: tcp server listening on port: " + ofToString(port);
			if (LS.setupTCPServer(port)) 	setCommonText(msg);
		}
		else {
			string msg = "status: udp sending to: " + ipStr;
			msg += "  port: " + ofToString(port);
			if (LS.setupUDP(ipStr, port)) setCommonText(msg);
		}
	}
	else {
		LS.close();
		setCommonText("status: all network i/o closed");
	}
}

//----------------------------------------------------
void appController::handleNetworkSending() {
	//if our network is setup
	//then lets send data
	if (LS.isSetup()) {
		LS.sendData(LT.getLaserPointString());
		if (LS.isTCPSetup()) {
			if (LS.checkNewTCPClient()) {
				string msg = "status: tcp client connected on: " + LS.getLatestTCPClientIP();
				setCommonText(msg);
			}
		}
	}
}

//----------------------------------------------------
void appController::trackLaser() {

	//lets see if we need to update the clearZone

	LT.setUseClearZone(CLEAR_ZONE);
	LT.setClearZone(CLEAR_X, CLEAR_Y, CLEAR_W, CLEAR_H);
	LT.setClearThreshold(CLEAR_THRESH);

	//lets process the video data to track that laser
	LT.processFrame(HUE_POINT, HUE_WIDTH, SAT_POINT, VAL_POINT, MIN_BLOB_SIZE, ACTIVITY, JUMP_DIST);

	//clear the images if the clear zone is hit
	if (LT.isClearZoneHit()) {
		clearProjectedImage();
		setCommonText("status: clear zone hit - clearning image\n");
	}


	LT.calcColorRange(HUE_POINT, HUE_WIDTH, SAT_POINT, VAL_POINT);


}


void appController::onMusicChange(bool& b) {
	if (MUSIC) {
		TP.unPause();
		TP.setVolume(VOL);
	}
	else {
		TP.pause();
	}
}

void appController::onTrackChange(int& i) {

}
//----------------------------------------------------
void appController::manageMusic() {

	///////////////////////////////////////////////////////////
	// our music player :)
	///////////////////////////////////////////////////////////

	int whichTrack = TRACK;

	if (MUSIC) {
		TP.unPause();
		TP.setVolume(VOL);
	}
	else {
		TP.pause();
	}


	if (TRACK != TP.getCurrentTrackNo()) {

		if (TRACK >= TP.getNumTracks()) {
			TRACK = 0;
		}
		TP.playTrack(whichTrack);
		setCommonText("Playing: " + ofToString(TP.getCurrentTrackNo()) + " " + TP.getCurrentTrackName());
	}

	if (TP.getFinished()) {
		whichTrack = TP.nextTrack();
		setCommonText("Playing: " + ofToString(TP.getCurrentTrackNo()) + " " + TP.getCurrentTrackName());
		TRACK = whichTrack;
	}
	TP.setVolume(VOL);

}

//----------------------------------------------------
void appController::updateBrushSettings(bool first) {

	CM.setCurrentColor(BRUSH_COLOR);
	unsigned char* rgb = CM.getColor3I();

	for (int i = 0; i < NUM_BRUSHES; i++) {
		brushes[i]->dripsSettings(DRIPS, DRIPS_FREQ, DRIPS_SPEED, DRIP_DIRECTION);
		brushes[i]->setBrushWidth(BRUSH_WIDTH);
		if (i == brushMode) brushes[i]->setBrushNumber(BRUSH_NO);
		brushes[i]->setBrushColor(rgb[0], rgb[1], rgb[2]);
		brushes[i]->setNumSteps(LINE_RES);
		brushes[i]->setBrushBrightness(PROJ_BRIGHTNESS);
	}

	IP.setProjectionColor(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------
void appController::managePainting() {

	//time to paint!
	//but only if we have got data
	if (LT.newData()) {

		float laserX = LT.laserX;
		float laserY = LT.laserY;

		//if our brush is a vector brush we need
		//to warp the coords as we are not 
		//texture warping
		if (brushes[brushMode]->getIsVector()) {
			LT.getWarpedCoordinates(IP.getQuadPoints(), &laserX, &laserY);
		}
		brushes[brushMode]->addPoint(laserX, laserY, LT.isStrokeNew());

		LT.clearNewStroke();
	}

	//idle our current brush
	for (int i = 0; i < NUM_BRUSHES; i++) {
		if (i == brushMode) {
			brushes[i]->update();

			//we need to update our brush style
			//as some brushes have different number
			//of brush styles

			BRUSH_NO = brushes[i]->getBrushNumber();
		}
	}

	//lets tell people which mode they are in

	setCommonText("brush: " + brushes[brushMode]->getName() + " - " + brushes[brushMode]->getDescription());

	//if we are a bitmap brush
	if (brushes[brushMode]->getIsVector() == false) {
		//lets update our texture!!
		if (brushes[brushMode]->getIsColor()) {
			IP.updateColorTexture(brushes[brushMode]->getImageAsPixels());
		}
		else {
			IP.updateGreyscaleTexture(brushes[brushMode]->getImageAsPixels());
		}
	}
}

//----------------------------------------------------
void appController::saveSettings() {

	setCommonText("status: saving settings to xml");

	GUI.saveToFile(ofToDataPath("settings/settings.xml"));
	LT.QUAD.saveToFile(ofToDataPath("settings/quad.xml"));
	IP.QUAD.saveToFile(ofToDataPath("settings/quadProj.xml"));

	setCommonText("status: settings saved to xml");
}

//----------------------------------------------------
void appController::reloadSettings() {

	setCommonText("status: reloading settings from xml");

	GUI.loadFromFile(ofToDataPath("settings/settings.xml"));
	LT.QUAD.loadSettings();
	IP.loadSettings(ofToDataPath("settings/quadProj.xml"));

	setCommonText("status: settings reloaded from xml");

}

//----------------------------------------------------
void appController::selectPoint(float x, float y) {

	if (singleScreenMode) {
		IP.selectQuad(x, y, 0, 0, 1024, 768, 60);
	}
	else {
		LT.QUAD.selectPoint(x, y, 0, 0, 320, 240, 60);

		//only try and select the mini quad 
		//if the large quad is not selected
		if (IP.selectQuad(x, y, 1024, 0, 1024, 768, 60) != 1) {
			IP.selectMiniQuad(x, y, 30);
		}
	}
}

//----------------------------------------------------
void appController::dragPoint(float x, float y) {

	if (singleScreenMode) {
		IP.updateQuad(x, y, 0, 0, 1024, 768);
	}
	else {
		LT.QUAD.updatePoint(x, y, 0, 0, 320, 240);

		//only try and update the mini quad 
		//if the large quad is not selected
		if (!IP.updateQuad(x, y, 1024, 0, 1024, 768)) {
			IP.updateMiniQuad(x, y);
		}
	}

}

//----------------------------------------------------
void appController::releasePoint() {
	LT.QUAD.releaseAllPoints();
	IP.releaseAllQuads();
}

//----------------------------------------------------
void appController::keyPress(int key) {

	if (key == 'f') {

#ifdef TARGET_WIN32
		if (!full) {
			ofBeginCustomFullscreen(0, 0, 2048, 768);
		}
		else {
			ofEndCustomFullscreen();
		}
		full = !full;
#else
		ofToggleFullscreen();
#endif
	}
	else if (key == '-') {
		singleScreenMode = false;
	}
	else if (key == '=') {
		singleScreenMode = true;
	}
	else if (key == 's') {
		saveSettings();
	}
	else if (key == 'r') {
		reloadSettings();
	}
	else if (key == 'c') {
		LT.openCameraSettings();
	}
	else if (key == 'd') {
		setCommonText("status: clearing projection");
		clearProjectedImage();
	}
	else if (key == ' ') {
		toggleGui = !toggleGui;
	}

}

//----------------------------------------------------
void appController::keyRelease(int key) {

}

//----------------------------------------------------
void appController::draw() {


	ofSetColor(255, 255, 255);
	LT.draw(0, 0);

	noticeImg.draw(0, 240);

	ofPushMatrix();
	ofTranslate(50, 5);
	drawText("Colors tracked", 10, 423);
	LT.drawColorRange(10, 429, 120, 44);

	ofSetColor(255, 255, 255);

	//put custom brush tool stuff here for your brush
	string brushName = "pngBrush";
	if (brushes[brushMode]->getName() == brushName) {
		drawText("Current brush", 10, 497);
		brushes[brushMode]->drawTool(10, 503, 32, 32);
	}

	drawText("Brush color", 10, 559);
	CM.drawColorPanel(10, 564, 128, 24, 5);

	ofPopMatrix();


	ofSetColor(255, 255, 255, 90);

	//if we are a vector brush
	if (brushes[brushMode]->getIsVector()) {
		brushes[brushMode]->draw(640, 0, 320, 240);
		IP.drawMiniProjectionTool(640, 0, true, false);
		brushes[brushMode]->draw(1024, 0, 1024, 768);
		IP.drawProjectionMask(1024, 0, 1024, 768);
	}
	else {
		IP.drawMiniProjectionTool(640,0, true, true);
		IP.drawProjectionTex(1024, 0, 1024, 768);
	}

	drawStatusMessage();



	drawGUI();

	ofSetColor(150, 150, 150);
	drawText("fps: " + ofToString(ofGetFrameRate()), 10, 740);

	//make sure we have a black background

	if (toggleGui)IP.drawProjectionToolHandles(1024, 0, 1024, 768, false, true);

	if (webMovieLoaded) {
		ofSetHexColor(0xFFFFFF);
		VP.draw(20, 20, 984, 728);
		VP.draw(1044, 20, 984, 728);
	}

	if(ofGetKeyPressed('d')) LT.drawDebug();
	GUI.draw();
}


//----------------------------------------------------
void appController::drawGUI() {

}

//----------------------------------------------------		
void appController::drawStatusMessage() {

	int txtFade = getFade();

	if (txtFade > 0) {

		float elapsed = ofGetElapsedTimeMillis() - getFadeTimeMs();

		if (elapsed > STATUS_SHOW_TIME / 2) {
			txtFade = 255 - ((int)(255.0 * (elapsed / STATUS_SHOW_TIME)));
			setFade(txtFade);
		}

		ofSetColor(txtFade * 2, txtFade * 2, txtFade * 2);
		ofDrawRectangle(0, 752, 800, 16);

		ofSetColor(txtFade, 0, 0);
		drawText(getCommonText(), 5, 762);
		txtFade--;
		setFade(txtFade);
	}
}
