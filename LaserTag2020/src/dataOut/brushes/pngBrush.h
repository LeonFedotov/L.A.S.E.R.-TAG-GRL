#ifndef _PNG_BRUSH_H
#define _PNG_BRUSH_H

#include "ofMain.h"


#include "drips.h"
#include "baseBrush.h"

//for small speed improvement?
#define ONE_OVER_255 0.00392157

class pngBrush : public baseBrush{
    
public:
    
    //stuff we have overidden from baseBrush
    
    void setupCustom();
    void clear();
    
    void addPoint(float _x, float _y, bool newStroke);
    void update();
    
    unsigned char * getImageAsPixels();
    void drawTool(int x, int y, int w, int h);
    
    void dripsSettings(bool doDrip, int dripFreq, float speed, int direction, int dwidth);
    void setBrushColor(int r, int g, int b, int a);
    void setBrushWidth(int width);
    void drawBrushColor(float x, float y, int w, int h);
    int  loadbrushes(string brushDir);
    void setBrushNumber(int _num);
    
    ofTexture & getTexture();
    
protected:
    
    
    void updateTmpImage(float _brushWidth);
    string getbrushName();
    
    
    ofTexture texture;
    ofDirectory DL;
    drips DRIPS;
    
    ofImage IMG;
    ofImage TMP;
    
    unsigned char * pixels;
    
    int oldX, oldY, numBrushes, imageNumBytes;
    bool drip;
    int dripCount, dripPattern;
};



#endif
