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
	numSteps		= 40;
	brushWidth		= 18;
	
	//default to white
	red				= 255;
	green			= 255;
	blue			= 255;	
	
	//our default drip settings
	dripsSettings(false, 8, 0.05, 0);

	//setup up our image buffer
	pixels = new unsigned char[width * height * imageNumBytes];
	clear();
	
	DRIPS.setup(width, height);
	
	//load brushes
	loadbrushes(ofToDataPath("brushes/"));
}


//------------------------
void pngBrush::clear(){
	memset(pixels, 0, width*height*imageNumBytes);
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
				
	IMG.loadImage(DL.getPath(brushNumber));
	
	 //no need for color
	 //we will use the image as a mask to 
	 //draw with and multiply it with
	 //our drawing color.
	IMG.setImageType(OF_IMAGE_GRAYSCALE); 
	TMP.setImageType(OF_IMAGE_GRAYSCALE); 
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
	
	TMP.clone(IMG);
	
	float tmpW 		= TMP.width;
	float tmpH 		= TMP.height;
	
	float ratio 	= (float)_brushWidth/tmpW;
		
	int newWidth  	= _brushWidth;
	int newHeight  	= tmpH*ratio;
	
	TMP.resize(newWidth, newHeight);
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
	ofSetColor(0xFFFFFF);
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
				
	//get the pixels of the brush
	unsigned char * brushPix = TMP.getPixels();
				
	//if no distance is to be travelled
	//draw only one point
	if(dx == 0 && dy == 0) steps = 1;
	
	// lets draw the brushNumber many times to make a line
	for(int i = 0; i < steps; i++){				
							
		// we do - TMP.width/2 because we want the brushNumber to be
		// drawn from the center
		if(newStroke){
			tx = startX - TMP.width/2;
			ty = startY - TMP.height/2;
		}else{
			tx = (oldX + (int)(dx*(float)i)) - TMP.width/2;
			ty = (oldY + (int)(dy*(float)i)) - TMP.height/2;
		}
						
		//this is what we use to move through the
		//brushNumber array									
		int tPix = 0;
		
		int destX = (TMP.width  + tx);
		int destY = (TMP.height + ty);	
						
		//lets check that we don't draw out outside the projected
		//image
		if(destX >= width)   destX = width  -1;
		if(destY >= height)  destY = height -1;
		
		//if the brushNumber is a bit off the screen on the left side
		//we need to figure this amount out so we only copy part
		//of the brushNumber image 
		int offSetCorrectionLeft = 0;	
		if(tx < 0){
			offSetCorrectionLeft = -tx;
			tx = 0;
		}
		
		//same here for y - we need to figure out the y offset
		//for the cropped brush
		if(ty < 0){
			tPix    = -ty * TMP.width; 
			ty 		= 0;
		}
						
		//this is for the right hand side cropped brush 
		int offSetCorrectionRight = ((TMP.width + tx) -  destX);	
		tPix += offSetCorrectionLeft;

		//some vars we are going to need
		//put here to optimise? 
		float r, g, b, value, ival;

		for(int y = ty; y < destY; y++){
			for(int x = tx; x < destX; x++){
					
				pix = x*imageNumBytes + (y*stride);
				
				if(brushPix[tPix] == 0){
					//we don't need to do anything
					tPix++;
					continue;
				}						
				//okay so here we are going to use the pixel value of the brush
				//as a multiplier to add into the image
				value = (float)brushPix[tPix] * ONE_OVER_255;
				ival  = 1.0 - value;
				
				r 	= (float)pixels[pix  ] * ival + red   * value;
				g 	= (float)pixels[pix+1] * ival + green * value;
				b 	= (float)pixels[pix+2] * ival + blue  * value;
				
				pixels[pix  ] = (unsigned char)r;
				pixels[pix+1] = (unsigned char)g;
				pixels[pix+2] = (unsigned char)b;
																												
				tPix++;					
			}
			tPix += offSetCorrectionRight;
		}
	
	}
	
	oldX = startX;
	oldY = startY;
}
	
//-----------------------------------------------------
unsigned char * pngBrush::getImageAsPixels(){
	return pixels;
}


//-----------------------------------------------------
void pngBrush::drawBrushColor(float x, float y, int w, int h){
	ofSetColor(red, green, blue);
	ofRect(x,y,w,h);
}
		
	