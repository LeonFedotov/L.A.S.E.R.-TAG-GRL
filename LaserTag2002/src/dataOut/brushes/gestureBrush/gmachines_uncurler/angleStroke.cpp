#include "angleStroke.h"



//--------------------------------------------------------------
angleStroke::angleStroke(){
	
	nPts				= 0;
	currentAngle 		= 0;
	maxNumPts			= 500;
	angles				= new float[maxNumPts];
	widths				= new float[maxNumPts];
	lengthSection 		= 5.0f;
		
}

//--------------------------------------------------------------
void angleStroke::update(){

	
	
}

//--------------------------------------------------------------
void angleStroke::start (ofVec3f pt){
	startPosition.set(pt);
	currentPosition.set(pt);	
	currentAngle 		= 0;
	nPts				= 0;
}
		

//--------------------------------------------------------------
void angleStroke::addPoint(ofVec3f pt){

	if (nPts < maxNumPts){
		float distance = (pt - currentPosition).length();	
		if (distance > lengthSection){
			int nPointsToAdd = (distance / lengthSection);
			for (int i = 0; i < nPointsToAdd; i++){
				if (nPts == 0){
					angles[nPts] = returnAngle(currentPosition,pt);
					currentAngle = angles[0];
				} else {	
					float diff = (returnAngle(currentPosition,pt)) - currentAngle;
					while (diff < -PI) 	diff += TWO_PI;
					while (diff > PI) 	diff -= TWO_PI;
					angles[nPts] = diff;
					currentAngle += angles[nPts];
				}			
				currentPosition.x += lengthSection * cos(currentAngle);
				currentPosition.y += lengthSection * sin(currentAngle);
				nPts++;
			} 
		}
	}
}

//--------------------------------------------------------------
void angleStroke::fitToVec3fs(ofVec3f * pts, int nInputPts, int nTargetAngles){
	
	currentAngle 		= 0;
	nPts				= 0;
	float length 		= 0;
	
	for (int i = 0; i < nInputPts-1; i++){
		length += (pts[i] - pts[i + 1]).length();
	}
	lengthSection = length / (float)nTargetAngles;
	
	startPosition.set(pts[0]);
	currentPosition.set(pts[0]);	
	
	ofVec3f 	pt;
	float 		pct;
	
	//--------------------------------------
	for (int i = 0; i < nTargetAngles; i++){
		pct 	= (float)(i+1) / (float)nTargetAngles;
		pt 		= findPtForPct(pct, pts, nInputPts, length);
		
		widths[i] 		= pt.z;
		
		if (i == 0){
			angles[i] 		= returnAngle(currentPosition,pt);	
			currentAngle 	= angles[i];
		} else {
			float diff = (returnAngle(currentPosition,pt)) - currentAngle;
			while (diff < -PI) 	diff += TWO_PI;
			while (diff > PI) 	diff -= TWO_PI;
			angles[i] 		= diff;
			currentAngle 	+= angles[i];
		}
		currentPosition.x += lengthSection * cos(currentAngle);
		currentPosition.y += lengthSection * sin(currentAngle);
	}
	//--------------------------------------
	
	nPts 		= nTargetAngles;

	bFitted 	= true;
}


//--------------------------------------------------------------
ofVec3f inline angleStroke::findPtForPct(float pct, ofVec3f * pts, int nInputPts, float totalLen){
	
	float targetLength = totalLen * pct;
	ofVec3f start;
	ofVec3f end;
	float startLength = 0;
	float endLength = 0;
	
	float length = 0;
	for (int i = 0; i < nInputPts-1; i++){
		float thisSegLen = (pts[i] - pts[i + 1]).length();
		if (targetLength > length && targetLength <= (length + thisSegLen)){
			start.set(pts[i]);
			end.set(pts[i+1]);
			float pct = (targetLength - length) / thisSegLen;
			return (start + pct * (end - start));
		}
		length += thisSegLen;
	}
	
	return pts[nInputPts-1];
}


