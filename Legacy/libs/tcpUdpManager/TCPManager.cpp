#include "TCPManager.h"
#include <stdio.h>

//--------------------------------------------------------------------------------
bool TCPManager::m_bWinsockInit= false;

//--------------------------------------------------------------------------------
TCPManager::TCPManager() 
{
  // was winsock initialized?
  	#ifdef TARGET_WIN32
		if (!m_bWinsockInit) {
			WORD vr;
			WSADATA	wsaData;
			vr=	MAKEWORD(2,	2);
			WSAStartup(vr, &wsaData);
			m_bWinsockInit=	true;			
		}
	#endif
	
  nonBlocking = false;
  m_hSocket= INVALID_SOCKET; 
  m_dwTimeoutSend= DEFAULT_TIMEOUT; 
  m_dwTimeoutReceive= DEFAULT_TIMEOUT; 
  m_dwTimeoutAccept= DEFAULT_TIMEOUT; 
  m_iListenPort= -1;
};

//--------------------------------------------------------------------------------
/// Closes an open socket.
/// NOTE: A closed socket cannot be reused again without a call to "Create()".
bool TCPManager::Close()
{
  if (m_hSocket == INVALID_SOCKET) return(false);

	#ifdef TARGET_WIN32
		if(closesocket(m_hSocket) == SOCKET_ERROR) 
	#else 
		if(close(m_hSocket) == SOCKET_ERROR) 
	#endif	
	{
		return(false);
	}
	
	m_hSocket= INVALID_SOCKET;
 
	#ifdef TARGET_WIN32
		//This was commented out in the original
		//WSACleanup();
	#endif
	return(true);
}

void TCPManager::CleanUp() {
	#ifdef TARGET_WIN32
		WSACleanup();
	#endif
  m_bWinsockInit = false;
}

//--------------------------------------------------------------------------------
bool TCPManager::Create()
{
  if (m_hSocket != INVALID_SOCKET) return(false);
  
  m_hSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_IP);
  return(m_hSocket != INVALID_SOCKET);
}


//--------------------------------------------------------------------------------
bool TCPManager::Listen(int iMaxConnections)
{
  if (m_hSocket == INVALID_SOCKET) return(false);
  m_iMaxConnections = iMaxConnections;
	return(listen(m_hSocket, iMaxConnections) != SOCKET_ERROR);
}

bool TCPManager::Bind(USHORT usPort)
{
	struct sockaddr_in local;
	memset(&local, 0, sizeof(sockaddr_in));
	
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	//Port MUST be in Network Byte Order
	local.sin_port = htons(usPort);

	if (bind(m_hSocket,(struct sockaddr*)&local,sizeof(local)))
		return false;
	return true;
}

//--------------------------------------------------------------------------------
bool TCPManager::Accept(TCPManager& sConnect)
{
  sockaddr_in addr;
  
  #ifndef TARGET_WIN32
	socklen_t iSize;
  #else
	int iSize;
  #endif 
  
  if (m_hSocket == INVALID_SOCKET) return(false);
  
  if (m_dwTimeoutAccept != NO_TIMEOUT) {
    FD_SET fd= {1, m_hSocket};
	  TIMEVAL tv= {m_dwTimeoutAccept, 0};
	  if(select(0, &fd, NULL, NULL, &tv) == 0) {
		  return(false);
	  }
  }

  iSize= sizeof(sockaddr_in);
  sConnect.m_hSocket= accept(m_hSocket, (sockaddr*)&addr, &iSize);
  return(sConnect.m_hSocket != INVALID_SOCKET);
}

//--------------------------------------------------------------------------------
bool TCPManager::Connect(char *pAddrStr, USHORT usPort)
{
  sockaddr_in addr_in= {0};
  struct hostent *he;

  if (m_hSocket == INVALID_SOCKET) return(false);
	
  if ((he = gethostbyname(pAddrStr)) == NULL) 
    return(false);
  
	addr_in.sin_family= AF_INET; // host byte order
	addr_in.sin_port  = htons(usPort); // short, network byte order
	addr_in.sin_addr  = *((struct in_addr *)he->h_addr);
  
	return(connect(m_hSocket, (LPSOCKADDR)&addr_in, sizeof(SOCKADDR)) != SOCKET_ERROR);
}

//--------------------------------------------------------------------------------
///Theo added - Choose to set nonBLocking - default mode is to block
bool TCPManager::SetNonBlocking(bool useNonBlocking)
{
	nonBlocking		= useNonBlocking;
	
	#ifdef TARGET_WIN32
		unsigned long arg = nonBlocking;
		int retVal = ioctlsocket(m_hSocket,FIONBIO,&arg);
	#else
		int arg = nonBlocking;
		int retVal = ioctl(m_hSocket,FIONBIO,&arg); 
	#endif	
		
	return (retVal >= 0);
}

//--------------------------------------------------------------------------------
int TCPManager::Write(const char* pBuff, const int iSize)
{
	int iBytesSent= 0;
	int iBytesTemp;
	const char* pTemp= pBuff;
	
  do {
	  iBytesTemp= Send(pTemp, iSize - iBytesSent);
    // error occured?
    if (iBytesTemp == SOCKET_ERROR) return(SOCKET_ERROR);
    if (iBytesTemp == SOCKET_TIMEOUT) return(SOCKET_TIMEOUT);

		iBytesSent+= iBytesTemp;
		pTemp+= iBytesTemp;
	} while(iBytesSent < iSize);
	
  return(iBytesSent);
}

//--------------------------------------------------------------------------------
//Theo added - alternative to GetTickCount for windows
//This version returns the milliseconds since the unix epoch
//Should be good enough for what it is being used for here
//(mainly time comparision)
#ifndef TARGET_WIN32 
unsigned long GetTickCount(){
  timeb bsdTime;
  ftime(&bsdTime);
  
   unsigned long msSinceUnix = (bsdTime.time*1000) + bsdTime.millitm;
   return msSinceUnix;
}
#endif

//--------------------------------------------------------------------------------
/// Return values:
/// SOCKET_TIMEOUT indicates timeout 
/// SOCKET_ERROR in case of a problem.
int TCPManager::Send(const char* pBuff, const int iSize)
{
  if (m_hSocket == INVALID_SOCKET) return(SOCKET_ERROR);
  
  if (m_dwTimeoutSend != NO_TIMEOUT) {
    FD_SET fd= {1, m_hSocket};
	  TIMEVAL tv= {m_dwTimeoutSend, 0};	
    if(select(0, NULL, &fd, NULL, &tv) == 0) {
		  return(SOCKET_TIMEOUT);
	  }	
  }
	return(send(m_hSocket, pBuff, iSize, 0));
}

//--------------------------------------------------------------------------------
/// Return values:
/// SOCKET_TIMEOUT indicates timeout 
/// SOCKET_ERROR in case of a problem.
int TCPManager::SendAll(const char* pBuff, const int iSize)
{
  if (m_hSocket == INVALID_SOCKET) return(SOCKET_ERROR);
  

	DWORD timestamp= GetTickCount();

	if (m_dwTimeoutSend != NO_TIMEOUT) {
		FD_SET fd= {1, m_hSocket};
		TIMEVAL tv= {m_dwTimeoutSend, 0};	
		if(select(0, NULL, &fd, NULL, &tv) == 0) {
			return(SOCKET_TIMEOUT);
		}	
	}
		
	int total= 0;
	int bytesleft = iSize;
	int ret;

	while (total < iSize) {
		ret = send(m_hSocket, pBuff + total, bytesleft, 0);
		if (ret == -1) { break; }
		total += ret;
		bytesleft -=ret;
		if (GetTickCount() - timestamp > m_dwTimeoutSend * 1000) return SOCKET_TIMEOUT;
	}

	return ret==-1?SOCKET_ERROR:total; 
}


//--------------------------------------------------------------------------------
/// Return values:
/// SOCKET_TIMEOUT indicates timeout 
/// SOCKET_ERROR in case of a problem.
int TCPManager::Receive(char* pBuff, const int iSize)
{
  if (m_hSocket == INVALID_SOCKET) return(SOCKET_ERROR);

  if (m_dwTimeoutReceive != NO_TIMEOUT) {
    FD_SET fd= {1, m_hSocket};
	  TIMEVAL tv= {m_dwTimeoutReceive, 0};	
    if(select(0, &fd, NULL, NULL, &tv) == 0) {
		  return(SOCKET_TIMEOUT);
	  }
  }
	return(recv(m_hSocket, pBuff, iSize, 0));
}


//--------------------------------------------------------------------------------
/// Return values:
/// SOCKET_TIMEOUT indicates timeout 
/// SOCKET_ERROR in case of a problem.
int TCPManager::ReceiveAll(char* pBuff, const int iSize)
{
	if (m_hSocket == INVALID_SOCKET) return(SOCKET_ERROR);

	DWORD timestamp= GetTickCount();

	if (m_dwTimeoutReceive != NO_TIMEOUT) {
	FD_SET fd= {1, m_hSocket};
		TIMEVAL tv= {m_dwTimeoutReceive, 0};	
	if(select(0, &fd, NULL, NULL, &tv) == 0) {
			return(SOCKET_TIMEOUT);
		}
	}
	int totalBytes=0;
	
	DWORD stamp = GetTickCount();

	do {
		int ret= recv(m_hSocket, pBuff+totalBytes, iSize-totalBytes, 0);
		if (ret==0 && totalBytes != iSize) return SOCKET_ERROR;
		if (ret < 0) return SOCKET_ERROR;
		if (GetTickCount() - timestamp > m_dwTimeoutReceive * 1000) return SOCKET_TIMEOUT;
		totalBytes += ret;
		#ifndef TARGET_WIN32
			usleep(20000); //should be 20ms
		#else	
			Sleep(20);
		#endif 
		if (GetTickCount() - stamp > 10000)
			return SOCKET_TIMEOUT;
	}while(totalBytes < iSize);

/*
	if (totalBytes > 0)
	{
		char out[400];
		sprintf(out, "%d bytes received:", totalBytes);
		int len = strlen(out);
		memcpy((char*)out + len, pBuff, totalBytes);
		len += totalBytes; 
		out[len] = 0;
		OutputDebugString(out);
	}
*/
	return totalBytes;
}

//--------------------------------------------------------------------------------
bool TCPManager::GetRemoteAddr(LPINETADDR pInetAddr)
{
  if (m_hSocket == INVALID_SOCKET) return(false);
	
	#ifndef TARGET_WIN32
		socklen_t iSize;
	#else
		int iSize;
	#endif 
	
	iSize= sizeof(SOCKADDR);
	return(getpeername(m_hSocket, (LPSOCKADDR)pInetAddr, &iSize) != SOCKET_ERROR);
}

//--------------------------------------------------------------------------------
bool TCPManager::GetInetAddr(LPINETADDR pInetAddr)
{
  if (m_hSocket == INVALID_SOCKET) return(false);
	
	#ifndef TARGET_WIN32
		socklen_t iSize;
	#else
		int iSize;
	#endif 	
	
	iSize= sizeof(SOCKADDR);
	return(getsockname(m_hSocket, (LPSOCKADDR)pInetAddr, &iSize) != SOCKET_ERROR);
}

void TCPManager::SetTimeoutSend(int timeoutInSeconds) {
	m_dwTimeoutSend= timeoutInSeconds;
}
void TCPManager::SetTimeoutReceive(int timeoutInSeconds) {
	m_dwTimeoutReceive= timeoutInSeconds;
}
void TCPManager::SetTimeoutAccept(int timeoutInSeconds) {
	m_dwTimeoutAccept= timeoutInSeconds;
}
int TCPManager::GetTimeoutSend() {
	return m_dwTimeoutSend;
}
int TCPManager::GetTimeoutReceive() {
	return m_dwTimeoutReceive;
}
int TCPManager::GetTimeoutAccept() {
	return m_dwTimeoutAccept;
}

int TCPManager::GetReceiveBufferSize() {
	if (m_hSocket == INVALID_SOCKET) return(false);
	
	#ifndef TARGET_WIN32
		socklen_t size;
	#else
		int size;
	#endif 
	
	int sizeBuffer=0;
	size = sizeof(int);
	getsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&sizeBuffer, &size);
	
	return sizeBuffer;
}

bool TCPManager::SetReceiveBufferSize(int sizeInByte) {
	if (m_hSocket == INVALID_SOCKET) return(false);
	
	if ( setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&sizeInByte, sizeof(sizeInByte)) == 0)
		return true;
	else
		return false;
}

int TCPManager::GetSendBufferSize() {
	if (m_hSocket == INVALID_SOCKET) return(false);
	
	#ifndef TARGET_WIN32
		socklen_t size;
	#else
		int size;
	#endif 
	
	int sizeBuffer=0;
	size = sizeof(int);
	getsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sizeBuffer, &size);
	
	return sizeBuffer;
}

bool TCPManager::SetSendBufferSize(int sizeInByte) {
	if (m_hSocket == INVALID_SOCKET) return(false);
	
	if ( setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sizeInByte, sizeof(sizeInByte)) == 0)
		return true;
	else
		return false;
}

int TCPManager::GetMaxConnections() {
  return m_iMaxConnections;
}

bool TCPManager::CheckHost(const char *pAddrStr) {
  LPHOSTENT hostEntry;
  in_addr iaHost;
  iaHost.s_addr = inet_addr(pAddrStr);
  hostEntry = gethostbyaddr((const char *)&iaHost, sizeof(struct in_addr), AF_INET);
  return ((!hostEntry) ? false : true);
}

