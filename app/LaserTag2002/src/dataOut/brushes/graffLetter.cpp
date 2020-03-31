
#include "graffLetter.h"

//----------------------------------------------
void graffLetter::setupCustom(){

	//we are a raster brush
	isVector = false;
	
	//we are greyscale
	isColor	 = false;
	
	//our name and description
	setName("graffLetters");
	setShortDescription("drop shadow letters with drips");

	//we store our previous coordinates
	oldX = 0;
	oldY = 0;
		
	//opencv stuff
	cvImgWhite.allocate(width,height);
	cvImgWhite.set(0);
	cvImgBlack.allocate(width,height);
	cvImgBlack.set(0);
	
	cvImgBottom.allocate(width,height);
	cvImgBottom.set(0);
	cvImgTop.allocate(width,height);
	cvImgTop.set(0);
	
	cvImgMix.allocate(width,height);
	cvImgMix.set(0);
	
	cvTopLayerMask.allocate(width,height);
	cvTopLayerMask.set(0);
	cvBottomLayerMask.allocate(width,height);
	cvBottomLayerMask.set(0);

	setBrushWidth(40);
	dripsSettings(false, 10, 0.5, 0);
	
	//-------------------------------------------  d r i p s 
	for (int i = 0; i < MAX_DRIP_PARTICLES; i++){
		particles[i].bOn = false;
	}
	nDrip = 0;
	//-------------------------------------------	
	
}

//----------------------------------------------
void graffLetter::setBrushWidth(int width){
	
	brushWidth = width;
	
	float ratio1 = 32.0/40.0;
	float ratio2 = 120.0/40.0;
	
	setupBrush(cvWhiteBrush, brushWidth);
	setupBrush(cvBlackBrush, (float)brushWidth * ratio1);
	setupBrush(cvDripWhiteBrush, (int)(brushWidth/2.5f));
	setupBrush(cvDripBlackBrush, (int)(brushWidth/2.5f) * ratio1);
	setupShadowBrush(cvDripDropShadowBrush, (float)(brushWidth * ratio2) / 2.5f );
	
	setupShadowBrush(cvDropShadowBrush,(float)brushWidth * ratio2);	
}


//----------------------------------------------
void graffLetter::setupBrush(ofCvGrayscaleImage &brush, int width){
	brush.allocate(width,width);
	unsigned char * temp = new unsigned char[width*width];
	float midPt 	= (float)width/2.0f;
	float radius 	= (float)width/2.0f;
	for (int i = 0; i < width; i++){
		for (int j = 0; j < width; j++){
			float dist = (sqrt((midPt-i)*(midPt-i) + (midPt-j)*(midPt-j)) / radius);
			if (dist < 1){
				float shape = powf((1-dist), 2.0f);
				temp[j*width + i] = 255;//(shape) * 100.0f;	
			} else {
				temp[j*width + i] = 0;
			}
		}
	}
	brush.setFromPixels(temp, width, width);
	delete temp;
}

//----------------------------------------------
void graffLetter::setupShadowBrush(ofCvGrayscaleImage &brush, int width){
	
	brush.allocate(width,width);
	unsigned char * temp = new unsigned char[width*width];
	float midPt 	= (float)width/2.0f;
	float radius 	= (float)width/2.0f;
	for (int i = 0; i < width; i++){
		for (int j = 0; j < width; j++){
			float dist = (sqrt((midPt-i)*(midPt-i) + (midPt-j)*(midPt-j)) / radius);
			if (dist < 1){
				float shape = powf((1-dist), 2.0f);
				temp[j*width + i] = (shape) * 100.0f;	
			} else {
				temp[j*width + i] = 0;
			}
				
		}
	}
	brush.setFromPixels(temp, width, width);
	brush.blurHeavily();
	delete temp;
}

//----------------------------------------------
void graffLetter::update(){
	
	for (int i = 0; i < MAX_DRIP_PARTICLES; i++){
		particles[i].vx *= 0.99f;
		particles[i].vy *= 0.99f;
		
		particles[i].x += particles[i].vx;
		particles[i].y += particles[i].vy;
		if ((fabs(particles[i].vx) < 0.0001) && (fabs(particles[i].vy) < 0.0001)){
			particles[i].bOn = false;
		}
	}
	
	
	if(dripsEnabled){
		for (int i = 0; i < MAX_DRIP_PARTICLES; i++){
			if (particles[i].bOn == true){
				addDrip(particles[i].x, particles[i].y, particles[i].x, particles[i].y);
			}
		}
	}
	
	cvImgBottom.set(0);
	cvImgBottom += cvImgWhite;
	cvImgBottom -= cvImgBlack;

	cvImgMix = cvImgTop;
	cvImgMix.composite(cvImgBottom, cvBottomLayerMask);
}

//----------------------------------------------
unsigned char * graffLetter::getImageAsPixels(){
	return cvImgMix.getPixels();
}

//----------------------------------------------
void graffLetter::newLetter(){
		
	cvImgBottom.set(0);
	cvImgBottom += cvImgWhite;
	cvImgBottom -= cvImgBlack;
	
	cvImgTop.composite(cvImgBottom, cvBottomLayerMask);
	cvImgWhite.set(0);
	cvImgBlack.set(0);
	cvBottomLayerMask.set(0);
	
	for (int i = 0; i < MAX_DRIP_PARTICLES; i++){
		particles[i].bOn = false;
	}
}

//----------------------------------------------
void graffLetter::clear(){
	cvImgBottom.set(0);
	cvImgTop.set(0);
	cvImgWhite.set(0);
	cvImgBlack.set(0);
	cvBottomLayerMask.set(0);
	
	for (int i = 0; i < MAX_DRIP_PARTICLES; i++){
		particles[i].bOn = false;
	}
}

//-----------------------------------------------
void graffLetter::addPoint(float x1, float y1, bool newStroke){
	
	//scale up to our dimensions
	x1 *= (float)width;
	y1 *= (float)height;
	
	//printf("addingPoint(%i, %i)\n", (int)x1, (int)y1);
	
	if(newStroke){
		
		newLetter();
	
		oldX = x1;
		oldY = y1;
	}
	
	addLine(x1, y1, oldX, oldY);
		
	oldX = x1;
	oldY = y1; 
}

//----------------------------------------------
void graffLetter::addLine(float x1, float y1, float x2, float y2){
	
	
	int nDivs 	= 18;
	float dx 	= (float)(x2 - x1)/(float)nDivs;
	float dy 	= (float)(y2 - y1)/(float)nDivs;
	
	if(dripsEnabled){
		if (ofRandom(0,dripsFrequency) > dripsFrequency-1){
			float pct = ofRandom(0,1);
			//printf("here pickin \n");
			float x = x1 + (x2-x1)*pct;
			float y = y1 + (y2-y1)*pct;
			
			particles[nDrip].bOn = true;
			particles[nDrip].x = x;
			particles[nDrip].y = y;
			particles[nDrip].vx =  ofRandom(-0.05, 0.05);; //// 0.0002;
			particles[nDrip].vy = ofRandom(0.8, 1.0);
			nDrip++;
			nDrip %= MAX_DRIP_PARTICLES;
		}
	}
	
	// white
	for (int j = 0; j < nDivs; j++){
		cvImgWhite.addIntoMe(cvWhiteBrush, x1 + dx*j-(brushWidth*0.5f), y1 + dy*j-(brushWidth*0.5f));
		cvBottomLayerMask.addIntoMe(cvWhiteBrush, x1 + dx*j-(brushWidth*0.5f), y1 + dy*j-(brushWidth*0.5f));
	}
	
	// offset!
	for (int j = 0; j < nDivs; j++){
		cvImgWhite.addIntoMe(cvWhiteBrush, x1 + dx*j-(brushWidth*0.75f), y1 + dy*j-(brushWidth*0.75f));
		cvBottomLayerMask.addIntoMe(cvWhiteBrush, x1 + dx*j-(brushWidth*0.75f), y1 + dy*j-(brushWidth*0.75f));
	}
	
	cvBottomLayerMask.addIntoMe(cvDropShadowBrush, x1 -(brushWidth*1.5f), y1 -(brushWidth*1.5f));
	
	
	
	// black 
	for (int j = 0; j < nDivs; j++){
		cvImgBlack.addIntoMe(cvBlackBrush, x1 + dx*j-(brushWidth*(17.0/40.0)), y1 + dy*j-(brushWidth*(17.0/40.0)));
	}
		
}



//----------------------------------------------
void graffLetter::addDrip(float x1, float y1, float x2, float y2){
	
	
	int nDivs 	= 1;
	float dx 	= (float)(x2 - x1)/(float)nDivs;
	float dy 	= (float)(y2 - y1)/(float)nDivs;
	
	
	
	
	// white
	for (int j = 0; j < nDivs; j++){
		cvImgWhite.addIntoMe(cvDripWhiteBrush, x1 + dx*j-(brushWidth*(5.0/40.0)), y1 + dy*j-(brushWidth*(5.0/40.0)));
		cvBottomLayerMask.addIntoMe(cvDripWhiteBrush, x1 + dx*j-(brushWidth*(5.0/40.0)), y1 + dy*j-(brushWidth*(5.0/40.0)));
	}
	
	// offset!
	for (int j = 0; j < nDivs; j++){
		cvImgWhite.addIntoMe(cvDripWhiteBrush, x1 + dx*j-(brushWidth*(10.5/40.0)), y1 + dy*j-(brushWidth*(10.5/40.0)));
		cvBottomLayerMask.addIntoMe(cvDripWhiteBrush, x1 + dx*j-(brushWidth*(7.5/40.0)), y1 + dy*j-(brushWidth*(7.5/40.0)));
	}
	
	cvBottomLayerMask.addIntoMe(cvDripDropShadowBrush, x1 -(brushWidth*(15/40.0)), y1 -(brushWidth*(15/40.0)));
	
	
	
	// black 
	for (int j = 0; j < nDivs; j++){
		cvImgBlack.addIntoMe(cvDripBlackBrush, x1 + dx*j-(brushWidth*(4.25/40.0)), y1 + dy*j-(brushWidth*(4.25/40.0)));
	}
	
}

