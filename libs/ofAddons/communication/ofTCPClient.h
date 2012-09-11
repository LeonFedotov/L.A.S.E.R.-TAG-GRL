#ifndef _OF_TCP_CLIENT_
#define _OF_TCP_CLIENT_

#include "ofConstants.h"
#include "ofThread.h"
#include "TCPManager.h"

#define TCP_MAX_MSG_SIZE 512

class ofTCPClient : public ofThread{

	public:
	
		ofTCPClient();
		~ofTCPClient();
		void setVerbose(bool _verbose);
		bool setup(string ip, int _port);		
		bool close();
		bool send(string message);
		string receive();
		bool isConnected();
		int getPort();
		string getIP();		
		
		//don't use this one
		//for server to use internally only!
		//--------------------------
		bool setup(int _index);
		
		//don't call this
		void threadedFunction();
		
		TCPManager	TCPClient;
		char		tmpBuff[TCP_MAX_MSG_SIZE];
		string		str, tmpStr, ipAddr;
		int			index, messageSize, port;
		bool		connected, verbose;
	
};

#endif	



