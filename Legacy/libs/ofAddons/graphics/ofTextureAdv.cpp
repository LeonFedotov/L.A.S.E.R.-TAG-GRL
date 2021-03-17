#include "ofTextureAdv.h"

//----------------------------------------------------------
ofTextureAdv::ofTextureAdv(){
	
	texName[0] = 0;
}


//----------------------------------------------------------
ofTextureAdv::~ofTextureAdv(){
	
}

//----------------------------------------------------------
void ofTextureAdv::clear(){
	// try to free up the texture memory so we don't reallocate 
	// http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/deletetextures.html
	if (texName[0] != 0){
		glDeleteTextures(1, (GLuint *)texName);	
	}
	
	width = 0;
	height = 0;
}

//----------------------------------------------------------
void ofTextureAdv::allocate(int w, int h, int internalGlDataType){

	// 	can pass in anything (320x240) (10x5)
	// 	here we make it power of 2 for opengl (512x256), (16x8)

	tex_w = ofNextPow2(w);			
	tex_h = ofNextPow2(h);
	
	tex_t = (float)w/tex_w;
	tex_u = (float)h/tex_h;

	// attempt to free the previous bound texture, if we can:
	clear();
	
	glGenTextures(1, (GLuint *)texName);   // could be more then one, but for now, just one

	glEnable(GL_TEXTURE_2D);
	
		glBindTexture(GL_TEXTURE_2D, (GLuint)texName[0]);
		// upload 0 or white pixels....
		glTexImage2D(GL_TEXTURE_2D, 0, internalGlDataType, tex_w, tex_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);  // init to black...
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glDisable(GL_TEXTURE_2D);
	
	width = w;
	height = h;
}

//----------------------------------------------------------
void ofTextureAdv::loadData(unsigned char * data, int w, int h, int glDataType){

	//	can we allow for uploads bigger then texture and 
	//	just take as much as the texture can?
	//
	//	ie:
	//	int uploadW = MIN(w, tex_w); 
	//	int uploadH = MIN(h, tex_h); 
	//  but with a "step" size of w?
	// 	check "glTexSubImage2D"
	
	if ( w > tex_w || h > tex_h) {
		printf("image data too big for allocated texture. not uploading... \n");
		return;
	}
	
	width 	= w;
	height 	= h;
	
	//compute new tex co-ords based on the ratio of data's w, h to texture w,h;
	tex_t = (float)(w) / (float)tex_w;
	tex_u = (float)(h) / (float)tex_h;

	// 	ok this is an ultra annoying bug :
	// 	opengl texels and linear filtering - 
	// 	when we have a sub-image, and we scale it
	// 	we can clamp the border pixels to the border, 
	//  but the borders of the sub image get mixed with
	//  neighboring pixels...
	//  grr...
	//  
	//  the best solution would be to pad out the image
	// 	being uploaded with 2 pixels on all sides, and 
	//  recompute tex_t coordinates..
	//  another option is a gl_arb non pow 2 textures...
	//  the current hack is to alter the tex_t, tex_u calcs, but
	//  that makes the image slightly off... 
	//  this is currently being done in draw...
	//
	// 	we need a good solution for this..
	//   
	//  http://www.opengl.org/discussion_boards/ubb/ultimatebb.php?ubb=get_topic;f=3;t=014770#000001
	//  http://www.opengl.org/discussion_boards/ubb/ultimatebb.php?ubb=get_topic;f=3;t=014770#000001
	
	//------------------------ likely, we are uploading continuous data
	GLint prevAlignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevAlignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	// update the texture image:
	glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (GLuint)texName[0]);
 		glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w,h,glDataType,GL_UNSIGNED_BYTE,data);	
	glDisable(GL_TEXTURE_2D);

	//------------------------ back to normal.
	glPixelStorei(GL_UNPACK_ALIGNMENT, prevAlignment);
	
}



void ofTextureAdv::setPoint(int which, float x, float y){
	
	if( which>4 || which < 0){
		printf("ofTextureAdv:: please choose point 0-3\n");
		return;
	}
	
	quad[which].set(x, y, 0);
	
	updatePoints();
}

void ofTextureAdv::setPoints(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4){
	
	quad[3].set(x4, y4, 0);
	quad[2].set(x3, y3, 0);
	quad[0].set(x1, y1, 0);
	quad[1].set(x2, y2, 0);
		
	
	updatePoints();
}

void ofTextureAdv::updatePoints(){

	int gridSizeX = GRID_X;
	int gridSizeY = GRID_Y;
	
	float xRes = 1.0/(gridSizeX-1);
	float yRes = 1.0/(gridSizeY-1);

	GLfloat offsetw = 1.0f/(tex_w);
	GLfloat offseth = 1.0f/(tex_h);
	
	//this is because we want pct to go from offsetw to tex_t - offsetw 
	GLfloat texTAdj = tex_t - offsetw*2;	
	GLfloat texUAdj = tex_u - offseth*2;
		
	for(int y = 0; y < gridSizeY; y++){	
		for(int x = 0; x < gridSizeX; x++){
			
			float pctx = (float)x * xRes;
			float pcty = (float)y * yRes;
			
			float linePt0x = (1-pcty)*quad[0].x + pcty * quad[3].x;
			float linePt0y = (1-pcty)*quad[0].y + pcty * quad[3].y;
			
			float linePt1x = (1-pcty)*quad[1].x + pcty * quad[2].x;
			float linePt1y = (1-pcty)*quad[1].y + pcty * quad[2].y;			
			
			float ptx 	   = (1-pctx) * linePt0x + pctx * linePt1x;
			float pty 	   = (1-pctx) * linePt0y + pctx * linePt1y;
		
			float tt 	   =  offsetw + pctx * texTAdj;
			float uu	   =  offseth + (1.0-pcty) * texUAdj;
								
			grid[y*gridSizeX + x].set(ptx, pty, 0);
			coor[y*gridSizeX + x].set(tt, (tex_u - offseth) - uu, 0);
		}
	}
	
}

void ofTextureAdv::draw(){
		
	int gridSizeX = GRID_X;
	int gridSizeY = GRID_Y;
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, (GLuint)texName[0] );
		
	for(int y = 0; y < gridSizeY-1; y++){	
		for(int x = 0; x < gridSizeX-1; x++){

			glBegin(GL_QUADS);		

			int pt0 = x + y*gridSizeX;
			int pt1 = (x+1) + y*gridSizeX;
			int pt2 = (x+1) + (y+1)*gridSizeX;
			int pt3 = x + (y+1)*gridSizeX;
			
			glTexCoord2f(coor[pt0].x, coor[pt0].y );			
			glVertex2f(  grid[pt0].x, grid[pt0].y);
			
			glTexCoord2f(coor[pt1].x, coor[pt1].y );
			glVertex2f(  grid[pt1].x, grid[pt1].y);
	
			glTexCoord2f(coor[pt2].x, coor[pt2].y );
			glVertex2f(  grid[pt2].x, grid[pt2].y);

			glTexCoord2f(coor[pt3].x, coor[pt3].y );
			glVertex2f(  grid[pt3].x, grid[pt3].y);	
					
			glEnd();
		
		}
	}	
	
	glDisable(GL_TEXTURE_2D);
}
