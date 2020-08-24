#ifndef _LASER_SENDING_
#define _LASER_SENDING_

#include "ofMain.h"
#include "ofxOsc.h"

class laserSending{
public:
    
    // ------------------------
    laserSending();
    ~laserSending();
    
    bool setup(string host, int port);
    
    void close();
    
    bool isSetup();
    
    bool sendData(float x, float y, bool isNewStroke);
    
protected:
    bool bSetup;
    int  port;
    string host;
    ofxOscSender   OSC;
};

#endif
