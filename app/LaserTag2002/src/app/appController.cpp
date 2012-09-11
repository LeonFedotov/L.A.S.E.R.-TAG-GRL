#include "appController.h"

//---------------------------------------------------
appController::appController(){
	
}

//all our initalizers
//----------------------------------------------------
void appController::setup(){
	
	//some vars
	full				= false;
	singleScreenMode 	= false;
	toggleGui 			= false;
	callibration		= false;
	keyTimer  			= 0;
	bInverted			= false;
	brushMode			= 0;
	
	//this is for win32 custom fullscreen
	//see winUtils.h - do not rename windowTitle
	windowTitle = "laser tag server";
	ofSetWindowTitle(windowTitle);
	
	////// BRUSHES and PROJECTION ///
	IP.setup(PROJECTION_W, PROJECTION_H);
	setupBrushes(PROJECTION_W, PROJECTION_H);
	CM.loadColorSettings(ofToDataPath("settings/colors.xml"));

	////// XML SETTINGS ////
	loadSettings();
	IP.loadSettings(ofToDataPath("settings/quadProj.xml"));
		
		
	/////// GUI STUFF ////
	settingsImg.loadImage("sys/settings.png");
	noticeImg.loadImage("sys/criticalDontEditOrDelete.png");
	useTrueTypeFont("fonts/courbd.ttf", 10);
	LT.useTrueTypeFont("fonts/courbd.ttf", 10);
	GUI.useTrueTypeFont("fonts/courbd.ttf", 10);
	
	//////// NETWORK SETUP ///
	setupNetwork();
	
	//////// MUSIC PLAYER ////			
	TP.loadTracks(ofToDataPath("tunes/"));
				
	//////// VIDEO TRACKING /
	camWidth 	= GUI.getI("CAM_WIDTH");
	camHeight 	= GUI.getI("CAM_HEIGHT");
	
	//if we have a camera lets see if the requested dimensions
	//is what we asked for and if not lets update our settings
	//with the real dimensions
	if( GUI.getI("USE_CAMERA") ){
		LT.setupCamera(GUI.getI("CAM_ID"), camWidth, camHeight);
		if(LT.W != 0 && LT.H != 0){
			 camWidth 	= LT.W;
			 camHeight 	= LT.H;
			 GUI.set("CAM_WIDTH", camWidth);
			 GUI.set("CAM_HEIGHT", camHeight);					 
		}
	}
	else LT.setupVideo("videos/lasertag_test.mov");
	
	LT.setupCV(ofToDataPath("settings/quad.xml"));
	
	//lets update our brushes on launch
	updateBrushSettings(true);

	//see if there is a video from GRL to display.
	webMovieLoaded = VP.loadMovie("http://graffitiresearchlab.com/lasertag2000/helloFromGRL.php?"+ofToString(ofRandom(10000, 50000)));
	if(webMovieLoaded)VP.play();
}

//-----------------------------------------------------------
void appController::setupBrushes(int w, int h){
	
	brushes[0] = new pngBrush();
	brushes[1] = new graffLetter();
	brushes[2] = new vectorBrush();
	brushes[3] = new gestureBrush();
	
	for(int i = 0; i < NUM_BRUSHES; i++){
		brushes[i]->setup(w,h);
	}

}

//lets read some xml!
//----------------------------------------------------
void appController::loadSettings(){

	//we read our settings in 
	setCommonText("status: loading settings from xml");
	GUI.readFromFile(ofToDataPath("settings/settings.xml"));

	//the order of these settings is the order they are displayed.
	//titles break up the settings into sub menu's
	
	
	GUI.addTitle("Drawing settings");

	GUI.addSetting("Brush mode",		"BRUSH_MODE", 		0, 0, NUM_BRUSHES-1, 1);
	GUI.addSetting("Brush width",		"BRUSH_WIDTH",  	4, 2, 128, 1);
	GUI.addSetting("Brush image/style", "BRUSH_NO",			0, 0, 200, 1);
	GUI.addSetting("Brush color",		"BRUSH_COLOR", 	    0, 0, 8, 1);
	GUI.addSetting("Line resolution",	"LINE_RES", 		40, 1, 200, 1);
	GUI.addSetting("Proj Brightness",	"PROJ_BRIGHTNESS", 	100, 0, 100, 4);
	
	GUI.addTitle("Drip settings");
	GUI.addSetting("Drips enabled",		"DRIPS", 			0, 0, 1, 1);
	GUI.addSetting("Drips freq", 		"DRIPS_FREQ", 		11, 1, 120, 1);
	GUI.addSetting("Drips speed", 		"DRIPS_SPEED", 		0.3f, 0.0f, 12.0f, 0.02f);
	GUI.addSetting("Drips direction",	"DRIP_DIRECTION", 	0, 0,3, 1);
	
							
	GUI.addTitle("Tracking settings");
	GUI.addSetting("Hue Point", 		"HUE_POINT", 		0.35f, 0.0f, 1.0f, 0.02f);
	GUI.addSetting("Hue Thresh Width", 	"HUE_WIDTH", 		0.25f, 0.0f, 1.0f, 0.02f);
	GUI.addSetting("Sat Threshold", 	"SAT_POINT", 		0.2f,  0.0f, 1.0f, 0.02f);
	GUI.addSetting("Value Threshold", 	"VAL_POINT", 		0.15f, 0.0f, 1.0f, 0.02f);
	GUI.addSetting("Min blob size",		"MIN_BLOB_SIZE",	8, 1, 100, 1);
	GUI.addSetting("Advanced quad",		"ADVANCED_QUAD",	0, 0, 1, 1);
	GUI.addSetting("Activity thresh", 	"ACTIVITY", 		10, 0, 100, 1);
	GUI.addSetting("Jump dist",			"JUMP_DIST",    	0.0f, 0.0f, 1.0f, 0.01f);
	
	GUI.addTitle("Clear zone settings");
	GUI.addSetting("Use clear zone",	"CLEAR_ZONE", 		0, 0, 1, 1);
	GUI.addSetting("Clear sensitivty",	"CLEAR_THRESH", 	1, 1, 9000, 1);			
	GUI.addSetting("Clear x pos"	,	"CLEAR_X", 			0, 0, 320, 2);
	GUI.addSetting("Clear y pos"	,	"CLEAR_Y", 			1, 0, 240, 2);
	GUI.addSetting("Clear width"	,	"CLEAR_W", 			1, 0, 320, 2);
	GUI.addSetting("Clear height"	,	"CLEAR_H", 			1, 0, 240, 2);

	GUI.addTitle("Network settings");
	GUI.addSetting("Enable Network",	"NETWORK_SEND",		0, 0, 1, 1);
	GUI.addSetting("Send data",			"SEND_DATA",		0, 0, 1, 1);
	GUI.addSetting("UDP(0) / TCP(1)",	"UDP_OR_TCP",		0, 0, 1, 1);
	GUI.addSetting("ip: val.xxx.xxx.xxx",	"IP_PT1",		127, 0, 255, 1);
	GUI.addSetting("ip: xxx.val.xxx.xxx",	"IP_PT2",		0, 0, 255, 1);
	GUI.addSetting("ip: xxx.xxx.val.xxx",	"IP_PT3",		0, 0, 255, 1);
	GUI.addSetting("ip: xxx.xxx.xxx.val",	"IP_PT4",		1, 0, 255, 1);
	GUI.addSetting("port:",					"PORT",			5544, 0, 65000, 1);
	
	GUI.addTitle("Music player settings");
	GUI.addSetting("Play music",	"MUSIC", 	0, 0, 1, 1);	
	GUI.addSetting("Which track",	"TRACK", 	0, 0, 999, 1);	
	GUI.addSetting("Volume",		"VOL", 		80, 0, 100, 1);	
	
	GUI.addTitle("Camera settings (restart app!)");
	GUI.addSetting("Use camera", 				"USE_CAMERA", 			0, 0, 1, 1);	
	GUI.addSetting("Camera #", 					"CAM_ID", 				0, 0, 900, 1);	
	GUI.addSetting("Camera Width",				"CAM_WIDTH", 			320, 0, 900, 2);	
	GUI.addSetting("Camera Height", 			"CAM_HEIGHT", 			240, 0, 900, 2);	
							
	setCommonText("status: loaded settings from xml");
		
}
	
	
//----------------------------------------------------
void appController::clearProjectedImage(){
	for(int i = 0; i < NUM_BRUSHES; i++){
		brushes[i]->clear();
	}
}

//----------------------------------------------------
void appController::mainLoop(){
	
	//we use this everywhere so lets make it a member
	//variable
	brushMode 	= GUI.getI("BRUSH_MODE");

	//lets find dat laser!
	trackLaser();	
		
	//if sending data is enabled
	//then lets send our data!	
	if( GUI.getI("SEND_DATA") && LT.newData() ){
		handleNetworkSending();
	} 
	
	//for our built in music player
	manageMusic();
	
	//this deals with telling our brushes
	//all about the settings that are being
	//changed
	updateBrushSettings(false);
	
	//this is where we paint
	managePainting();
	
	//if you have a crazy bright projector
	//and a weak laser - you might need to dim the 
	//projector - this method is for raster brushes
	//for gl brushes we do it in updateBrushSettings
	IP.setProjectionBrightness(GUI.getI("PROJ_BRIGHTNESS"));
	
	//this is for the singlescreen mode
	//it will show the current setting for a few seconds
	if(ofGetElapsedTimeMillis() - keyTimer > STATUS_SHOW_TIME )keyTimer = 0;

	if(webMovieLoaded)VP.idleMovie();
}


//----------------------------------------------------
void appController::setupNetwork(){

	if( GUI.getI("NETWORK_SEND") ){
		string ipStr 	= ofToString(GUI.getI("IP_PT1")) + ".";
		ipStr +=  ofToString(GUI.getI("IP_PT2")) + ".";
		ipStr +=  ofToString(GUI.getI("IP_PT3")) + ".";
		ipStr +=  ofToString(GUI.getI("IP_PT4"));
						
		int port 		= GUI.getI("PORT"); 
	
		if( GUI.getI("UDP_OR_TCP") ){
			string msg = "status: tcp server listening on port: "+ofToString(port);
			if( LS.setupTCPServer(port) ) 	setCommonText(msg);
		}else{
			string msg = "status: udp sending to: "+ipStr;
			msg +="  port: "+ofToString(port);
			if( LS.setupUDP(ipStr, port) ) setCommonText(msg);
		}
	}else{
		LS.close();
	 	setCommonText("status: all network i/o closed");
	}
}

//----------------------------------------------------
void appController::handleNetworkSending(){

	//look to see if network send has changed
	//if so we might need to open or close
	//a network connection 
	//also look to see if mode between udp and tcp
	//has changed as this will require resetup
	if( GUI.hasChanged("NETWORK_SEND") || ( GUI.hasChanged("UDP_OR_TCP") && GUI.getI("NETWORK_SEND") )  ){
		setupNetwork();
	}
	
	//if our network is setup
	//then lets send data
	if( LS.isSetup() ){
		LS.sendData(LT.getLaserPointString());
		if(LS.isTCPSetup()){
			if( LS.checkNewTCPClient() ){
				string msg = "status: tcp client connected on: "+LS.getLatestTCPClientIP();
				setCommonText(msg);
			}
		}
	}
}

//----------------------------------------------------
void appController::trackLaser(){
		
	//lets see if we need to update the clearZone
	if( GUI.hasChanged("CLEAR_ZONE") || GUI.hasChanged("CLEAR_X") || GUI.hasChanged("CLEAR_Y")
		|| GUI.hasChanged("CLEAR_W") || GUI.hasChanged("CLEAR_H") || GUI.hasChanged("CLEAR_THRESH") ){
		
		LT.setUseClearZone(GUI.getI("CLEAR_ZONE"));
		
		LT.setClearZone(GUI.getI("CLEAR_X"), GUI.getI("CLEAR_Y"), 
						GUI.getI("CLEAR_W"), GUI.getI("CLEAR_H"));
						
		LT.setClearThreshold(GUI.getI("CLEAR_THRESH"));
	}
		
	//these two settings are used to determin
	//when a new stroke is created
	int activity 	= GUI.getI("ACTIVITY");
	float jump	 	= GUI.getF("JUMP_DIST");
	
	//the minSize of our blobs
	int blobSize 	= GUI.getI("MIN_BLOB_SIZE");
			
	//our color tracking settings
	float hue 		= GUI.getF("HUE_POINT");
	float hueThresh = GUI.getF("HUE_WIDTH"); 
	float sat		= GUI.getF("SAT_POINT"); 
	float value		= GUI.getF("VAL_POINT");
	
	//lets process the video data to track that laser
	LT.processFrame(hue, hueThresh, sat,  value, blobSize, activity, jump, GUI.getI("ADVANCED_QUAD")); 
	
	//clear the images if the clear zone is hit
	if( LT.isClearZoneHit() ){
		clearProjectedImage();
		setCommonText("status: clear zone hit - clearning image\n");
	}
	
	//only update our color tracking indicator if values have changed				
	if( GUI.hasChanged("HUE_POINT") || GUI.hasChanged("HUE_WIDTH") 
	    || GUI.hasChanged("SAT_POINT") || GUI.hasChanged("VAL_POINT") ){
	
		LT.calcColorRange(hue, hueThresh, sat, value);	
	}
	
}

//----------------------------------------------------
void appController::manageMusic(){
	
	///////////////////////////////////////////////////////////
	// our music player :)
	///////////////////////////////////////////////////////////
	
	//update our mp3 player
	bool playMusic = GUI.getI("MUSIC");
	int whichTrack = GUI.getI("TRACK");
	int volume	   = GUI.getI("VOL");

	if(playMusic){
		TP.unPause();
		if( whichTrack != TP.getCurrentTrackNo()){
			
			if(whichTrack >= TP.getNumTracks()){
				whichTrack = 0;
				GUI.set("TRACK", whichTrack);
			}
			
			TP.playTrack(whichTrack);
			setCommonText("Playing: "+ofToString(TP.getCurrentTrackNo())+" "+TP.getCurrentTrackName());
		}
		
		if( TP.getFinished() ){
			whichTrack = TP.nextTrack();
			setCommonText("Playing: "+ofToString(TP.getCurrentTrackNo())+" "+TP.getCurrentTrackName());
			GUI.set("TRACK", whichTrack);
		}
		TP.setVolume(volume);
	}else{
		TP.pause();	
	}
}

//----------------------------------------------------
void appController::updateBrushSettings(bool first){
					
	//if our drips settings need to be updated
	if(	GUI.hasChanged("DRIPS") || GUI.hasChanged("DRIPS_FREQ") ||
	 	GUI.hasChanged("DRIPS_SPEED") ||  GUI.hasChanged("DRIP_DIRECTION") || first){
				 	
		int drips 		= GUI.getI("DRIPS");
		int freq 		= GUI.getI("DRIPS_FREQ");
		float speed		= GUI.getF("DRIPS_SPEED");		 	
		int direction	= GUI.getI("DRIP_DIRECTION");
				 	
		for(int i = 0; i < NUM_BRUSHES; i++){
			brushes[i]->dripsSettings(drips, freq, speed, direction);
		}
	}					

	//brush size
	if(	GUI.hasChanged("BRUSH_WIDTH") ){
		for(int i = 0; i < NUM_BRUSHES; i++){
			brushes[i]->setBrushWidth( GUI.getI("BRUSH_WIDTH") );
		}
	}	
	
	//update the brush number (this is used for selecting different pngs)
	//but other brushe classes might need it too
	if(	GUI.hasChanged("BRUSH_NO") ){
		for(int i = 0; i < NUM_BRUSHES; i++){
			if(i == brushMode) brushes[i]->setBrushNumber( GUI.getI("BRUSH_NO") );
		}
	}	
	
	//our brush color 
	if( GUI.hasChanged("BRUSH_COLOR") || first){
		CM.setCurrentColor( GUI.getI("BRUSH_COLOR") );
		
		unsigned char * rgb = CM.getColor3I();
		
		for(int i = 0; i < NUM_BRUSHES; i++){
			brushes[i]->setBrushColor(rgb[0], rgb[1], rgb[2]);
		}
		
		IP.setProjectionColor(rgb[0], rgb[1], rgb[2]);
	} 				
		
	//non performance intensive stuff - we don't check if it is changed
	for(int i = 0; i < NUM_BRUSHES; i++){
		brushes[i]->setNumSteps( GUI.getI("LINE_RES") );
		brushes[i]->setBrushBrightness(GUI.getI("PROJ_BRIGHTNESS"));
	}
	
}

//----------------------------------------------------
void appController::managePainting(){
			
	//time to paint!
	//but only if we have got data
	if( LT.newData() ){
	
		float laserX = LT.laserX;
		float laserY = LT.laserY;
		
		//if our brush is a vector brush we need
		//to warp the coords as we are not 
		//texture warping
		if( brushes[brushMode]->getIsVector() ){
			LT.getWarpedCoordinates(IP.getQuadPoints(), &laserX, &laserY);
		}
		brushes[brushMode]->addPoint(laserX, laserY, LT.isStrokeNew());
		
		LT.clearNewStroke();
	}
	
	//idle our current brush
	for(int i = 0; i < NUM_BRUSHES; i++){
		if(i == brushMode){
			brushes[i]->update();
			
			//we need to update our brush style
			//as some brushes have different number
			//of brush styles
			
			GUI.set("BRUSH_NO", brushes[i]->getBrushNumber());
		}
	}

	//lets tell people which mode they are in
	if( GUI.hasChanged("BRUSH_MODE") ){
		setCommonText("brush: "+brushes[brushMode]->getName()+" - "+brushes[brushMode]->getDescription());
	} 
	
	//if we are a bitmap brush
	if( brushes[brushMode]->getIsVector() == false){
		//lets update our texture!!
		if( brushes[brushMode]->getIsColor() ){
			IP.updateColorTexture(brushes[brushMode]->getImageAsPixels());																
		}else{
			IP.updateGreyscaleTexture(brushes[brushMode]->getImageAsPixels());																
		}
	}
}

//----------------------------------------------------
void appController::saveSettings(){

	setCommonText("status: saving settings to xml");

	GUI.saveToFile(ofToDataPath("settings/settings.xml"));
	LT.QUAD.saveToFile(ofToDataPath("settings/quad.xml"));
	IP.QUAD.saveToFile(ofToDataPath("settings/quadProj.xml"));
	
	setCommonText("status: settings saved to xml");
}

//----------------------------------------------------
void appController::reloadSettings(){
	
	setCommonText("status: reloading settings from xml");

	GUI.reloadSettings();
	LT.QUAD.loadSettings();
	IP.loadSettings(ofToDataPath("settings/quadProj.xml"));
	
	setCommonText("status: settings reloaded from xml");
	
}

//----------------------------------------------------
void appController::selectPoint(float x, float y){
	
	if(singleScreenMode){
		IP.selectQuad(x, y, 0, 0, 1024, 768, 60);
	}
	else{		
		LT.QUAD.selectPoint(x, y, 0, 0, 320, 240, 60);
			
		//only try and select the mini quad 
		//if the large quad is not selected
		if( IP.selectQuad(x, y, 1024, 0, 1024, 768, 60) != 1){								
			IP.selectMiniQuad(x, y, 30);
		}
	} 
}

//----------------------------------------------------
void appController::dragPoint(float x, float y){

	if(singleScreenMode){
		IP.updateQuad(x, y, 0, 0, 1024, 768);
	}
	else{
		LT.QUAD.updatePoint(x, y, 0, 0, 320, 240);
		
		//only try and update the mini quad 
		//if the large quad is not selected
		if( !IP.updateQuad(x, y, 1024, 0, 1024, 768) ){
			IP.updateMiniQuad(x, y);
		}
	} 	
	
}

//----------------------------------------------------
void appController::releasePoint(){
	LT.QUAD.releaseAllPoints();
	IP.releaseAllQuads(); 
}
	
//----------------------------------------------------
void appController::keyPress(int key){

	if(key == 'f'){

		#ifdef TARGET_WIN32
			if(!full){
				ofBeginCustomFullscreen(0,0,2048, 768);
			}else{
				ofEndCustomFullscreen();
			}
			full = !full;
		#else
			ofToggleFullscreen();
		#endif
	}
	else if(key == '-'){
		singleScreenMode = false;
	}
	else if(key == '='){
		singleScreenMode = true;	
	}
	else if(key == 's'){
		saveSettings();
	}
	else if(key == 'r'){
		reloadSettings();
	}
	else if(key == OF_KEY_UP){
		GUI.selectPrev();
		keyTimer = ofGetElapsedTimeMillis();
	}
	else if(key == OF_KEY_DOWN){
		GUI.selectNext();
		keyTimer = ofGetElapsedTimeMillis();
	}
	else if(key == OF_KEY_RIGHT){
		GUI.increase();
		keyTimer = ofGetElapsedTimeMillis();
	}
	else if(key == OF_KEY_LEFT){
		GUI.decrease();
		keyTimer = ofGetElapsedTimeMillis();
	}else if(key == 'c'){
		LT.openCameraSettings();
	}else if(key == 'd'){
		setCommonText("status: clearing projection");
		clearProjectedImage();
	}else if(key == ' '){
		toggleGui = !toggleGui;
	}
	
}

//----------------------------------------------------
void appController::keyRelease(int key){
	
}	

//----------------------------------------------------
void appController::draw(){
		
	if( singleScreenMode ){
	
		//make sure we have a black background
		ofSetColor(0,0,0);
		ofRect(0, 0, 1024, 768);
		
		//if we are a vector brush
		if( brushes[brushMode]->getIsVector() ){
			brushes[brushMode]->draw(0, 0, 1024, 768);
			IP.drawProjectionMask(0, 0, 1024, 768);
		}else IP.drawProjectionTex(0, 0, 1024, 768);

		if(toggleGui)IP.drawProjectionToolHandles(0, 0, 1024, 768, false, true);
		
		ofSetColor(0x555555);
		if(toggleGui)ofDrawBitmapString("fps: "+ofToString(ofGetFrameRate()), 10, 740);
		if(keyTimer > 0){
			ofDrawBitmapString("press '-' key to return to main view", 10, 704);
			GUI.drawSelected(10, 720, 200);
		}

		if(webMovieLoaded){
			ofSetColor(0xFFFFFF);
			VP.draw(20, 20, 984, 728);
		}
	}
	else{
		drawGUI();
		
		ofSetColor(150,150,150);
		drawText("fps: "+ofToString(ofGetFrameRate()), 10, 740);
		
		//make sure we have a black background
		ofSetColor(0,0,0);
		ofRect(1024, 0, 1024, 768);
		
		//if we are a vector brush
		if( brushes[brushMode]->getIsVector() ){
			brushes[brushMode]->draw(1024, 0, 1024, 768);
			IP.drawProjectionMask(1024, 0, 1024, 768);
		}
		else IP.drawProjectionTex(1024, 0, 1024, 768);
						
		if(toggleGui)IP.drawProjectionToolHandles(1024, 0, 1024, 768, false, true);

		if(webMovieLoaded){		
			ofSetColor(0xFFFFFF);
			VP.draw(20, 20, 984, 728);
			VP.draw(1044, 20, 984, 728);
		}
	}

}


//----------------------------------------------------
void appController::drawGUI(){
	LT.draw(0,0);
	
	ofSetColor(255, 255, 255);
	
	noticeImg.draw(4, 246);
	settingsImg.draw(13, 404);
				
	glPushMatrix();
		glTranslatef(50, 5, 0);
		drawText("Colors tracked", 574, 423);
		LT.drawColorRange(577, 429, 120, 44);
		
		ofSetColor(255, 255, 255);
		
		//put custom brush tool stuff here for your brush
		string brushName = "pngBrush";
		if(brushes[brushMode]->getName() == brushName){
			drawText("Current brush", 574, 497);
			brushes[brushMode]->drawTool(577, 503, 32, 32);
		}
					
		drawText("Brush color", 573, 559);
		CM.drawColorPanel(577, 564, 128, 24, 5);
		
	glPopMatrix();
	
	GUI.drawCollapsedStyle(300, 415, 200, 13.0);
	
	ofSetColor(255,255,255,90);
	
	//if we are a vector brush
	if( brushes[brushMode]->getIsVector() ){
		brushes[brushMode]->draw(640, 0, 320, 240);
		IP.drawMiniProjectionTool(640, 0, true, false);
	}
	else IP.drawMiniProjectionTool(640, 0, true, true);

	drawStatusMessage();
}

//----------------------------------------------------		
void appController::drawStatusMessage(){
		
	int txtFade = getFade();	
		
	if(txtFade > 0){
		
		float elapsed = ofGetElapsedTimeMillis() - getFadeTimeMs();
		
		if(elapsed > STATUS_SHOW_TIME/2){
			txtFade = 255 - ((int)(255.0*(elapsed/STATUS_SHOW_TIME)));
			setFade(txtFade);
		} 
				
		ofSetColor(txtFade*2, txtFade*2, txtFade*2);
		ofRect(0, 752, 800, 16);
		
		ofSetColor(txtFade, 0, 0);
		drawText(getCommonText(), 5, 762);
		txtFade--;
		setFade(txtFade);
	}
}
