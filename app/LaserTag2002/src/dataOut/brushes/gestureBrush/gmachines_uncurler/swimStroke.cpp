#include "swimStroke.h"



//--------------------------------------------------------------
swimStroke::swimStroke(){
	bSwim 				= false;
	originalAngles = new float[maxNumPts];
}

void swimStroke::clear(){
	nPts = 0;
	bSwim 				= false;
	
}

//--------------------------------------------------------------
void swimStroke::start(){

	if(nPts == 0)return;

	float pct = 1 - MIN(MAX((lengthSection / 10.0f),0),0.5);
	lengthSectionTarget 	= pct * lengthSection;
	bSwim 					= true;
	bFirstFrame 			= true;
	overallTargetLength 	= lengthSectionTarget * nPts;
	
	
	for (int i = 0; i < nPts; i++){
		originalAngles[i] = angles[i];
	}
	originalLength = lengthSection;
	bUncurling = true;
	
	
	
}

//--------------------------------------------------------------
void swimStroke::update(){
	
	if(nPts == 0)return;
	
	//------------------------------------------------ swimming:
	if (bSwim == true){
		
		
		centroid.set(0,0,0);

		lengthSection = 0.98 * lengthSection + 
						0.02f * ((bUncurling == true) ? lengthSectionTarget : originalLength);
						
		ofVec3f pt = startPosition;
		
		ofVec3f ptPrev = pt;
		glBegin(GL_LINE_STRIP);
		glVertex2f(pt.x, pt.y);
		float angle = 0;
		for (int i = 0; i < nPts; i++){
			ptPrev = pt;
			if (i == 0) angle = angles[i];
			else (angle += angles[i]);
			pt.x += lengthSection * cos(angle);
			pt.y += lengthSection * sin(angle);
			
			centroid += pt;
			
			if (i != 0){
			if (bUncurling) angles[i] *= 0.995f;
			else  angles[i] = 0.95f * angles[i] + 
								  0.05f * originalAngles[i];
			}
		}
		glEnd();
		
		//----------------------------------------
		float dist = (startPosition - pt).length();
		float pct =  1 - (dist / overallTargetLength);
		//samp.setSpeed(1.0f + pct * 2.5);
		//----------------------------------------
		
		centroid /=  nPts;
		
		float backAngle = returnAngle(ptPrev, pt);
		angle = 0;
		for (int i = nPts-1; i >= 0; i--){
			ptPrev = pt;
			if (i == nPts-1) angle = backAngle;
			else (angle -= angles[i+1]);
			pt.x -= lengthSection * cos(angle);
			pt.y -= lengthSection * sin(angle);
			if (i != 0){
				if (bUncurling) angles[i] *= 0.95f;
				/*else  angles[i] = 0.995f * angles[i] + 
								  0.005f * originalAngles[i];*/
			}
		}
		glEnd();
		
		float diff = returnAngle(pt,ptPrev) - angles[0];
		while (diff > PI){ diff -= TWO_PI; };
		while (diff < -PI){ diff += TWO_PI;};
		diff /= 2.0f;
		angles[0] += diff;
		while (angles[0] > PI){ angles[0] -= TWO_PI;};
		while (angles[0] < -PI){ angles[0] += TWO_PI;};
		
		startPosition += (pt-startPosition)/2.0f;
	
	
	
	if (bFirstFrame == true){
		prevCentroid = centroid;
		bFirstFrame = false;
		//samp.setVolume(0);
	} else {
		ofVec3f diff 	= centroid - prevCentroid;
		float len = diff.length();
		//printf("len %f \n", len);
		startPosition 	-= diff * 0.05f;
		prevCentroid  	= centroid;
	}
	
	}  else {
		//samp.setVolume(0.0f);
	}
	
	
	if (bUncurling == true){
		float diffInAngleOverall = 0;
		for (int i = 1; i < nPts; i++){
			diffInAngleOverall += (fabs)(angles[i]);
		}
		//printf("%f \n", diffInAngleOverall);
		
		if (bSwim == true){
			//volume = 0.8f * volume + 0.2f * MIN((diffInAngleOverall/100.0f), 0.4f);
			//samp.setVolume(volume);
		} 
		//else samp.setVolume(0);
		
		if (fabs(diffInAngleOverall) < 0.1){
			bUncurling = false;
		}
	}
	
	
	if (bUncurling == false){
		float diffInAngleOverall = 0;
		for (int i = 1; i < nPts; i++){
			diffInAngleOverall += fabs(angles[i] - originalAngles[i]);
		}
		//printf("%f \n", );
		//if (bSwim == true) samp.setVolume(MIN((diffInAngleOverall/100.0f), 0.3f));
		//else samp.setVolume(0);
		
		if (fabs(diffInAngleOverall) < 0.1){
			bUncurling = true;
		}
	}
	
}

//--------------------------------------------------------------
void swimStroke::draw(){
	if(nPts == 0)return;
	
	//ofSetColor(0xffffff);
	ofVec3f pt = startPosition;
	
	renderStroke(angles, widths, nPts, startPosition, lengthSection);

	/*if (bSwim == true){
		ofNoFill();
		ofRect(boundingBoxBottom.x , boundingBoxBottom.y, 
			   boundingBoxTop.x - boundingBoxBottom.x,
			   boundingBoxTop.y - boundingBoxBottom.y);
	}*/

	
}

