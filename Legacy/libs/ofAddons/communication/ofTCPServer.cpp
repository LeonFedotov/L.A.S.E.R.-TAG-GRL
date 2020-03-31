#include "ofTCPServer.h"
#include "ofTCPClient.h"

//--------------------------
ofTCPServer::ofTCPServer(){
	verbose		= true;
	connected	= false;
	count		= 0;
	port		= 0;
	str			= "";
	
	TCPConnections = new ofTCPClient[TCP_MAX_CLIENTS];
}

//--------------------------
ofTCPServer::~ofTCPServer(){
	close();
}

//--------------------------			
void ofTCPServer::setVerbose(bool _verbose){
	verbose = _verbose;
}

//--------------------------	
bool ofTCPServer::setup(int _port){
	if( !TCPServer.Create() ){
		if(verbose)printf("ofTCPServer: create() failed\n");
		return false;
	}
	if( !TCPServer.Bind(_port) ){
		if(verbose)printf("ofTCPServer: bind(port = %i) failed\n", _port);
		return false;
	}
	
	connected = true;
	port	  = _port;
	
	startThread(true, false);
	return true;
}

//--------------------------
bool ofTCPServer::close(){
	
	if(!connected) return true;
	
	for(int i = 0; i < count; i++){
		TCPConnections[i].close();
	}
	
	stopThread(); //stop the thread
	
	if( !TCPServer.Close() ){
		if(verbose)printf("ofTCPServer: unable to close connection\n");
		return false;
	}else{
		connected = false;
		return true;
	}
}

//--------------------------
bool ofTCPServer::disconnectClient(int id){
	if( !checkId(id) ){
		if(verbose)printf("ofTCPServer: client %i doesn't exist\n", id);
		return false;
	}
	else if(TCPConnections[id].close()){
		return true;
	}
	return false;
}

//--------------------------
bool ofTCPServer::send(int id, string message){
	if( !checkId(id) ){
		if(verbose)printf("ofTCPServer: client %i doesn't exist\n", id);
		return false;
	}
	else{
		TCPConnections[id].send(message);
		return true;
	}
}

//--------------------------
bool ofTCPServer::sendToAll(string message){
	for(int i = 0; i < count; i++){
		if(TCPConnections[i].isConnected())TCPConnections[i].send(message);
	}
	return true;
}

//--------------------------
string ofTCPServer::receive(int id){
	if( !checkId(id) ){
		if(verbose)printf("ofTCPServer: client %i doesn't exist\n", id);
		return "client doesn't exist";
	}
	
	return TCPConnections[id].receive();
}		


//--------------------------
int ofTCPServer::getClientPort(int id){
	if( !checkId(id) ){
		if(verbose)printf("ofTCPServer: client %i doesn't exist\n", id);
		return 0;
	}
	else return TCPConnections[id].getPort();
}

//--------------------------
string ofTCPServer::getClientIP(int id){
	if( !checkId(id) ){
		if(verbose)printf("ofTCPServer: client %i doesn't exist\n", id);
		return "000.000.000.000";
	}
	else return TCPConnections[id].getIP();
}

//--------------------------
int ofTCPServer::getNumClients(){
	return count;
}

//--------------------------
int ofTCPServer::getPort(){
	return port;
}	

//--------------------------
bool ofTCPServer::isConnected(){
	return connected;
}

//--------------------------		
bool ofTCPServer::checkId(int id){
	return (id < count && id < TCP_MAX_CLIENTS); 
}

//don't call this
//--------------------------
void ofTCPServer::threadedFunction(){
				
	while( isThreadRunning() ){
		
		if(count == TCP_MAX_CLIENTS){
			if(verbose)printf("ofTCPServer: reached max connected clients! \nofTCPServer: no more connections accepted\n");
			break;
		}
		
		if( !TCPServer.Listen(TCP_MAX_CLIENTS) ){
			if(verbose)printf("ofTCPServer: Listen() failed\n");
		}
		
		if( !TCPServer.Accept(TCPConnections[count].TCPClient) ){
			if(verbose)printf("ofTCPServer: Accept() failed\n");
			continue;
		}else{
			TCPConnections[count].setup(count);
			count++;
			if(verbose)printf("ofTCPServer: client %i connected on port %i\n", count, TCPConnections[count].getPort());
		}
	}
	if(verbose)printf("ofTCPServer: listen thread ended\n");
}

	



