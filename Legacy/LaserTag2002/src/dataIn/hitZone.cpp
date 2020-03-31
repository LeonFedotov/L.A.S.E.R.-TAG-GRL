#include "hitZone.h"

//---------------------------
hitZone::hitZone(){
	bActive = true;			
}

//---------------------------
void hitZone::setActive(bool active){
	bActive = active;			
}

//---------------------------
bool hitZone::getActive(){
	return bActive;			
}

//---------------------------
bool hitZone::isHit(float x, float y){
	if(x  >= points[0].x && x <= points[1].x){
		if(y >= points[0].y && y <= points[2].y){
			return true;
		}
	}
	return false;
}

//---------------------------
void hitZone::setPosition(float x, float y){
	
	if(x < 0.0) x = 0.0;
	else if(x > 1.0) x = 1.0;
	
	if(y < 0.0) y = 0.0;
	else if(y > 1.0) y = 1.0;
	
	zoneDimensions[0].x = x;
	zoneDimensions[0].y = y;
	
	updatePoints();			
}


//---------------------------
void hitZone::setDimensions(float w, float h){
	
	if(w < 0.0) w = 0.0;
	else if(w > 1.0) w = 1.0;
	
	if(h < 0.0) h = 0.0;
	else if(h > 1.0) h = 1.0;
	
	zoneDimensions[1].x = w;
	zoneDimensions[1].y = h;
	
	updatePoints();
}

//---------------------------
void hitZone::updatePoints(){
	
	//top left corner
	points[0] = zoneDimensions[0];
	
	//top right corner
	points[1].x = zoneDimensions[0].x + zoneDimensions[1].x;
	points[1].y = zoneDimensions[0].y;
	
	//bottom right corner
	points[2].x = zoneDimensions[0].x + zoneDimensions[1].x;
	points[2].y = zoneDimensions[0].y + zoneDimensions[1].y;
	
	//bottom left corner
	points[3].x = zoneDimensions[0].x;
	points[3].y = zoneDimensions[0].y + zoneDimensions[1].y;			
	
	//lets check that the points don't lie outside our range 0-1			
	for(int i = 0; i < 4; i++){
		if(points[i].x > 1.0) points[i].x = 1.0;
		if(points[i].x < 0.0) points[i].x = 0.0;
		if(points[i].y > 1.0) points[i].y = 1.0;
		if(points[i].y < 0.0) points[i].y = 0.0;
	}			
							
}

