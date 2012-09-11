#ifndef _BASE_BRUSH_H
#define _BASE_BRUSH_H

#include "ofMain.h"
#include "ofAddons.h"

//inhereit me to add your own brush
class baseBrush{
	
	public: 
		
		//------------------------------------------------
		//------------------------------------------------
		//=======   things you need to overide   =========
		//------------------------------------------------
		//------------------------------------------------

		//this is called automatically
		//you will already have access to width and height
		//paramaters
		virtual void setupCustom(){
			//your setup custom should include the following stuff

			//isVector   = false; //or true - decides if you are drawing with pixels or gl
			//isColor	 = false; //or true - if you are using pixels is it color or greyscale
	
			//our name and description
			//setName("ourBrushName");
			//setShortDescription("about this much text saying what we do");
	
		};
		
		//this is where new data comes in
		//should be 0.0 - 1.0 scale for coordinates
		//up to you to scale up to your width and height
		virtual void addPoint(float x, float y, bool bNewStroke){}; 
 		
		//if you need to animate inbetween when points come in
		virtual void update(){};
		
		//clear code here
		virtual void clear(){};
		
		//only for pixel brushes
		virtual unsigned char * getImageAsPixels(){
			
			printf("you should never see this message!\n- you need to make your own getImageAsPixels function\n");
			
			//we have to return something - so here is some garbage
			unsigned char tmp[3];
			return (unsigned char *) tmp;
		};
		
		//if you need to draw anything gui stuff for your
		//brush do it here
		virtual void drawTool(int x, int y, int w, int h){}
				
		//only for vector brushes
		virtual void draw(int x, int y, int w, int h){};
				
		//------------------------------------------------
		//------------------------------------------------
		//==== things that you can overide but need  =====
		//==  to maintain their existing functionaity ====
		//------------------------------------------------
		//------------------------------------------------
		
		//either your texture width and height - raster
		//or your projection width and height - vector
		void setup(int widthIn, int heightIn){
			width  			= widthIn;
			height 			= heightIn;
			brushBrightness = 100;
			brushNumber		= 0;
			brushWidth		= 10;

			//the inhereited setup
			//which you can use
			//you'll have width and height set
			setupCustom();
		}
		
		virtual void dripsSettings(bool doDrip, int dripFreq, float speed, int direction){
			dripsEnabled 	= doDrip;
			dripsFrequency 	= dripFreq;
			dripsDirection	= direction;
			dripsSpeed		= speed;
		}
		
		virtual void setName(string brushName){
			name = brushName;
		}
		
		virtual void setShortDescription(string brushDesription){
			description = brushDesription;
		}
		
		virtual void setBrushColor(unsigned char r, unsigned char g, unsigned b){
			red 	= r;
			green 	= g;
			blue	= b;
		}
		
		virtual void setBrushBrightness(int brightness){
			brushBrightness = brightness;
		}						
		
		virtual void setNumSteps(int steps){
			int numSteps;
		}		
		
		virtual void setBrushWidth(int width){
			brushWidth = width;
		}	
		
		virtual void setBrushNumber(int number){
			brushNumber = number;
		}	
		
		virtual int getBrushNumber(){
			return brushNumber;
		}
		
		virtual string getName(){
			return name;
		}
		
		virtual string getDescription(){
			return description;
		}
		
		virtual bool getIsVector(){
			return isVector;
		}
		
		virtual bool getIsColor(){
			return isColor;
		}
				
	
	protected:
		string 	name; 
		string 	description;
	
		bool 	dripsEnabled;
		bool 	isVector;  //pixel based or vector (openGL) based
		bool	isColor;
		
		int 	brushWidth; //width in pixels
		int		brushBrightness;
		int 	numSteps;
		int 	dripsFrequency;
		int		dripsDirection;
		int		brushNumber;
		int 	width;
		int		height;
		int		red;
		int		green;
		int		blue;
		
		float	dripsSpeed;
		
};


#endif
