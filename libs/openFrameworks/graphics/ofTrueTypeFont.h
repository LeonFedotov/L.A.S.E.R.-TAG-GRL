#ifndef _OF_TTF_FONT_H_
#define _OF_TTF_FONT_H_


#include "ofConstants.h"
#include "ofGraphics.h"
#include "ofUtils.h"

//--------------------------------------------------
typedef struct {
	int value;
	int height;
	int width;
	int setWidth;
	int topExtent;
	int leftExtent;
	float tTex;
	float vTex;		//0-1 pct of bitmap...
	float xOff;
	float yOff;
} charProps;
//--------------------------------------------------
#define NUM_CHARACTER_TO_START		33		// 0 - 32 are control characters, no graphics needed.




class ofTrueTypeFont{

public:


	ofTrueTypeFont();
	~ofTrueTypeFont();
	
	// 			-- default, non-full char set, anti aliased:
	void 		loadFont(string filename, int fontsize);
	void 		loadFont(string filename, int fontsize, bool _bAntiAliased, bool _bFullCharacterSet);

	bool		bLoadedOk;
	bool 		bAntiAlised;
	bool 		bFullCharacterSet;

  	float 		getLineHeight();
  	void 		setLineHeight(float height);
	float 		stringWidth(string s);
	float 		stringHeight(string s);

	void 		drawString(string s, float x, float y);
	int 		nCharacters;

protected:

	float 			lineHeight;
	charProps 		* 	cps;			// properties for each character
	GLuint			*	texNames;		// textures for each character
	int				fontSize;

	void 			drawChar(int c, float x, float y);
	int 			ofNextPow2(int a);
	int				border, visibleBorder;


};


#endif

