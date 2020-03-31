#ifndef _BASE_GUI_
#define _BASE_GUI_

#include "ofMain.h"
#include "ofAddons.h"

static string commonText = "";
static int	textFade = 0;
static int  timeMs = 0;

class baseGui
{
	public:
		
		// ------------------------
		virtual void useTrueTypeFont(string font, int fontSize){
			TTF.loadFont(font, fontSize);
			bUseTTF = true;
		}
			
		// ------------------------
		virtual void drawText(string text, float x, float y){
			if(bUseTTF)TTF.drawString(text, x, y);
			else  ofDrawBitmapString(text, x, y);
		}
		
		// ------------------------		
		virtual void setCommonText(string theText){
			commonText 	= theText;
			textFade	= 255;
			timeMs		= ofGetElapsedTimeMillis();
		}
		
		// ------------------------		
		virtual string getCommonText(){
			return commonText;
		}
		
		//-------------------------
		virtual void setFade(int fade){
			textFade = fade;
		}
		
		//-------------------------
		virtual int getFade(){
			return textFade;
		}
		
		//-------------------------
		virtual void setFadeTimeMs(int millis){
			timeMs = millis;
		}
		
		//-------------------------
		virtual int getFadeTimeMs(){
			return timeMs;
		}
				
	protected:
		
		ofTrueTypeFont  TTF;
		bool			bUseTTF;
	
};

#endif