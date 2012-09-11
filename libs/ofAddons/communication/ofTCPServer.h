#ifndef _OF_TCP_SERVER_
#define _OF_TCP_SERVER_

#include "ofConstants.h"
#include "ofThread.h"
#include "TCPManager.h"

#define TCP_MAX_CLIENTS  32

//forward decleration
class ofTCPClient;

class ofTCPServer : public ofThread{

	public:
	
		ofTCPServer();
		~ofTCPServer();			
		void setVerbose(bool _verbose);	
		bool setup(int _port);
		bool close();
		bool disconnectClient(int id);
		bool send(int id, string message);
		bool sendToAll(string message);	
		string receive(int id);
		int getClientPort(int id);
		string getClientIP(int id);
		int getNumClients();
		int getPort();
		bool isConnected();		
		bool checkId(int id);
		
		//don't call this
		//--------------------------
		void threadedFunction();

		TCPManager	TCPServer;
		ofTCPClient * TCPConnections;
		bool		connected, verbose;
		string		str;
		int			count, port;
	
};


#endif	



