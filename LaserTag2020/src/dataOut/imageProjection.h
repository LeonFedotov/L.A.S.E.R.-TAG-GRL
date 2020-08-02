#ifndef _IMG_PROJECTION_H
#define _IMG_PROJECTION_H

#include "ofMain.h"
#include "guiQuad.h"

//this class deals with the projection of lasertag
//it manages textures for the different modes and the distortion
//of the projected texture. 

class imageProjection : public baseGui{

	public:
	
		imageProjection();
		void setup(int _width, int _height);
		
		void setProjectionBrightness(float pBrightness);
		void setProjectionColor(int r, int g, int b);
		
		void updateGreyscaleTexture(unsigned char * pixels);
		void updateColorTexture(unsigned char * pixels);

		void loadSettings(string filePath);
		
		void setToolDimensions(float desiredW, float desiredH);	
		void applyQuad();
		
		ofPoint *  getQuadPoints();
		ofPoint *  getQuadPointsScaled();
		
		int selectQuad(int x, int y, int xCheck, int yCheck, int w, int h, int hitArea);
		bool updateQuad(int x, int y, int xCheck, int yCheck,  int w, int h);	
		int selectMiniQuad(int x, int y, int hitArea);
		bool updateMiniQuad(int x, int y);
		void releaseAllQuads();
		ofMatrix4x4 getMatrix(const ofPoint* srcPoints, const ofPoint* dstPoints);
		
		void drawMiniProjectionTool(float x, float y, bool showOutline, bool showTexture);
		void drawProjectionToolHandles(float x, float y, float w, float h, bool showOutline, bool highlyVisible);		
		void drawProjectionTex(float x, float y, float w, float h);
		void drawPreviewTex(float x, float y, float w, float h);
		void drawProjectionMask(float x, float y, float w, float h);
		

		ofMatrix4x4 matrix;
		//our textures and preview textures
		ofTexture   colorTexture;
		ofTexture 	colorPreviewTexture;
		ofTexture   greyscaleTexture;
		ofTexture 	greyscalePreviewTexture;
		
		guiQuad		 QUAD;
		ofPoint 	 warpSrc[4];
		
		bool bGreyscaleTexture;

		int toolX, toolY, red, green, blue;
		int width, height, scaledWidth, scaledHeight, whichToolSelected;

		float scaleX, scaleY;
		float brightness;

};

#endif
