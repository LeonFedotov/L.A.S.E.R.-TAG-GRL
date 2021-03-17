#include "ofTCPClient.h"
#include "ofAppRunner.h"  

//--------------------------
ofTCPClient::ofTCPClient(){
	
	verbose		= true;
	connected	= false;
	messageSize = 0;
	port		= 0;
	index		= -1;
	str			= "";
	tmpStr		= "";
	ipAddr		="000.000.000.000";
}

//--------------------------
ofTCPClient::~ofTCPClient(){
	close();
}

//--------------------------
void ofTCPClient::setVerbose(bool _verbose){
	verbose = _verbose;
}

//--------------------------
bool ofTCPClient::setup(string ip, int _port){
	
	if( !TCPClient.Create() ){
		if(verbose)printf("ofTCPClient: Create() failed\n");
		return false;
	}else if( !TCPClient.Connect((char *)ip.c_str(), _port) ){
		if(verbose)printf("ofTCPClient: Connect(%s, %i) failed\n", ip.c_str(), _port);
		return false;
	}
	
	port		= _port;
	ipAddr		= ip;
	connected	= true;

	TCPClient.SetNonBlocking(true);			
	startThread(false, false);
	
	return true;
}

//don't use this one
//for server to use internally only!
//--------------------------
bool ofTCPClient::setup(int _index){
	index = _index;
	
	//this fetches the port that the server 
	//sets up the connection on
	//different to the server's listening port
	InetAddr addr;
	if( TCPClient.GetRemoteAddr(&addr) ){
		port   =  addr.GetPort();
		ipAddr =  addr.DottedDecimal();
	}
	
	connected = true;
	
	TCPClient.SetNonBlocking(true);
	startThread(false, false);
	return true;
}


//--------------------------
bool ofTCPClient::close(){
	if( connected ){
		
		stopThread(); //stop the thread
		
		if( !TCPClient.Close() ){
			if(verbose)printf("ofTCPClient: Close() failed\n");
			return false;
		}else{
			connected = false;
			return true;
		}
	}else{
		return true;
	}
}

//--------------------------
bool ofTCPClient::send(string message){

	//fuck this is ghetto 
	//it is the only way I can think of
	//to make sure we get the full message
	//there must be a better way then this :)
	message += "[ofTCPEnd]\0";

	if( !TCPClient.SendAll( message.c_str(), message.length()+1 ) ){
		if(verbose)printf("ofTCPClient: send() failed\n");
		return false;
	}else{
		return true;
	}
}

//--------------------------
string ofTCPClient::receive(){
	
	if( lock() ){
		int pos = str.find("[ofTCPEnd]",0);
		if(pos >= 0) tmpStr = str.substr(0,pos);
		unlock();
	}
	
	return tmpStr;
}

//--------------------------
bool ofTCPClient::isConnected(){
	return connected;
}

//--------------------------
int ofTCPClient::getPort(){
	return port;
}

//--------------------------
string ofTCPClient::getIP(){
	return ipAddr;
}		

//don't call this
//--------------------------
void ofTCPClient::threadedFunction(){
			
	while( isThreadRunning() ){
		if( lock() ){
			messageSize = TCPClient.Receive(tmpBuff,TCP_MAX_MSG_SIZE);
			if(messageSize > 0)str = tmpBuff;
			unlock();
		}
		ofSleepMillis(2);
	}
	
}





