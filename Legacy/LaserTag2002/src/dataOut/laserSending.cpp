#include "laserSending.h"
		
// ------------------------		
laserSending::laserSending(){
	udpSetup 		= false;
	tcpSetup 		= false;
	numConnected 	= 0;
}

laserSending::~laserSending(){
	close();
}

//-------------------------
void laserSending::close(){
	if(udpSetup){
		UDP.Close();
		udpSetup = false;
	}
	if(tcpSetup){
		TCP.close();
		tcpSetup = false;
		numConnected = 0;
	}
}

// ------------------------		
bool laserSending::setupUDP(string ip, int port){
	close();
	if(UDP.Create() && UDP.Connect((char *)ip.c_str(), (unsigned short)port) ){
		udpSetup = true;
		return true;
	}
	return false;
}

// ------------------------		
bool laserSending::isSetup(){
	return (udpSetup || tcpSetup);
}

// ------------------------		
bool laserSending::isTCPSetup(){
	return tcpSetup;
}		

// ------------------------		
bool laserSending::isUDPSetup(){
	return udpSetup;
}		
		

// ------------------------		
bool laserSending::setupTCPServer(int port){
	close();
	if(TCP.setup(port)){
		tcpSetup = true;
		return true;
	}
	return false;
}
	
// ------------------------		
bool laserSending::checkNewTCPClient(){
	if(tcpSetup){
		if(TCP.getNumClients() > numConnected){
			numConnected = TCP.getNumClients();
			return true;
		}
	}
	return false;
}

// ------------------------		
string laserSending::getLatestTCPClientIP(){
	if(tcpSetup){
		if(TCP.getNumClients() > 0){
			return TCP.getClientIP(TCP.getNumClients()-1);
		}
	}
	return "???.???.???.???";
}

// ------------------------		
int laserSending::getLatestTCPClientPort(){
	if(tcpSetup){
		if(TCP.getNumClients() > 0){
			return TCP.getClientPort(TCP.getNumClients()-1);
		}
	}
	return 0;
}		
				
		
// ------------------------		
bool laserSending::sendData(string dataString){
	if(!udpSetup && !tcpSetup)return false;
	
	if(udpSetup)UDP.Send(dataString.c_str(), dataString.length());
	if(tcpSetup && TCP.getNumClients()){
		TCP.sendToAll(dataString.c_str());
	}
	return true;
}		
