#ifndef _BASIC_VECTOR_BRUSH_H
#define _BASIC_VECTOR_BRUSH_H

#include "ofMain.h"
#include "ofAddons.h"

#include "baseBrush.h"

#define MAX_NUM_STROKES 256
#define MAX_STROKE_PTS 512

typedef struct{
	ofVec2f pts[MAX_STROKE_PTS];
	float width[MAX_STROKE_PTS];
	int num;
	int r;
	int g; 
	int b;
}singleStroke;


//true vector brush - uses openGL shapes
class basicVectorBrush : public baseBrush{
	
	public: 
		
		//------------------------
		void setupCustom(){
		
			//we are a vector brush
			isVector = true;
	
			//doesn't matter as we are an openGL brush
			isColor	 = false;
	
			//our name and description
			setName("basicVectorBrush");
			setShortDescription("openGL based brush - speed effects stroke width");
			
			whichStroke = -1;
			
			clear();
		}
		
		//------------------------
		void clear(){
			for(int i = 0; i < MAX_NUM_STROKES; i++){
				stroke[i].num = 0;
			}
			whichStroke = 0;
		}
		
		//------------------------
		void addPoint(float _x, float _y, bool isNewStroke){
		
			if(whichStroke >= MAX_NUM_STROKES){
				printf("vectorBrush::addPoint() - maxStrokes reached\n");
				return;
			}
		
			if(isNewStroke || whichStroke == -1 || stroke[whichStroke].num >= MAX_STROKE_PTS){
				whichStroke++;
				
				if(whichStroke >= MAX_NUM_STROKES){
					printf("vectorBrush::addPoint() - maxStrokes reached\n");
					return;
				}
				
				stroke[whichStroke].r 	= red;
				stroke[whichStroke].g 	= green;
				stroke[whichStroke].b 	= blue;
				
				stroke[whichStroke].num = 0;
				
				printf("new vec Stroke\n");
			}
		
			//scale up to our dimensions
			_x *= (float)width;
			_y *= (float)height;
			
			int pos = stroke[whichStroke].num;
			
			stroke[whichStroke].pts[pos].set(_x, _y);
			
			stroke[whichStroke].num++;
			
			
			printf("stroke[%i] pos %i \n", whichStroke, pos);

		}	
		
		
		//------------------------
		void draw(int x, int y, int w, int h){
			if(whichStroke < 0 || whichStroke == MAX_NUM_STROKES)return;
			
			float scaleX =  (float)w / (float)width;
			float scaleY =  (float)h / (float)height;
			
			glPushMatrix();
				glTranslatef(x, y, 0);
				glScalef(scaleX, scaleY, 1);
				
				for(int i = 0; i <= whichStroke; i++){
					ofSetColor(stroke[i].r, stroke[i].g, stroke[i].b);
					for(int j = 1; j < stroke[i].num; j++){
						ofLine(stroke[i].pts[j-1].x, stroke[i].pts[j-1].y, stroke[i].pts[j].x, stroke[i].pts[j].y);
					}
				}
			glPopMatrix();
		}
	
	protected:
		singleStroke stroke[MAX_NUM_STROKES]; 
		int whichStroke;
			
		
};


#endif
