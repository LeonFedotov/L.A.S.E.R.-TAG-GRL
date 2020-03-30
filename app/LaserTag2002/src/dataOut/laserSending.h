#ifndef _LASER_SENDING_
#define _LASER_SENDING_

#include "ofMain.h"
#include "ofxTCPServer.h"
#include "ofxUDPManager.h"

class laserSending{
	public:
		
		// ------------------------		
		laserSending();
		~laserSending();
		
		bool setupTCPServer(int port);
		bool setupUDP(string ip, int port);

		void close();
		
		bool isSetup();
		
		bool isTCPSetup();
		bool isUDPSetup();
			
		bool checkNewTCPClient();
		
		string getLatestTCPClientIP();
		int getLatestTCPClientPort();
						
		bool sendData(string dataString);
	
		
	protected:
		bool udpSetup;
		bool tcpSetup;
		int  numConnected;
	
		ofxTCPServer TCP;
		ofxUDPManager  UDP;
};

#endif
