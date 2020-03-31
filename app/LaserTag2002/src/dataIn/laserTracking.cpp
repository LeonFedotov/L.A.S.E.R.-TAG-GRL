#include "laserTracking.h"


static int pnpoly (int npol, float *xp, float *yp, float x, float y)

 {
   int i, j, c = 0;
   for (i = 0, j = npol-1; i < npol; j = i++) {
     if ((((yp[i]<=y) && (y < yp[j])) ||
          ((yp[j]<=y) && (y < yp[i]))) &&
           (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))
       c = !c;
   }
   return c;
 }


//---------------------------
laserTracking::laserTracking(){

	bCameraSetup 	= false;
	bVideoSetup  	= false;
	newStroke		= true;
	shouldClear		= false;
	noLaserCounter 	= 0;
	distDifference  = 0.0;
	smoothPos.x 	= 0;
	smoothPos.y 	= 0;
	smoothVel.x 	= 0;
	smoothVel.y 	= 0;
	distX			= 0;
	distY			= 0;
	oldX			= 0;
	oldY			= 0;
	clearThresh		= 6;
	
}

//lower threshold means more senstive clear zone
//---------------------------
void laserTracking::setClearThreshold(float clearSens){
	clearThresh = clearSens;
}

//
//---------------------------
void laserTracking::setClearZone(float x, float y, float w, float h){
	clearZone.setPosition(x/(float)W, y/(float)H);
	clearZone.setDimensions(w/(float)W, h/(float)H);
}

//
//---------------------------
void laserTracking::setUseClearZone(bool useClearZone){
	clearZone.setActive(useClearZone);
}

//---------------------------
bool laserTracking::isClearZoneHit(){
	if( shouldClear ){
		shouldClear = false;
		return true;
	}
	return false;
}
		
//changing cameras or switching from/to the camera mode
//requires the app to be restarted - mabe we can change this?
//---------------------------		
void laserTracking::setupCamera(int deviceNumber, int width, int height){
	VG.setDeviceID(deviceNumber);
	VG.initGrabber(width, height);
	W = VG.width;
	H = VG.height;			
	bCameraSetup = true;
	noLaserCounter  = 0;
	distDifference  = 0.0;
}

//same as above - from/to using test movies
//requires a restart
//---------------------------		
void laserTracking::setupVideo(string videoPath){
	VP.loadMovie(videoPath);
	VP.play(); 
	VP.setUseTexture(true);
	W = VP.width;
	H = VP.height;
	bVideoSetup = true;
	noLaserCounter = 0;
	distDifference  = 0.0;
}

//good for adjusting the color balance, brightness etc
//---------------------------		
void laserTracking::openCameraSettings(){
	VG.videoSettings();			
}


//do all our openCV allocation
//and loads our quad settings from xml
//---------------------------		
void laserTracking::setupCV(string filePath){
				
	//our openCV inits
	VideoFrame.allocate(W, H);
	WarpedFrame.allocate(W, H);
	PresenceFrame.allocate(W, H);
				
	//we do all our tracking at 320 240 - regardless 
	//of camera size - larger cameras get scaled down to
	//these dimensions - otherwise 'shit would be slow'
	pre	= new unsigned char[W * H * 3];
	
	//this is so we can select a sub region of the 
	//camera image and warp it to full 320 by 240
	warpSrc[0].x = 0;
	warpSrc[0].y = 0;
	
	warpSrc[1].x = 1.0;
	warpSrc[1].y = 0;
	
	warpSrc[2].x = 1.0;
	warpSrc[2].y = 1.0;

	warpSrc[3].x = 0;
	warpSrc[3].y = 1.0;
	
	//Now we update out quad with the default
	//values and if then if the file is found
	//we overwrite with values from xml file
	QUAD.setup("QUAD_SRC_");
	QUAD.setQuadPoints(warpSrc);
	QUAD.readFromFile(filePath);
	
	//warp dst will always be to 320 by 240
	//unless you want shit slow!
	warpDst[0].x = 0;
	warpDst[0].y = 0;
	
	warpDst[1].x = W;
	warpDst[1].y = 0;
	
	warpDst[2].x = W;
	warpDst[2].y = H;

	warpDst[3].x = 0;
	warpDst[3].y = H;	
}

//the heart of the beast - where we process 
//the incoming frame and look for a laser
//---------------------------		
void laserTracking::processFrame(float hue, float hueThresh, float sat, float value, int minSize, int deadCount, float jumpDist, bool slowButAccurateQuad){
	
	int t1, t2, t3, t4, t5;
	
	accurateQuad = slowButAccurateQuad;
	
	//int t0 = ofGetElapsedTimeMillis();
	
	///////////////////////////////////////////////////////////
	// Part 1 - get the video data
	///////////////////////////////////////////////////////////
	
	//pointer to our incoming video pixels			
	unsigned char * pixCam;
	
	//either grab pixels from video or grab from camera
	if(bVideoSetup){
		VP.idleMovie();	
		pixCam 	= VP.getPixels();
	}else{
		VG.grabFrame();
		pixCam 	= VG.getPixels();		
	}
	
	
	//t1 = ofGetElapsedTimeMillis();

	
	///////////////////////////////////////////////////////////
	// Part 2 - warp the video based on our quad
	///////////////////////////////////////////////////////////
	
	//add to openCV and warp to our dst image
	VideoFrame.setFromPixels(pixCam, W, H);
	
	if(accurateQuad){
		WarpedFrame.warpIntoMe(VideoFrame, QUAD.getScaledQuadPoints(W,H), warpDst);
	}
	
	
	//WarpedFrame.wao hsv and search for matching pixels
	////////////////////////////////////////////////////////////
				
	//Get pixels and convert to hue sat and value;
	hsvFrame = VideoFrame;
	hsvFrame.rgbToHsv();
	
	//okay time to look for our laser!
	//based on hue sat and val
	int totalPixels = W*H*3;
	
	unsigned char * pix = hsvFrame.getPixels();
				
	float h 		= hue 		* 255;
	float ht		= hueThresh * 255; 
	float s 		= sat 		* 255;
	float v 		= value 	* 255;
	
	//here we figure out what is the max allowed hue 
	//and the minimum allowed hue. because hue is 
	//continious we have to make sure we handle what happens 
	//if hueMax goes over 255
	//or hueMin goes less than 0
					
	float hueMax	= h + ht*0.5;
	float hueMin    = h - ht*0.5;
		
	int k = 0;
	int x = 0;
	int y = 0;

	//our clear zone stuff
	//back to being able to use any area of the camera
	//as the clear zone - even if it is outide of the quad
	
	int   clearCount 	= 0;
	bool  clearActive 	= clearZone.getActive();
	float clearXMin 	= clearZone.points[0].x * (float)W;
	float clearYMin 	= clearZone.points[0].y * (float)H;
	float clearXMax 	= clearZone.points[1].x * (float)W;
	float clearYMax 	= clearZone.points[2].y * (float)H;
	
	
	if(accurateQuad){
		
		int i, stride;
		for(int y = clearYMin; y < clearYMax; y++){
			
			stride = y*W*3;
			for(int x = clearXMin; x < clearXMax; x++){

				i = stride + x*3;
				
				if( pix[i+1] >= s  && pix[i+2] >= v){

					 //we do this to check the cases when the
					 //hue min could have wrapped
					 //or the hue max could have wrapped
					 //also if saturation is zero
					 //then hue doesn't matter hence (s == 0)         
					  float pixHue = pix[i];

					 if( (s == 0) || (pixHue >= hueMin && pixHue <= hueMax) ||
					    (pixHue -255 >= hueMin && pixHue -255 <= hueMax) ||
					    (pixHue +255 >= hueMin && pixHue +255 <= hueMax) ){
					   
					   
						if( clearActive && x > clearXMin && x < clearXMax){				   
							if( y > clearYMin && y < clearYMax){
								clearCount++;
							}
						}
					}
				}
			}
		}
		
							
		//Get pixels and convert to hue sat and value;
		hsvFrame = WarpedFrame;
		hsvFrame.rgbToHsv();
		pix = hsvFrame.getPixels();
		
		k = 0;
						
		for(int i = 0; i < totalPixels; i+=3){
		 
		  float pixHue = pix[i];
		           
		  if( pix[i+1] >= s  && pix[i+2] >= v){
		 
		     //we do this to check the cases when the
		     //hue min could have wrapped
		     //or the hue max could have wrapped
		     //also if saturation is zero
		     //then hue doesn't matter hence (s == 0)               
		     if( (s == 0) || (pixHue >= hueMin && pixHue <= hueMax) ||
				(pixHue -255 >= hueMin && pixHue -255 <= hueMax) ||
				(pixHue +255 >= hueMin && pixHue +255 <= hueMax) ){
		       
		        //we have a white pixel
		        pre[k] = 255;
		     }
		     else pre[k] = 0;
		  }else pre[k] = 0;
		           
		  k++;
		} 	
		
		
	}else{
	
		ofPoint2f * pts =  QUAD.getScaledQuadPoints(W, H);
	
		float ptsx[4] = {pts[0].x, pts[1].x, pts[2].x, pts[3].x};
		float ptsy[4] = {pts[0].y, pts[1].y, pts[2].y, pts[3].y};
		
		k = 0;
		x = 0;
		y = 0;
	
		for(int i = 0; i < totalPixels; i+=3){

			if( x >= W){
				x = 0;
				y++;
			}
			
			if( pix[i+1] >= s  && pix[i+2] >= v){

			 //we do this to check the cases when the
			 //hue min could have wrapped
			 //or the hue max could have wrapped
			 //also if saturation is zero
			 //then hue doesn't matter hence (s == 0)         
			  float pixHue = pix[i];

			 if( (s == 0) || (pixHue >= hueMin && pixHue <= hueMax) ||
			    (pixHue -255 >= hueMin && pixHue -255 <= hueMax) ||
			    (pixHue +255 >= hueMin && pixHue +255 <= hueMax) ){
			   
			   
			   if( clearActive && x > clearXMin && x < clearXMax){				   
			   		if( y > clearYMin && y < clearYMax){
			   			clearCount++;
			   		}
			   }
			   
			   //if we are not in the polygon
				//don't draw
									
				if( !pnpoly(4, ptsx, ptsy, x, y) ){
					pre[k] = 0;
					x++;
					k++;
					continue;
				}
								   
			    //we have a white pixel
			    pre[k] = 255;
			 }
			 else pre[k] = 0;
			}else pre[k] = 0;
			       
			x++;
			k++;
		}
		
	} 			
	
	if(clearActive && clearCount >= clearThresh){
		shouldClear = true;
	}
	
	//t3 = ofGetElapsedTimeMillis();

	
	///////////////////////////////////////////////////////////
	// Part 4 - find the largest blob of possible candidates 
	////////////////////////////////////////////////////////////
					
	//lets find blobs!
	PresenceFrame.setFromPixels(pre, W, H);
	
	//max blob size - our laser shouldn't be bigger
	//than 20 pixels by 20 pixels right?
	//so lets make our max blob rpIntoMe(VideoFrame, QUAD.getScaledQuadPoints(W,H), warpDst);

	//t2 = ofGetElapsedTimeMillis();


	///////////////////////////////////////////////////////////
	// Part 3 - convert tsize 400

	int maxSize = 999999999;		
	Contour.findContours(PresenceFrame, minSize, maxSize, 50, false);  
	
	//t4 = ofGetElapsedTimeMillis();

	//printf("v%i - w%i - h%i - b%i = %i\n", t1-t0, t2-t1, t3-t2, t4-t3, t4-t0);


	///////////////////////////////////////////////////////////
	// Part 5 - finally calculate our laser coordinates
	////////////////////////////////////////////////////////////
								
	//okay so we found some blobs that matched our criteria
	//we are only really interested in the largest one
	if(Contour.nBlobs > 0){
		
		//get the center pos of the largest blob in 0 - 1 range
		float tmpX		= Contour.blobs[0].centroid.x/(float)W;
		float tmpY		= Contour.blobs[0].centroid.y/(float)H;
		
		ofPoint2f dst[4];
		
		dst[0].x = 0;
		dst[0].y = 0;

		dst[1].x = 1;
		dst[1].y = 0;

		dst[2].x = 1;
		dst[2].y = 1;
		
		dst[3].x = 0;
		dst[3].y = 1;

		if(!accurateQuad){
		
			ofPoint2f *  src = QUAD.getQuadPoints();

			CW.calculateMatrix(src, dst);
			ofPoint2f out = CW.transform(tmpX, tmpY);
			
			tmpX = out.x;
			tmpY = out.y;
			
		}
					
		//calculate the horizontal and vertical distance 
		//between the last point
		oldX			= laserX;
		oldY			= laserY;
		
		distX			= tmpX - laserX;
		distY			= tmpY - laserY;
		
		if(distX == 0 && distY == 0)newPos = false;
		else newPos = true;

		distDifference  = sqrt( pow(distX, 2) + pow(distY, 2) );
		
		//now update our laser position with this new position
		laserX 			= tmpX;
		laserY			= tmpY;
		noLaserCounter 	= 0;
		
	}else{
		//we are waiting for a laser!
		newPos = false;
		noLaserCounter++;
		
		distX			= 0;
		distY			= 0;			
	}
	
	///////////////////////////////////////////////////////////
	// Part 6 - calculate related variables, smoothing etc
	////////////////////////////////////////////////////////////

	// we need to store if a new stroke has occured
	// this is so we can no when to start a new line 
	if(!newStroke) newStroke = (noLaserCounter >= deadCount) ||  (distDifference >= jumpDist);

	if(newStroke){
	
		oldX		= laserX;
		oldY		= laserY;
		smoothPos.x = laserX;
		smoothPos.y = laserY;
		distX		= 0;
		distY		= 0;
		smoothVel.x = 0;
		smoothVel.y = 0;
		
	}
	
} 

//if you want to warp the coords to another quad
//-----------------------------------------------------------------------
void laserTracking::getWarpedCoordinates(ofPoint2f * dst, float *warpedX, float *warpedY){
		
	ofPoint2f srcTmp[4];
		
		srcTmp[0].x = 0;
		srcTmp[0].y = 0;

		srcTmp[1].x = 1;
		srcTmp[1].y = 0;

		srcTmp[2].x = 1;
		srcTmp[2].y = 1;
		
		srcTmp[3].x = 0;
		srcTmp[3].y = 1;
		

	CW.calculateMatrix(srcTmp, dst);
	ofPoint2f out = CW.transform(laserX, laserY);
	
	*warpedX = out.x;
	*warpedY = out.y;
}


//---------------------------
bool laserTracking::newData(){		
	return newPos;
}

//---------------------------
void laserTracking::clearNewStroke(){
	newStroke = false;
}

//---------------------------
bool laserTracking::isStrokeNew(){
	return newStroke;
}


//this formats the x and y coordinates
//to a string that can be sent to other apps
//over tcp and udp we will see both regular
//and transformed coordinates
//---------------------------		
string laserTracking::getLaserPointString(){
	
	if( newData() ){
		sendStr = ofToString(laserX);
		sendStr += "!"+ ofToString(laserY);
		//printf("sending %s \n", sendStr.c_str());
	}
	
	return sendStr;
}


//---------------------------		
void laserTracking::calcColorRange(float hue, float hueThresh, float sat, float value){
	
	//ehh this is kind of ugly and needs to be fixed I think
	//Lets calc the rgb values for the min and max hue range			
	float rgb[3];
	float hsb[3];
	
	hsb[0] = hue-hueThresh*0.5;
	hsb[1] = sat;
	hsb[2] = value;
	
	hsbToRgb(hsb, rgb);
	
	r0Min = (int)(rgb[0]*255);
	g0Min = (int)(rgb[1]*255);
	b0Min = (int)(rgb[2]*255);
	
	hsb[0] = hue+hueThresh*0.5;
	hsb[1] = sat;
	hsb[2] = value;
			
	hsbToRgb(hsb, rgb);
						
	r0Max = (int)(rgb[0]*255);
	g0Max = (int)(rgb[1]*255);
	b0Max = (int)(rgb[2]*255);
	
	hsb[0] = hue-hueThresh*0.5;
	if(sat == 0) hsb[1] = 0.0;
	else hsb[1] = 1.0;			
	hsb[2] = 1.0;
	
	hsbToRgb(hsb, rgb);
	
	r1Min = (int)(rgb[0]*255);
	g1Min = (int)(rgb[1]*255);
	b1Min = (int)(rgb[2]*255);
	
	hsb[0] = hue+hueThresh*0.5;
	if(sat == 0) hsb[1] = 0.0;
	else hsb[1] = 1.0;	
	hsb[2] = 1.0;
			
	hsbToRgb(hsb, rgb);
						
	r1Max = (int)(rgb[0]*255);
	g1Max = (int)(rgb[1]*255);
	b1Max = (int)(rgb[2]*255);
	
}


//---------------------------				
void laserTracking::draw(float x, float y){
	
	//lets draw our openCV frames
	//with a nice border around them
	ofNoFill();
	ofSetColor(0xFFFFFF);	
	VideoFrame.draw(0, 0, 320, 240);
	ofRect(0,0,320,240);
	
	if(accurateQuad)WarpedFrame.draw(320,0, 320, 240);
	else PresenceFrame.draw(320,0, 320, 240);
	ofNoFill();
	ofRect(320,0,320,240);
	ofFill();
	
	//lets draw the contours to show our blobs!
	glPushMatrix();
		glTranslatef(320, 0, 0);
		glScalef(320.0/(float)W, 240.0/(float)H, 1);
		ofSetColor(0xFF00FF);
		//Contour.draw();
	glPopMatrix();
	
	if(clearZone.getActive())drawClearZone(0, 0, 320, 240);
	
	//lets draw our camera quad
	QUAD.draw(0,0,320,240);
	
}


//---------------------------		
void laserTracking::drawColorRange(float x, float y, float w, float h){

	//this is not super accurate
	//but good for an indication
	glBegin(GL_QUADS);
		ofSetColor(r1Min, g1Min, b1Min);
		glVertex2f(x,  y);
		
		ofSetColor(r1Max, g1Max, b1Max);				
		glVertex2f(x+w, y);
		
		ofSetColor(r0Max, g0Max, b0Max);
		glVertex2f(x+w, y + h );

		ofSetColor(r0Min, g0Min, b0Min);
		glVertex2f(x, y + h);
	glEnd();

	
	glBegin(GL_LINE_LOOP);
	ofSetColor(0xFFFFFF);
		glVertex2f(x, y);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + h);
		glVertex2f(x  ,   y + h);
	glEnd();

}


//---------------------------
void laserTracking::drawClearZone(float x, float y, float w, float h){
	ofNoFill();
	ofSetColor(0xFF0000);
	
	float xdraw = x + (clearZone.zoneDimensions[0].x * w);
	float ydraw = y + (clearZone.zoneDimensions[0].y * h);
	float wdraw =      clearZone.zoneDimensions[1].x * w;
	float hdraw =      clearZone.zoneDimensions[1].y * h;

	ofRect(xdraw, ydraw, wdraw, hdraw); 				
	ofFill();
}

//---------------------------		
void laserTracking::drawQuadSetupImage(float x, float y, float w, float h){
	ofSetColor(0xFFFFFF);	
	ofRect(0,0,w, h);
	VideoFrame.draw(16, 12, w-32, h-24);
	ofSetColor(0xFFFF00);	
	QUAD.draw(16, 12, w-32, h-24);
}
