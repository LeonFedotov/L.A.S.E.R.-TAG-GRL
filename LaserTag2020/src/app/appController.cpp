#include "appController.h"

//---------------------------------------------------
appController::appController() {
    
}

//all our initalizers
//----------------------------------------------------
void appController::setup() {
    
    //some vars
    full = false;
    toggleGui = false;
    callibration = false;
    keyTimer = 0;
    bInverted = false;
    BRUSH_MODE = 0;
    bSetupCamera = false;
    bSetupVideo = false;
    

    
    loadSettings();
    
    setupProjections();
    imageProjection.loadSettings(ofToDataPath("settings/quadProj.xml"));
    colorManager.loadColorSettings(ofToDataPath("settings/colors.xml"));
    /////// GUI STUFF ////
    settingsImg.load("sys/settings.png");
    noticeImg.load("sys/criticalDontEditOrDelete.png");
    twentyTwentyImg.load("sys/2020.png");
    laserTracking.useTrueTypeFont("fonts/courbd.ttf", 10);
    
    //////// NETWORK SETUP ///
    setupNetwork();
    
    if (USE_CAMERA) {
        setupCamera();
    }else{
        setupVideo();
    }
    //lets update our brushes on launch
    updateBrushSettings(true);
    
    //see if there is a video from GRL to display.
    webMovieLoaded = VP.load("");
    if (webMovieLoaded)VP.play();
    
    //////// MUSIC PLAYER ////
    trackPlayer.loadTracks(ofToDataPath("tunes/"));
    
    setupListeners();
}

void appController::setupProjections(){
    imageProjection.setup(PROJECTION_W, PROJECTION_H);
    setupBrushes(PROJECTION_W, PROJECTION_H);
    imageProjection.setToolDimensions(640, 360);
}

void appController::setupCamera(){
    //////// VIDEO TRACKING /
    camWidth = CAM_WIDTH;
    camHeight = CAM_HEIGHT;
    
    //if we have a camera lets see if the requested dimensions
    //is what we asked for and if not lets update our settings
    //with the real dimensions
    
    laserTracking.setupCamera(CAM_ID, camWidth, camHeight);
    if (laserTracking.W != 0 && laserTracking.H != 0) {
        camWidth = laserTracking.W;
        camHeight = laserTracking.H;
        CAM_WIDTH = camWidth;
        CAM_HEIGHT = camHeight;
    }
    laserTracking.setupCV(ofToDataPath("settings/quad.xml"));
}

void appController::setupVideo(){
    laserTracking.setupVideo("videos/lasertag-IR-trackLaser.mp4");
    laserTracking.setupCV(ofToDataPath("settings/quad.xml"));
}

//-----------------------------------------------------------
void appController::setupBrushes(int w, int h) {
    
    brushes[0] = new pngBrush();
    brushes[1] = new vectorBrush();
    brushes[2] = new gestureBrush();
    brushes[3] = new graffLetter();
    
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
    
    BRUSH_SETTINGS.setName("Brush Settings");
    BRUSH_SETTINGS.add(BRUSH_MODE.set("Brush mode", 0, 0, NUM_BRUSHES-1));
    BRUSH_SETTINGS.add(BRUSH_WIDTH.set("Brush width", 4, 2, 128));
    BRUSH_SETTINGS.add(BRUSH_NO.set("Brush image/style", 0, 0, 25));
    BRUSH_SETTINGS.add(BRUSH_COLOR.set("Brush color", 0, 0, 8));
    BRUSH_SETTINGS.add(LINE_RES.set("Line resolution", 40, 1, 200));
    BRUSH_SETTINGS.add(PROJ_BRIGHTNESS.set("Proj Brightness", 100, 0, 100));
    
    brush_panel = GUI.addPanel(BRUSH_SETTINGS);
    
    DRIPS_SETTINGS.setName("Drip settings");
    DRIPS_SETTINGS.add(DRIPS.set("Drips enabled", true));
    DRIPS_SETTINGS.add(DRIPS_FREQ.set("Drips freq", 11, 1, 120));
    DRIPS_SETTINGS.add(DRIPS_SPEED.set("Drips speed", 0.3, 0.0, 12.0));
    DRIPS_SETTINGS.add(DRIP_DIRECTION.set("Drips direction", 0, 0, 3));
    DRIPS_SETTINGS.add(DRIP_WIDTH.set("Drips width", 1, 1, 25));
    drip_panel = GUI.addPanel(DRIPS_SETTINGS);
    
    TRACKING_SETTINGS.setName("Tracking settings");
    TRACKING_SETTINGS.add(HUE_POINT.set("Hue Point", 0.280000001, 0.0f, 1.0f));
    TRACKING_SETTINGS.add(HUE_WIDTH.set("Hue Thresh Width", 0.170000002, 0.0f, 1.0f));
    TRACKING_SETTINGS.add(SAT_POINT.set("Sat Threshold", 0.219999999, 0.0f, 1.0f));
    TRACKING_SETTINGS.add(VAL_POINT.set("Value Threshold", 0.160000995, 0.0f, 1.0f));
    TRACKING_SETTINGS.add(MIN_BLOB_SIZE.set("Min blob size", 8, 1, 100));
    TRACKING_SETTINGS.add(ACTIVITY.set("Activity thresh", 10, 0, 100));
    TRACKING_SETTINGS.add(JUMP_DIST.set("Jump dist", 0.610000014, 0.0f, 1.0f));
    tracking_panel = GUI.addPanel(TRACKING_SETTINGS);
    
    CLEAR_ZONE_SETTINGS.setName("Clear zone settings");
    CLEAR_ZONE_SETTINGS.add(CLEAR_ZONE.set("Use clear zone", 0, 0, 1));
    CLEAR_ZONE_SETTINGS.add(CLEAR_THRESH.set("Clear sensitivty", 1, 1, 9000));
    CLEAR_ZONE_SETTINGS.add(CLEAR_X.set("Clear x pos", 0, 0, 640));
    CLEAR_ZONE_SETTINGS.add(CLEAR_Y.set("Clear y pos", 1, 0, 480));
    CLEAR_ZONE_SETTINGS.add(CLEAR_W.set("Clear width", 1, 0, 200));
    CLEAR_ZONE_SETTINGS.add(CLEAR_H.set("Clear height", 1, 0, 200));

    clear_panel = GUI.addPanel(CLEAR_ZONE_SETTINGS);

    NETWORK_SETTINGS.setName("Network settinSgs");
    NETWORK_SETTINGS.add(NETWORK_SEND.set("Enable OSC", false));
    NETWORK_SETTINGS.add(SEND_DATA.set("Send data", false));
    NETWORK_SETTINGS.add(IP_PT1.set("ip: val.xxx.xxx.xxx", 192, 0, 255));
    NETWORK_SETTINGS.add(IP_PT2.set("ip: xxx.val.xxx.xxx", 168, 0, 255));
    NETWORK_SETTINGS.add(IP_PT3.set("ip: xxx.xxx.val.xxx", 1, 0, 255));
    NETWORK_SETTINGS.add(IP_PT4.set("ip: xxx.xxx.xxx.val", 255, 0, 255));
    NETWORK_SETTINGS.add(PORT.set("port", 5544, 0, 65000));
    network_panel = GUI.addPanel(NETWORK_SETTINGS);
    
    MUSIC_SETTINGS.setName("Music player settings");
    MUSIC_SETTINGS.add(MUSIC.set("Play music", true));
    MUSIC_SETTINGS.add(TRACK.set("Which track", 0, 0, 999));

    MUSIC_SETTINGS.add(VOL.set("Volume", 80, 0, 100));
    music_panel = GUI.addPanel(MUSIC_SETTINGS);
    
    CAMERA_SETTINGS.setName("Camera settings");
    CAMERA_SETTINGS.add(USE_CAMERA.set("Use camera", false));
    ofVideoGrabber g;
    vector<ofVideoDevice> tempG = g.listDevices();
    CAMERA_SETTINGS.add(CAM_ID.set("Camera", 0, 0, tempG.size()-1));
    CAMERA_SETTINGS.add(CAM_WIDTH.set("Camera Width", 320, 0, 640));
    CAMERA_SETTINGS.add(CAM_HEIGHT.set("Camera Height", 240, 0, 480));
    CAMERA_SETTINGS.add(PROJECTION_W.set("Porjection Width", 1280, 640, 1920));
    CAMERA_SETTINGS.add(PROJECTION_H.set("Porjection Height", 720, 480, 1080));
    camera_panel = GUI.addPanel(CAMERA_SETTINGS);
    
    
    save_panel = GUI.addPanel();
    SAVE.set("Save", false);
    LOAD.set("Load", false);
    CLEAR.set("Clear Screen", false);
    SHOW_CHECKERBOARD.set("Show Checkerboard", false);
    
    save_panel->add(SAVE, ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    save_panel->add(LOAD, ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    save_panel->add(SHOW_CHECKERBOARD, ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    save_panel->add(CLEAR, ofJson({{"type", "fullsize"}, {"text-align", "center"}}));
    
    brush_panel->loadFromFile(ofToDataPath("settings/brush_settings.xml"));
    drip_panel->loadFromFile(ofToDataPath("settings/drip_settings.xml"));
    tracking_panel->loadFromFile(ofToDataPath("settings/tracking_settings.xml"));
    clear_panel->loadFromFile(ofToDataPath("settings/clear_settings.xml"));
    network_panel->loadFromFile(ofToDataPath("settings/network_settings.xml"));
    music_panel->loadFromFile(ofToDataPath("settings/music_settings.xml"));
    camera_panel->loadFromFile(ofToDataPath("settings/camera_settings.xml"));
    
    positionGui();
}

void appController::setupListeners(){
    BRUSH_MODE.addListener(this, &appController::onBrushModeChange);
    BRUSH_WIDTH.addListener(this, &appController::onBrushModeChange);
    BRUSH_NO.addListener(this, &appController::onBrushModeChange);
    BRUSH_COLOR.addListener(this, &appController::onBrushModeChange);
    LINE_RES.addListener(this, &appController::onBrushModeChange);
    USE_CAMERA.addListener(this, &appController::onCameraChange);
    TRACK.addListener(this, &appController::onTrackChange);
    MUSIC.addListener(this, &appController::onMusicChange);
    NETWORK_SEND.addListener(this, &appController::onEnableNetwork);
    SAVE.addListener(this, &appController::onSave);
    LOAD.addListener(this, &appController::onLoad);
    CLEAR.addListener(this, &appController::onClear);
}

void appController::positionGui(){
    save_panel->setShowHeader(false);
    save_panel->setPosition(0, 0);
    
    camera_panel->setShowHeader(false);
    camera_panel->setPosition(0, save_panel->getHeight());
    
    network_panel->setShowHeader(false);
    network_panel->setPosition(0, save_panel->getHeight()+camera_panel->getHeight());
    
    tracking_panel->setShowHeader(false);
    tracking_panel->setPosition(camera_panel->getWidth(), 0);
    
    clear_panel->setShowHeader(false);
    clear_panel->setPosition(camera_panel->getWidth(), tracking_panel->getHeight());
    
    music_panel->setShowHeader(false);
    music_panel->setPosition(camera_panel->getWidth(), tracking_panel->getHeight()+clear_panel->getHeight());
    
    brush_panel->setShowHeader(false);
    brush_panel->setPosition(camera_panel->getWidth()+tracking_panel->getWidth(), 0);
    
    drip_panel->setShowHeader(false);
    drip_panel->setPosition(camera_panel->getWidth()+tracking_panel->getWidth(), brush_panel->getHeight());
}


//----------------------------------------------------
void appController::clearProjectedImage() {
    for (int i = 0; i < NUM_BRUSHES; i++) {
        brushes[i]->clear();
    }
}

//----------------------------------------------------
void appController::mainLoop() {
    
    if(bSetupCamera){
        setupCamera();
        bSetupCamera = false;
    }
    
    if(bSetupVideo){
        setupVideo();
        bSetupVideo = false;
    }
    
    //lets find dat laser!
    trackLaser();
    
    //if sending data is enabled
    //then lets send our data!
    if (SEND_DATA && laserTracking.newData()) {
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
    imageProjection.setProjectionBrightness(PROJ_BRIGHTNESS);
    
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
        
        laserSending.setup(ipStr, PORT.get());
    }
    else {
        laserSending.close();
        setCommonText("status: all network i/o closed");
    }
}

//----------------------------------------------------
void appController::handleNetworkSending() {
    if (laserSending.isSetup()) {
        laserSending.sendData(laserTracking.laserX, laserTracking.laserY, laserTracking.isStrokeNew());
    }
}

//----------------------------------------------------
void appController::trackLaser() {
    
    //lets see if we need to update the clearZone
    
    laserTracking.setUseClearZone(CLEAR_ZONE);
    laserTracking.setClearZone(CLEAR_X, CLEAR_Y, CLEAR_W, CLEAR_H);
    laserTracking.setClearThreshold(CLEAR_THRESH);
    
    //lets process the video data to track that laser
    laserTracking.processFrame(HUE_POINT, HUE_WIDTH, SAT_POINT, VAL_POINT, MIN_BLOB_SIZE, ACTIVITY, JUMP_DIST);
    
    //clear the images if the clear zone is hit
    if (laserTracking.isClearZoneHit()) {
        clearProjectedImage();
        setCommonText("status: clear zone hit - clearning image\n");
    }
    
    
    laserTracking.calcColorRange(HUE_POINT, HUE_WIDTH, SAT_POINT, VAL_POINT);
    
    
}

void appController::onCameraChange(bool&b){
    if(b) bSetupCamera = true;
    else bSetupVideo = true;
}

void appController::onMusicChange(bool& b) {
    if (MUSIC) {
        trackPlayer.unPause();
        trackPlayer.setVolume(VOL);
    }
    else {
        trackPlayer.pause();
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
        trackPlayer.unPause();
        trackPlayer.setVolume(VOL);
    }
    else {
        trackPlayer.pause();
    }
    
    
    if (TRACK != trackPlayer.getCurrentTrackNo()) {
        
        if (TRACK >= trackPlayer.getNumTracks()) {
            TRACK = 0;
        }
        trackPlayer.playTrack(whichTrack);
        setCommonText("Playing: " + ofToString(trackPlayer.getCurrentTrackNo()) + " " + trackPlayer.getCurrentTrackName());
    }
    
    if (trackPlayer.getFinished()) {
        whichTrack = trackPlayer.nextTrack();
        setCommonText("Playing: " + ofToString(trackPlayer.getCurrentTrackNo()) + " " + trackPlayer.getCurrentTrackName());
        TRACK = whichTrack;
    }
    trackPlayer.setVolume(VOL);
    
}

//----------------------------------------------------
void appController::updateBrushSettings(bool first) {
    
    colorManager.setCurrentColor(BRUSH_COLOR);
    unsigned char* rgb = colorManager.getColor3I();
    
    for (int i = 0; i < NUM_BRUSHES; i++) {
        brushes[i]->dripsSettings(DRIPS, DRIPS_FREQ, DRIPS_SPEED, DRIP_DIRECTION, DRIP_WIDTH);
        brushes[i]->setBrushWidth(BRUSH_WIDTH);
        if (i == BRUSH_MODE) brushes[i]->setBrushNumber(BRUSH_NO);
        brushes[i]->setBrushColor(rgb[0], rgb[1], rgb[2]);
        brushes[i]->setNumSteps(LINE_RES);
        brushes[i]->setBrushBrightness(PROJ_BRIGHTNESS);
    }
    
    imageProjection.setProjectionColor(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------
void appController::managePainting() {
    
    //time to paint!
    //but only if we have got data
    if (laserTracking.newData()) {
        
        float laserX = laserTracking.laserX;
        float laserY = laserTracking.laserY;
        
        //if our brush is a vector brush we need
        //to warp the coords as we are not
        //texture warping
        if (brushes[BRUSH_MODE]->getIsVector()) {
            laserTracking.getWarpedCoordinates(imageProjection.getQuadPoints(), &laserX, &laserY);
        }
        brushes[BRUSH_MODE]->addPoint(laserX, laserY, laserTracking.isStrokeNew());
        
        laserTracking.clearNewStroke();
    }
    
    //idle our current brush
    for (int i = 0; i < NUM_BRUSHES; i++) {
        if (i == BRUSH_MODE) {
            brushes[i]->update();
            
            //we need to update our brush style
            //as some brushes have different number
            //of brush styles
            
            BRUSH_NO = brushes[i]->getBrushNumber();
        }
    }
    
    //lets tell people which mode they are in
    
    setCommonText("brush: " + brushes[BRUSH_MODE]->getName() + " - " + brushes[BRUSH_MODE]->getDescription());
    
    
    if (brushes[BRUSH_MODE]->getIsColor()) {
        imageProjection.setColorTexture(brushes[BRUSH_MODE]->getTexture());
    }
    else {
        imageProjection.setGrayTexture(brushes[BRUSH_MODE]->getTexture());
    }
}

//----------------------------------------------------
void appController::saveSettings() {
    
    setCommonText("status: saving settings to xml");
    
    brush_panel->saveToFile(ofToDataPath("settings/brush_settings.xml"));
    drip_panel->saveToFile(ofToDataPath("settings/drip_settings.xml"));
    tracking_panel->saveToFile(ofToDataPath("settings/tracking_settings.xml"));
    clear_panel->saveToFile(ofToDataPath("settings/clear_settings.xml"));
    network_panel->saveToFile(ofToDataPath("settings/network_settings.xml"));
    music_panel->saveToFile(ofToDataPath("settings/music_settings.xml"));
    camera_panel->saveToFile(ofToDataPath("settings/camera_settings.xml"));
    laserTracking.QUAD.saveToFile(ofToDataPath("settings/quad.xml"));
    imageProjection.QUAD.saveToFile(ofToDataPath("settings/quadProj.xml"));
    
    setCommonText("status: settings saved to xml");
}

//----------------------------------------------------
void appController::reloadSettings() {
    
    setCommonText("status: reloading settings from xml");
    
    brush_panel->loadFromFile(ofToDataPath("settings/brush_settings.xml"));
    drip_panel->loadFromFile(ofToDataPath("settings/drip_settings.xml"));
    tracking_panel->loadFromFile(ofToDataPath("settings/tracking_settings.xml"));
    clear_panel->loadFromFile(ofToDataPath("settings/clear_settings.xml"));
    network_panel->loadFromFile(ofToDataPath("settings/network_settings.xml"));
    music_panel->loadFromFile(ofToDataPath("settings/music_settings.xml"));
    camera_panel->loadFromFile(ofToDataPath("settings/camera_settings.xml"));
    laserTracking.QUAD.loadSettings();
    imageProjection.loadSettings(ofToDataPath("settings/quadProj.xml"));
    
    setCommonText("status: settings reloaded from xml");
    
}

//----------------------------------------------------
void appController::selectPoint(float x, float y) {
    laserTracking.QUAD.selectPoint(x, y, noticeImg.getWidth(), 10, 320, 240, 60);
    imageProjection.selectMiniQuad(x, y, 30);
    imageProjection.updateMiniQuad(x, y);
}

void appController::selectPointProjector(float x, float y, float width, float height){
    imageProjection.selectQuad(x, y, 0, 0, width, height, 60);
}

void appController::dragPoint(float x, float y) {
   laserTracking.QUAD.updatePoint(x, y, noticeImg.getWidth(), 10, 320, 240);
   imageProjection.updateMiniQuad(x, y);
}

//----------------------------------------------------
void appController::dragPointProjector(float x, float y, float width, float height) {
    imageProjection.updateQuad(x, y, 0, 0, width, height);
}

//----------------------------------------------------
void appController::releasePoint() {
    laserTracking.QUAD.releaseAllPoints();
    imageProjection.releaseAllQuads();
}

//----------------------------------------------------
void appController::keyPress(int key) {
    
    if (key == 's') {
        saveSettings();
    }
    else if (key == 'r') {
        reloadSettings();
    }
    else if (key == 'c') {
        laserTracking.openCameraSettings();
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
void appController::keyPressProjector(int key) {
    if (key == 'f') {
        ofToggleFullscreen();
        PROJECTION_W = ofGetWindowWidth();
        PROJECTION_H = ofGetWindowHeight();
        setupProjections();
    }
    else if (key == ' ') {
        toggleGui = !toggleGui;
    }else if (key == 'd') {
         setCommonText("status: clearing projection");
         clearProjectedImage();
     }
}

//----------------------------------------------------
void appController::keyRelease(int key) {
    
}

//----------------------------------------------------
void appController::drawProjector() {
    ofBackground(0, 0, 0);
    ofPushStyle();
    ofSetColor(255, 255, 255, 255);
    
    //if we are a vector brush
    
    imageProjection.drawProjectionTex(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    
    
    if (webMovieLoaded) {
        VP.draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    }
    ofPopMatrix();
  
    
    if (toggleGui || SHOW_CHECKERBOARD){
        imageProjection.drawProjectionToolHandles(0, 0, ofGetWindowWidth(), ofGetWindowHeight(), false, true);
        drawCheckerBoard();
    }
    ofPopStyle();
}


//----------------------------------------------------
void appController::drawGUI() {
    ofPushStyle();
    ofSetColor(255, 255, 255);
    
    noticeImg.draw(0, ofGetHeight()-noticeImg.getHeight()-16);
    ofPushMatrix();
    {
        ofTranslate(420, ofGetHeight()-noticeImg.getHeight()-twentyTwentyImg.getHeight()/2);
        ofRotateDeg(15);
        twentyTwentyImg.draw(0, 0);
        
    }
    ofPopMatrix();
    
    laserTracking.draw(noticeImg.getWidth(), 10);
    
    ofPushMatrix();
    {
        ofTranslate(camera_panel->getWidth()+tracking_panel->getWidth(), brush_panel->getHeight()+drip_panel->getHeight());
        drawText("Colors tracked", 10, 23);
        laserTracking.drawColorRange(10, 29, 120, 44);
        
        ofSetColor(255, 255, 255);
        
        //put custom brush tool stuff here for your brush
        string brushName = "pngBrush";
        if (brushes[BRUSH_MODE]->getName() == brushName) {
            drawText("Current brush", 10, 97);
            brushes[BRUSH_MODE]->drawTool(10, 103, 32, 32);
        }
        
        drawText("Brush color", 10, 159);
        colorManager.drawColorPanel(10, 164, 128, 24, 5);
    }
    ofPopMatrix();

    imageProjection.drawMiniProjectionTool(noticeImg.getWidth(), 250, true, true);

    //make sure we have a black background
    drawStatusMessage();
    
    ofSetColor(0, 0, 0);
    drawText("fps: " + ofToString(ofGetFrameRate()), ofGetWindowWidth()-100, ofGetWindowHeight()-4);
    
    if (webMovieLoaded) {
        ofSetColor(255, 255, 255);
        VP.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    ofPopStyle();
}

void appController::drawCheckerBoard(){
    
    int width=ofGetWindowWidth();
    int height=ofGetWindowHeight();
    int squareSize = 120;
    int counter=0;
    
    
    checkerboardFBO.allocate(width, height, GL_RGBA);
    checkerboardFBO.begin();
    ofClear(0, 0, 0);
    ofPushStyle();
    for (int j=0; j<height;j=j+squareSize) {
        for (int i=0; i<width; i=i+squareSize*2){
            if (counter%2==0){
                ofSetColor(0);
                ofDrawRectangle(i, j, squareSize, squareSize);
                ofSetColor(255);
                ofDrawRectangle(i+squareSize, j, squareSize, squareSize);
            }else {
                ofSetColor(255);
                ofDrawRectangle(i, j, squareSize, squareSize);
                ofSetColor(0);
                ofDrawRectangle(i+squareSize, j, squareSize, squareSize);
            }
        }
        counter++;
    }
    ofPopStyle();
    checkerboardFBO.end();
    
    ofPushMatrix();
    ofMultMatrix(imageProjection.matrix);
    ofSetColor(255, 255, 255);
    checkerboardFBO.draw(0, 0);
    ofPopMatrix();
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
        ofDrawRectangle(0, ofGetHeight()-16, ofGetWidth(), 16);
        
        ofSetColor(txtFade, 0, 0);
        drawText(getCommonText(), 5, ofGetHeight()-4);
        txtFade--;
        setFade(txtFade);
    }
}


void appController::onSave(bool & b){
    saveSettings();
    SAVE = false;
}
void appController::onLoad(bool & b){
    reloadSettings();
    LOAD = false;
}
void appController::onClear(bool & b){
    CLEAR = false;
    setCommonText("status: clearing projection");
    clearProjectedImage();
}

void appController::exit(){
    if(MUSIC){
        trackPlayer.stop();
    }
}
