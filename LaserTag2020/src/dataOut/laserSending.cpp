#include "laserSending.h"
		
// ------------------------		
laserSending::laserSending(){

}

laserSending::~laserSending(){
	close();
}

//-------------------------
void laserSending::close(){
    OSC.clear();
    bSetup = false;
}

bool laserSending::setup(string host, int port){
    this->port = port;
    this->host = host;
    bSetup = OSC.setup(host, port);
}

bool laserSending::isSetup(){
    return bSetup;
}
		
// ------------------------		
bool laserSending::sendData(float x , float y, bool isNewStroke){
    ofxOscMessage m;
    m.setAddress("/LaserTag/pos");
    m.addFloatArg(x);
    m.addFloatArg(y);
    m.addIntArg(isNewStroke?1:0);
    OSC.sendMessage(m);
	return true;
}		
