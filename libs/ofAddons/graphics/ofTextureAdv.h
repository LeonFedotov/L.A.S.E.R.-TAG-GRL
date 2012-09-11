#ifndef _IMAGE_TEXTURE_ADV_H_
#define _IMAGE_TEXTURE_ADV_H_

#include "ofMain.h"
#include "ofGraphics.h"
#include "ofVec3f.h"

#define GRID_X 12
#define GRID_Y 8

class ofTextureAdv{

	public :
		ofTextureAdv();
		~ofTextureAdv();
		void allocate(int w, int h, int internalGlDataType);	 
		void clear();
		void loadData(unsigned char * data, int w, int h, int glDataType); 
		void setPoint(int which, float x, float y);
		void setPoints(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
		void draw();
		
		
		int width, height;		 
				
	protected:		
		void updatePoints();
		
		float 			offset_x, offset_y;
		float 			tex_t, tex_u;   
		int 			tex_w, tex_h;
		unsigned int	texName[1];	
		
		ofVec3f quad[4];
		
		ofVec3f grid[GRID_X * GRID_Y];
		ofVec3f coor[GRID_X * GRID_Y];
		
		
}; 

#endif
