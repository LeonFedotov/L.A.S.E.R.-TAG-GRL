#include "drips.h"

//-----------------------------------	
drip::drip(){
	reset();
}

//-----------------------------------	
void drip::reset(){
	dripping	= false;
	direction 	= 0;
	dripLength  = 0;
	distance	= 0;
	dripWidth   = 0;
	
	r			= 255;
	g			= 255;
	b			= 255;
	
	vel.set(0,0);
	pos.set(0,0);
	dst.set(0,0);
	pre.set(0,0);
}

//-----------------------------------	
void drip::setColor(float red, float green, float blue){
	r = red;
	g = green;
	b = blue;
}

//-----------------------------------	
bool drip::setup(float x, float y, int imageW, int imageH, int _direction, float length, float speed, int dWidth){
				
	//set position
	pos.x = x;
	pos.y = y;
	
	//lets make sure it lies in the image
	if(pos.x < 0)pos.x = 0;
	else if(pos.x >= imageW)pos.x = imageW -1;
	if(pos.y < 0)pos.y = 0;
	else if(pos.y >= imageH)pos.y = imageH -1;
	
	//set pre to be the same as position
	pre = pos;
				
	//make sure we have only 4 directions
	direction = _direction;
	
	if(direction < 0) direction = 0;
	if(direction > 3) direction = 3;
	
	
	//figure out target and 
	//acceleration based on direction
	if(direction == 0){	//south
		dst.x = pos.x;
		dst.y = pos.y + length;
		vel.x = 0;
		vel.y = speed;
	}
	else
	if(direction == 1){	//west
		dst.x = pos.x - length;
		dst.y = pos.y;
		vel.x = -speed;
		vel.y = 0;			}
	else
	if(direction == 2){	//north
		dst.x = pos.x;
		dst.y = pos.y - length;
		vel.x = 0;
		vel.y = -speed;				
	}
	else
	if(direction == 3){	//east
		dst.x = pos.x + length;
		dst.y = pos.y;
		vel.x = speed;
		vel.y = 0;
	}
							
	//now lets check we haven't gone over our image border
	if(dst.x >= imageW){
		dst.x = imageW - 1;
	}
	else if(dst.x < 0){
		dst.x = 0;
	}
	
	if(dst.y >= imageH){
		dst.y = imageH - 1;
	}
	else if(dst.y < 0){
		dst.y = 0;
	}
	
	//recalculate the dripLength
	dripLength = floatAbs(dst.x - pos.x) + floatAbs(dst.y - pos.y);

	//lets not start dripping until specifically told to do so
	dripping	= false;
	
	//store our drip width
	dripWidth = dWidth;
	
	return true;
}

//-----------------------------------	
bool drip::update(){
				
	if(!isDripping())return false;
				
	//lets figure out how far we will have travelled
	distance += floatAbs(vel.x) + floatAbs(vel.y);
	
	//only update pos if doing so would keep us
	//under the dripLength - otherwise we could
	//go outside of the image dimensions
	
	//printf("dist is %f - length is %i \n", distance, dripLength);
	
	if(distance >= dripLength){
		stopDripping();
		return false;
	}else{
		
		//lets see if we have moved less than 1 pixel
		float preDist = floatAbs( (pre - pos).length() );
		
		//if(preDist > 1){
			//update our previous point
			pre = pos;
		//}
		
		//some deceleration 
		if( dripLength - distance < 24) {
			vel.x *= 0.987;
			vel.y *= 0.987;
			
			if( floatAbs(vel.x) < 0.01 && floatAbs(vel.y) < 0.01){
				stopDripping();
				return false;
			}
		}
		
		//update our current point
		pos.x += vel.x;
		pos.y += vel.y;
		
		//printf("pos is %f %f - vel is %f %f \n", pos.x, pos.y, vel.x, vel.y);
						
		//if we have moved less than a pixel
		//then no need to draw 
		//if(preDist < 1){
		//	return false;
		//}
		
	}

	return true;		
}

//-----------------------------------	
void drip::startDripping(){
	dripping = true;
}
		
//-----------------------------------	
void drip::stopDripping(){
	dripping = false;
}

//-----------------------------------	
bool drip::isDripping(){
	return dripping;
}


//------------------------------------
drips::drips(){
	currentDrip = 0;
	numDrips	= 0;
	width 		= 0;
	height 		= 0;
	totalPixels = 0;
	speed		= 0;
	red			= 255;
	green		= 255;
	blue		= 255;
	bSetup		= false;
}

//------------------------------------
void drips::setup(int w, int h){
	if(bSetup)return;

	width  = w;
	height = h;
	
	totalPixels = width * height * 3;
	pixels = new unsigned char[totalPixels];
	
	clear();		
	bSetup = true;
}


//-----------------------------------------------------
void drips::setColor(unsigned char r, unsigned char g, unsigned char b){
	red		= r;
	green	= g;
	blue 	= b;
}

//------------------------------------
void drips::clear(){
	
	for(int i = 0; i < numDrips; i++){
		DRIP[i].reset();
	}
	
	currentDrip = 0;
	numDrips	= 0;
	memset(pixels, 0, totalPixels);
}

//-------------------------------------
void drips::setDirection(int dripDirection){
	direction = dripDirection;
}

//-------------------------------------
int drips::getDirection(){
	return direction;
}

//-------------------------------------
void drips::setSpeed(float dripSpeed){
	speed = dripSpeed;
}	

//-------------------------------------
float drips::getSpeed(){
	return speed;
}

//-------------------------------------
void drips::setWidth(int dWidth){
	dripWidth = dWidth;
}

//-------------------------------------
int drips::getWidth(){
	return dripWidth;
}		

//------------------------------------
bool drips::addDrip(int x, int y){
	if(!bSetup){
		printf("addDrip: call setup first!!\n");	 
		return false;
	}
	
	if(numDrips >= MAX_DRIPS){
		//printf("addDrip: no more drips available till clear - max is %i\n", MAX_DRIPS);
		return false;
	}
	
	if(x > width || y > height || x < 0 || y < 0){
		//printf("addDrip: coordinates (%i, %i) outside image [%i x %i]! \n",x, y, width, height); 
		return false;
	}
				
	float length = 0;
				
	if(direction == 0){
		if(height - y > 0)length = rand()%(height - y);
	}
	else
	if(direction == 1){
		if(x > 0)length = rand()%x;
	}
	else 
	if(direction == 2){
		if(y > 0)length = rand()%y;
	}
	else
	if(direction == 3){
		if(width - x > 0)length = rand()%(width - x);
	}
	
	length *= 0.75;
	
	if((int)length == 0){
		//printf("addDrip: couldn't add drip: length is %i coord is (%i, %i), image %i by %i and direction %i\n", (int)length,  x, y, width, height,direction);
		return false;
	}			
				
	DRIP[numDrips].reset();
	DRIP[numDrips].setup(x, y, width, height, direction, (float)length, speed, dripWidth);
	DRIP[numDrips].setColor(red, green, blue);
	DRIP[numDrips].startDripping();
	
	numDrips++;
	
	return true;
}

//same as internal one but takes an array to draw into
//there is no checking of size etc so be careful!
//-----------------------------------
void drips::updateDrips(unsigned char * pix){
	if(!bSetup){
		printf("addDrip: call setup first!!\n");		 
		return;
	}

	for(int i = 0; i < numDrips; i++){
		if(DRIP[i].isDripping() && DRIP[i].update() ){
		
			//printf("drip %i - pos is (%f %f)\n", i, DRIP[i].pos.x, DRIP[i].pos.y);
		
			//then we can try and update our drip
			//into our pixels					
			if(DRIP[i].direction == 0 || DRIP[i].direction == 2){
				fastVerticalLine(pix, DRIP[i].pre.x, DRIP[i].pre.y, DRIP[i].pos.y,  1,  DRIP[i].r, DRIP[i].g, DRIP[i].b);    
			}else{
				fastHorizontalLine(pix, DRIP[i].pre.y, DRIP[i].pre.x, DRIP[i].pos.x,  1,  DRIP[i].r, DRIP[i].g, DRIP[i].b);    
			}
		}
	}
}

//-----------------------------------
void drips::updateDrips(){
	updateDrips(pixels);
}

//----------------------------
unsigned char * drips::getPixels(){
	return pixels;
}

//------------------------------------
void drips::fastVerticalLine(unsigned char * pix, int xPos, int y0, int y1, int lineWidth,  unsigned char r, unsigned char g, unsigned char b){
	if(xPos < 0 || xPos >= width)return;
	
	int x0 = xPos - lineWidth;
	int x1 = xPos + lineWidth;
	
	if(x0 < 0) x0 = 0;
	if(x1 >= width) x1 = width-1;

	int y = y0;
	
	int incr = 1;
	if(y1 < y0)incr = -1;
	
	int pos = 0;
	int x = x0;
	int stride;
	
	while(y != y1){
		stride = y*width;
		for(x = x0; x <x1; x++){

			pos = 3*(stride+x);
			
			pix[    pos] = r;
			pix[1 + pos] = g;
			pix[2 + pos] = b;
		}
		y+=incr;
	}
}

//------------------------------------
void drips::fastHorizontalLine(unsigned char * pix, int yPos, int x0, int x1, int lineWidth,  unsigned char r, unsigned char g, unsigned char b){
	if(yPos < 0 || yPos >= height)return;
	
	int y0 = yPos - lineWidth;
	int y1 = yPos + lineWidth;
	
	if(y0 < 0) y0 = 0;
	if(y1 >= height) y1 = height-1;

	int x = x0;
	
	int incr = 1;
	if(x1 < x0)incr = -1;
	
	int pos = 0;
	int y = y0;
	int stride;
	
	while(x != x1){
		for(y = y0; y <y1; y++){
			stride = y*width;
			pos = 3*(stride+x);
			pix[    pos] = r;
			pix[1 + pos] = g;
			pix[2 + pos] = b;
		}
		x+=incr;
	}
}





































