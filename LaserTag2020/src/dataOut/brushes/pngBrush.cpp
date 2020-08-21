#include "pngBrush.h"

//--------------------------
void pngBrush::setupCustom(){

	//we are a raster brush
	isVector = false;

	//we are color
	isColor	 		= true;
	imageNumBytes 	= 3;	
	
	//our name and description
	setName("pngBrush");
	setShortDescription("multi color - make your own png brushes");
	
	//our previous points
	oldX			= 0;
	oldY			= 0;
	
	//other vars
	numBrushes  	= 0;
	brushNumber		= -1;
	numSteps		= 100;
	brushWidth		= 18;
	
	//default to white
	red				= 255;
	green			= 255;
	blue			= 255;	
	
	//our default drip settings
	dripsSettings(false, 8, 0.05, 0);
	
	DRIPS.setup(width, height);
    FBO.allocate(width, height, GL_RGBA);
    clear();
    
	//load brushes
	loadbrushes(ofToDataPath("brushes/"));
}


//------------------------
void pngBrush::clear(){
    FBO.begin();
    ofClear(0, 0, 0, 0);
    FBO.end();
	DRIPS.clear();
}


//reads a directoty for png files
//loads them into imagebrushNumber as the different brushes
//format should be b&w 16 by 16 png with white being the 
//brushNumber shape
//------------------------
int pngBrush::loadbrushes(string brushDir){
	
	DL.allowExt("png");
	numBrushes = DL.listDir(brushDir);
				
	//set a default brush
	if(numBrushes > 0){
		brushNumber = 0;
		setBrushNumber(brushNumber);
	}
	
	return numBrushes;
}

//-----------------------------------------------------		
void pngBrush::setBrushNumber(int _num){

	if(numBrushes == 0)return;

	if(_num >= numBrushes){
		_num = numBrushes-1;
	}
	
	brushNumber = _num;
				
	IMG.load(DL.getPath(brushNumber));
	
	 //no need for color
	 //we will use the image as a mask to 
	 //draw with and multiply it with
	 //our drawing color.
	IMG.setImageType(OF_IMAGE_COLOR_ALPHA);
	TMP.setImageType(OF_IMAGE_COLOR_ALPHA);
	updateTmpImage(brushWidth);
    
    

}

//-----------------------------------------------------		
void pngBrush::setBrushWidth(int width){

	if(numBrushes == 0)return;
	
	//only resize the brushNumber when its size has changed
	if(brushWidth != width){		
		brushWidth = width;
		updateTmpImage(brushWidth);
	}	

}

//-----------------------------------------------------
void pngBrush::dripsSettings(bool doDrip, int dripFreq, float speed, int direction){
	dripsEnabled 	= doDrip;
	dripsFrequency 	= dripFreq;
	dripsDirection	= direction;
	dripsSpeed		= speed;
	
	DRIPS.setDirection(dripsDirection);
	DRIPS.setSpeed(dripsSpeed);
}

//-----------------------------------------------------
void pngBrush::updateTmpImage(float _brushWidth){
	
    TMP = IMG;
	
	float tmpW 		= TMP.getWidth();
	float tmpH 		= TMP.getHeight();
	
	float ratio 	= (float)_brushWidth/tmpW;
		
	int newWidth  	= _brushWidth;
	int newHeight  	= tmpH*ratio;
	
	TMP.resize(newWidth, newHeight);
    
    for(int i = 0; i < newWidth*newHeight*4; i+=4){
        TMP.getPixels()[i+3] = TMP.getPixels()[i];
    }
}

//-----------------------------------------------------
void pngBrush::setBrushColor(unsigned char r, unsigned char g, unsigned b){
	red		= r;
	green	= g;
	blue 	= b;
			
	DRIPS.setColor((float)r, (float)g, (float)b);
}

//-----------------------------------------------------
string pngBrush::getbrushName(){
	return DL.getName(brushNumber);
}

//-----------------------------------------------------
void pngBrush::update(){
	
	DRIPS.updateDrips(pixels);
}

//-----------------------------------------------------
void pngBrush::drawTool(int x, int y, int w, int h){
	ofSetHexColor(0xFFFFFF);
	IMG.draw(x,y,w,h);
}

//-----------------------------------------------------
void pngBrush::addPoint(float _x, float _y, bool newStroke){
	
	//scale up to our dimensions
	_x *= (float)width;
	_y *= (float)height;
	
	//make sure we have a brush to use
	if(brushNumber < 0){
		printf("pngBrush - no brushes found to draw with\n");
		return;
	}

	if ( dripsEnabled && ofRandom(0,dripsFrequency) > dripsFrequency-1){
		DRIPS.addDrip(_x, _y);
	}
				
	//calc some info for where to draw from						
	int pix			= 0;
	int stride 		= width*imageNumBytes;
	int startX 		= _x;
	int startY 		= _y;
	int steps		= numSteps;
	
	int tx = 0;
	int ty = 0;
	float dx = 0;
	float dy = 0;
			
	if(newStroke){
		steps	= 1;	//then just one image
		oldX	= startX;
		oldY	= startY;
		dx		= 0;
		dy		= 0;
	}else{
		dx = ((float)(startX - oldX)/steps);
		dy = ((float)(startY - oldY)/steps);
	}
            
				
	//if no distance is to be travelled
	//draw only one point
	if(dx == 0 && dy == 0) steps = 1;
	
	// lets draw the brushNumber many times to make a line
	for(int i = 0; i < steps; i++){				
							
		if(newStroke){
			tx = startX - TMP.getWidth()/2;
			ty = startY - TMP.getHeight()/2;
		}else{
			tx = (oldX + (int)(dx*(float)i)) - TMP.getWidth()/2;
			ty = (oldY + (int)(dy*(float)i)) - TMP.getHeight()/2;
		}
						
        ofSetRectMode(OF_RECTMODE_CENTER);
        ofPushStyle();
        ofEnableAlphaBlending();
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        FBO.begin();
        ofSetColor(255, 255, 255);
        TMP.draw(tx, ty);
        FBO.end();
        ofDisableAlphaBlending();
        ofPopStyle();
        ofSetRectMode(OF_RECTMODE_CORNER);
	}
	
	oldX = startX;
	oldY = startY;
}
	
ofTexture & pngBrush::getTexture(){
    return FBO.getTexture();
}

//-----------------------------------------------------
unsigned char * pngBrush::getImageAsPixels(){
	return pixels;
}


//-----------------------------------------------------
void pngBrush::drawBrushColor(float x, float y, int w, int h){
	ofSetColor(red, green, blue);
	ofDrawRectangle(x,y,w,h);
}
		
	
