#include "guiQuad.h"

//----------------------------------------------------
guiQuad::guiQuad(){
	selected = -1;
	quadName = "QUAD_";
}

//----------------------------------------------------
void guiQuad::setup(string _quadName){
	quadName = _quadName;
}

//----------------------------------------------------
void guiQuad::releaseAllPoints(){
	selected = -1;
}

//----------------------------------------------------
//these should be in the range x(0-maxX) y(0-maxH) 
//with 	width  = maxW
//		height = maxH 
void guiQuad::setQuadPoints( ofPoint * inPts ){
		
	for(int i = 0; i < 4; i++){
		srcZeroToOne[i].x	= inPts[i].x;
		srcZeroToOne[i].y	= inPts[i].y;
		
		if(srcZeroToOne[i].x > 1) srcZeroToOne[i].x = 1;
		if(srcZeroToOne[i].y > 1) srcZeroToOne[i].y = 1;
	}
}

//----------------------------------------------------
void guiQuad::readFromFile(string filePath){
	
	xml.loadFile(filePath);
	loadSettings();	
}

//----------------------------------------------------
void guiQuad::loadSettings(){
	
	string str;
	
	
	for(int i = 0; i < 4; i++){
		
		//These two lines give us:
		// "QUAD_SRC_0"
		// "QUAD_SRC_1" etc...
								
		str = quadName;
		str += ofToString(i);									
																						
		srcZeroToOne[i].x = xml.getValue(str+"_X", srcZeroToOne[i].x);
		srcZeroToOne[i].y = xml.getValue(str+"_Y", srcZeroToOne[i].y);		
		
		if(srcZeroToOne[i].x > 1) srcZeroToOne[i].x = 1;
		if(srcZeroToOne[i].y > 1) srcZeroToOne[i].y = 1;
									
	}	
				
}

//----------------------------------------------------
bool guiQuad::selectPoint(float x, float y, float offsetX, float offsetY, float width, float height, float hitArea){

	//make sure selected is -1 unless we really find a point			
	selected = -1;

	if(width == 0 || height == 0 || x < offsetX || x > offsetX + width || y < offsetY ||  y > offsetY + height){
		//then we are out of our possible quad area
		//so we ignore :)
		return false;
	}  
	
	//lets get it in range x(0 - width) y(0 - height)
	float px = x - offsetX;
	float py = y - offsetY;
	
	//now get in 0-1 range
	px /= width;
	py /= height;
	
	hitArea /= width;
	
	//we want to store the smallest distance found
	//because in the case when two points are in the 
	//hit area we want to pick the closet
	float storeDist = 9999999.0;
	
	for(int i = 0; i < 4; i++){
		float dx = abs(px -  srcZeroToOne[i].x);
		float dy = abs(py -  srcZeroToOne[i].y);
		
		float dist = sqrt(dx*dx + dy*dy);
		
		if(dist > hitArea)continue;
		
		if(dist < storeDist){
			selected = i;
			storeDist = dist;
		}
	}

	if(selected != -1){
		srcZeroToOne[selected].x 	= px;
		srcZeroToOne[selected].y 	= py;	
		srcScaled[selected].x		= px;
		srcScaled[selected].y		= py;					
	
		if(selected == 0)setCommonText("status: Quad - Top Left");
		else
		if(selected == 1)setCommonText("status: Quad - Top Right");
		else
		if(selected == 2)setCommonText("status: Quad - Bottom Right");
		else 
		if(selected == 3)setCommonText("status: Quad - Bottom Left");
		
		return true;
	}
	
	return false;
}

//----------------------------------------------------
bool guiQuad::updatePoint(float x, float y, float offsetX, float offsetY, float width, float height){
	
	//nothing to update
	if(selected == -1) return false;
	
	if(width == 0 || height == 0){
		//dangerous so we ignore :)
		return false;
	}  
	
	if( x < offsetX ) 			x = offsetX;
	if( x > offsetX + width ) 	x = offsetX + width;
	if( y < offsetY ) 			y = offsetY;
	if( y > offsetY + height) 	y = offsetY + height;
	
	//lets get it in range x(0 - width) y(0 - height)
	float px = x - offsetX;
	float py = y - offsetY;
				
	//now get in 0-1 range
	px /= width;
	py /= height;
	
	srcZeroToOne[selected].x 	= px;
	srcZeroToOne[selected].y 	= py;	
	srcScaled[selected].x		= px;
	srcScaled[selected].y		= py;
	
	return true;
}

//----------------------------------------------------
//returns pts to width by height range 
ofPoint * guiQuad::getScaledQuadPoints(float width, float height){

	for(int i = 0; i < 4; i++){
		srcScaled[i].x = srcZeroToOne[i].x * width;
		srcScaled[i].y = srcZeroToOne[i].y * height;				
	}
	
	return srcScaled;
}


//----------------------------------------------------
//returns pts in 0-1 scale
ofPoint * guiQuad::getQuadPoints(){
	return srcZeroToOne;
}


//----------------------------------------------------
void guiQuad::saveToFile(string filePath, string newQuadName){
	string str;
		
	for(int i = 0; i < 4; i++){
	
		str = newQuadName;
		str += ofToString(i);

		xml.setValue(str+"_X", srcZeroToOne[i].x);
		xml.setValue(str+"_Y", srcZeroToOne[i].y);		
	}
	
	xml.saveFile(filePath);
}		

ofMatrix4x4 guiQuad::getHomography(vector<cv::Point2f> cvSrcPos, vector<cv::Point2f> cvDstPos) {
	ofMatrix4x4 homographyMat;
	cv::Mat homography = cv::findHomography(cvSrcPos, cvDstPos);
	homographyMat.getPtr()[0] = homography.at<double>(0, 0);
	homographyMat.getPtr()[4] = homography.at<double>(0, 1);
	homographyMat.getPtr()[12] = homography.at<double>(0, 2);

	homographyMat.getPtr()[1] = homography.at<double>(1, 0);
	homographyMat.getPtr()[5] = homography.at<double>(1, 1);
	homographyMat.getPtr()[13] = homography.at<double>(1, 2);

	homographyMat.getPtr()[3] = homography.at<double>(2, 0);
	homographyMat.getPtr()[7] = homography.at<double>(2, 1);
	homographyMat.getPtr()[15] = homography.at<double>(2, 2);
	return homographyMat;
}

ofMatrix4x4 guiQuad::getHomography(ofPoint src[4], ofPoint dst[4]) {
	vector<cv::Point2f> cvSrcPos, cvDstPos;
	
		cvSrcPos.push_back(cv::Point2f(src[0].x, src[0].y));
		cvSrcPos.push_back(cv::Point2f(src[1].x, src[1].y));
		cvSrcPos.push_back(cv::Point2f(src[2].x, src[2].y));
		cvSrcPos.push_back(cv::Point2f(src[3].x, src[3].y));

		cvDstPos.push_back(cv::Point2f(dst[0].x, dst[0].y));
		cvDstPos.push_back(cv::Point2f(dst[1].x, dst[1].y));
		cvDstPos.push_back(cv::Point2f(dst[2].x, dst[2].y));
		cvDstPos.push_back(cv::Point2f(dst[3].x, dst[3].y));
		return getHomography(cvSrcPos, cvDstPos);
}

//----------------------------------------------------
void guiQuad::saveToFile(string filePath){
	saveToFile(filePath, quadName);
}

//----------------------------------------------------
void guiQuad::draw(float x, float y, float width, float height, int red, int green, int blue, int thickness){
	
	getScaledQuadPoints(width, height);
	ofPushMatrix();
		ofTranslate(x, y, 0);
		
		ofNoFill();
		
		ofSetColor(red, green, blue);
		ofSetLineWidth(thickness);
		glBegin(GL_LINE_LOOP);
		for(int i = 0; i < 4; i++){
			glVertex2f(srcScaled[i].x, srcScaled[i].y);
		}
		glEnd();
		
		ofSetLineWidth(1);
		ofSetRectMode(OF_RECTMODE_CENTER);
		for(int i = 0; i < 4; i++){
			
			if(i == 0)ofSetColor(255, 0, 0);
			if(i == 1)ofSetColor(0, 255, 0);
			if(i == 2)ofSetColor(0, 0, 255);
			if(i == 3)ofSetColor(0, 255, 255);

			ofDrawRectangle(srcScaled[i].x, srcScaled[i].y, 10, 10);
		}
		ofSetRectMode(OF_RECTMODE_CORNER);
		ofFill();
	ofPopMatrix();
}

//----------------------------------------------------
void guiQuad::draw(float x, float y, float width, float height){
	
	//default to a think yellow line
	draw(x, y, width, height, 255, 255, 0, 1);
}

