#define _CRT_SECURE_NO_DEPRECATE

#include "UDPManager.h"

//--------------------------------------------------------------------------------
bool UDPManager::m_bWinsockInit= false;

//--------------------------------------------------------------------------------
UDPManager::UDPManager() 
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

	m_hSocket= INVALID_SOCKET; 
	m_dwTimeoutReceive=	DEFAULT_TIMEOUT; 
	m_iListenPort= -1;

	canGetRemoteAddress	= false;
	nonBlocking			= false;

};

//--------------------------------------------------------------------------------
///	Closes an open socket.
///	NOTE: A	closed socket cannot be	reused again without a call	to "Create()".
bool UDPManager::Close()
{
	if (m_hSocket == INVALID_SOCKET) 
		return(false);
	
	#ifdef TARGET_WIN32
		if(closesocket(m_hSocket) == SOCKET_ERROR) 
	#else 
		if(close(m_hSocket) == SOCKET_ERROR) 
	#endif
	{
		return(false);
	}
	m_hSocket= INVALID_SOCKET;

	return(true);
}

//--------------------------------------------------------------------------------
bool UDPManager::Create()
{
	if (m_hSocket != INVALID_SOCKET) 
		return(false);
	m_hSocket =	socket(AF_INET,	SOCK_DGRAM,	IPPROTO_UDP);
	if (m_hSocket != INVALID_SOCKET)
	{
		BOOL val = TRUE;
		setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(BOOL));
	}
	return(m_hSocket !=	INVALID_SOCKET);
}

//--------------------------------------------------------------------------------
///Theo added - Choose to set nonBLocking - default mode is to block
bool UDPManager::SetNonBlocking(bool useNonBlocking)
{
	nonBlocking		= useNonBlocking;
	
	#ifdef TARGET_WIN32
		unsigned long arg = nonBlocking;	
		int retVal = ioctlsocket(m_hSocket,FIONBIO,&arg);
	#else
		int arg			= nonBlocking;	
		int retVal = ioctl(m_hSocket,FIONBIO,&arg);	
	#endif
	
	return (retVal >= 0);
}

//--------------------------------------------------------------------------------
bool UDPManager::Bind(USHORT usPort)
{
	saServer.sin_family	= AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;
	//Port MUST	be in Network Byte Order
	saServer.sin_port =	htons(usPort);
	
	int	ret	= bind(m_hSocket,(struct sockaddr*)&saServer,sizeof(struct sockaddr));

	//not using this var anyway but maybe should leave it in for debug
	#ifdef TARGET_WIN32
		int	err	= WSAGetLastError();
	#else
		int err = errno;
	#endif
		
	return (ret	== 0);
}

//--------------------------------------------------------------------------------
bool UDPManager::BindMcast(char *pMcast, USHORT usPort)
{
	// bind to port
	if (!Bind(usPort))
	{
		printf("can't bind to port \n");
		return false;
	}

	// join the multicast group
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(pMcast);
	mreq.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR*) &mreq, sizeof (mreq)) == SOCKET_ERROR)
	{
		//printf("setsockopt failed! Error: %d", WSAGetLastError());
		return false;
	}
	printf("here ? \n");
	// multicast bind successful
	return true;
}

//--------------------------------------------------------------------------------
bool UDPManager::Connect(char *pHost, USHORT usPort)
{
	//	sockaddr_in	addr_in= {0};
	memset(&saClient, 0, sizeof(sockaddr_in));
	struct hostent *he;

	if (m_hSocket == INVALID_SOCKET) return(false);

	if ((he	= gethostbyname(pHost))	== NULL) 
		return(false);

	saClient.sin_family= AF_INET; // host byte order
	saClient.sin_port  = htons(usPort);	// short, network byte order
	//	saClient.sin_addr  = *((struct g_addr *)he->h_addr_list);
	saClient.sin_addr.s_addr= inet_addr( pHost ); 

	return true;
}

//--------------------------------------------------------------------------------
bool UDPManager::ConnectMcast(char* pMcast, USHORT usPort)
{
	// associate the source socket's address with the socket
	if (!Bind(usPort))
	{
#ifdef _DEBUG
		printf("Binding socket failed! Error: %d", WSAGetLastError());
#endif
		return false;
	}

	// set ttl to default
	if (!SetTTL(1))
	{
#ifdef _DEBUG
		printf("SetTTL failed. Continue anyway. Error: %d", WSAGetLastError());
#endif
	}

	if (!Connect(pMcast, usPort))
	{
#ifdef _DEBUG
		printf("Connecting socket failed! Error: %d", WSAGetLastError ());
#endif
		return false;
	}

	// multicast connect successful
	return true;
}

//--------------------------------------------------------------------------------
///	Return values:
///	SOCKET_TIMEOUT indicates timeout 
///	SOCKET_ERROR in	case of	a problem.
int	UDPManager::Send(const char* pBuff,	const int iSize)
{
	if (m_hSocket == INVALID_SOCKET) return(SOCKET_ERROR);

	if (m_dwTimeoutSend	!= NO_TIMEOUT) 
	{
		FD_SET fd= {1, m_hSocket};
		TIMEVAL	tv=	{m_dwTimeoutSend, 0};	
		if(select(0, NULL, &fd,	NULL, &tv) == 0) 
		{
			return(SOCKET_TIMEOUT);
		}	
	}

	return (sendto(m_hSocket, (char*)pBuff,	iSize, 0, (LPSOCKADDR)&saClient, sizeof(SOCKADDR)));
	//	return(send(m_hSocket, pBuff, iSize, 0));
}

//--------------------------------------------------------------------------------
///	Return values:
///	SOCKET_TIMEOUT indicates timeout 
///	SOCKET_ERROR in	case of	a problem.
int	UDPManager::SendAll(const char*	pBuff, const int iSize)
{
	if (m_hSocket == INVALID_SOCKET) return(SOCKET_ERROR);

	if (m_dwTimeoutSend	!= NO_TIMEOUT) 
	{
		FD_SET fd= {1, m_hSocket};
		TIMEVAL	tv=	{m_dwTimeoutSend, 0};	
		if(select(0, NULL, &fd,	NULL, &tv) == 0) 
		{
			return(SOCKET_TIMEOUT);
		}	
	}


	int	total= 0;
	int	bytesleft =	iSize;
	int	n;

	while (total < iSize) 
	{
		n =	sendto(m_hSocket, (char*)pBuff,	iSize, 0, (LPSOCKADDR)&saClient, sizeof(SOCKADDR));
		if (n == -1) 
			{ 
				break; 
			}
		total += n;
		bytesleft -=n;
	}

	return n==-1?SOCKET_ERROR:total; 
}



//--------------------------------------------------------------------------------
///	Return values:
///	SOCKET_TIMEOUT indicates timeout 
///	SOCKET_ERROR in	case of	a problem.
int	UDPManager::Receive(char* pBuff, const int iSize)
{
	if (m_hSocket == INVALID_SOCKET) 
		return(SOCKET_ERROR);

	if (m_dwTimeoutReceive != NO_TIMEOUT) 
	{
		FD_SET fd= {1, m_hSocket};
		TIMEVAL	tv=	{m_dwTimeoutReceive, 0};	
		if(select(0, &fd, NULL,	NULL, &tv) == 0) {
			return(SOCKET_TIMEOUT);
		}
	}
	
	#ifndef TARGET_WIN32
		socklen_t nLen= sizeof(SOCKADDR);
	#else
		int	nLen= sizeof(SOCKADDR);
	#endif 
	
	int	ret=0;

	memset(pBuff, 0, iSize);
	ret= recvfrom(m_hSocket, pBuff,	iSize, 0, (LPSOCKADDR)&saClient, &nLen);

	if (ret	> 0) 
	{
		//		printf("\nreceived from: %s\n",	inet_ntoa((in_addr)saClient.sin_addr));	
		canGetRemoteAddress= true;
	}
	else 
	{
		//		printf("\nreceived from: ????\n");	
		canGetRemoteAddress= false;
	}

	return ret;
	//	return(recvfrom(m_hSocket, pBuff, iSize, 0));
}

void UDPManager::SetTimeoutSend(int	timeoutInSeconds) 
{
	m_dwTimeoutSend= timeoutInSeconds;
}
void UDPManager::SetTimeoutReceive(int timeoutInSeconds) 
{
	m_dwTimeoutReceive=	timeoutInSeconds;
}
int	UDPManager::GetTimeoutSend() 
{
	return m_dwTimeoutSend;
}
int	UDPManager::GetTimeoutReceive()	
{
	return m_dwTimeoutReceive;
}

//--------------------------------------------------------------------------------
bool UDPManager::GetRemoteAddr(char* address)
{
	if (m_hSocket == INVALID_SOCKET) return(false);
	if ( canGetRemoteAddress ==	false) return (false);

	strcpy(address,	inet_ntoa((in_addr)saClient.sin_addr));
	return true;
}

//--------------------------------------------------------------------------------
int	UDPManager::GetMaxMsgSize()	
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	int	sizeBuffer=0;
		
	#ifndef TARGET_WIN32
		socklen_t size = sizeof(int);
	#else
		int size = sizeof(int);
	#endif 	
	
	getsockopt(m_hSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&sizeBuffer, &size);

	return sizeBuffer;
}

//--------------------------------------------------------------------------------
int	UDPManager::GetReceiveBufferSize() 
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	int	sizeBuffer=0;
			
	#ifndef TARGET_WIN32
		socklen_t size = sizeof(int);
	#else
		int size = sizeof(int);
	#endif 
	
	getsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&sizeBuffer, &size);

	return sizeBuffer;
}

//--------------------------------------------------------------------------------
bool UDPManager::SetReceiveBufferSize(int sizeInByte) 
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	if ( setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&sizeInByte, sizeof(sizeInByte)) == 0)
		return true;
	else
		return false;
}

//--------------------------------------------------------------------------------
int	UDPManager::GetSendBufferSize()	
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	int	sizeBuffer=0;
	
	#ifndef TARGET_WIN32
		socklen_t size = sizeof(int);
	#else
		int size = sizeof(int);
	#endif 
	
	getsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sizeBuffer, &size);

	return sizeBuffer;
}

//--------------------------------------------------------------------------------
bool UDPManager::SetSendBufferSize(int sizeInByte) 
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	if ( setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sizeInByte, sizeof(sizeInByte)) == 0)
		return true;
	else
		return false;
}

//--------------------------------------------------------------------------------
bool UDPManager::SetReuseAddress(bool allowReuse) 
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	int	on;
	if (allowReuse)	on=1;
	else			on=0;

	if ( setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) ==	0)
		return true;
	else
		return false;
}

//--------------------------------------------------------------------------------
bool UDPManager::SetEnableBroadcast(bool enableBroadcast) 
{
	int	on;
	if (enableBroadcast)	on=1;
	else					on=0;

	if ( setsockopt(m_hSocket, SOL_SOCKET, SO_BROADCAST, (char*)&on, sizeof(on)) ==	0)
		return true;
	else
		return false;
}

//--------------------------------------------------------------------------------
int UDPManager::GetTTL()
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	int nTTL;
	
	#ifndef TARGET_WIN32
		socklen_t nSize = sizeof(int);
	#else
		int nSize = sizeof(int);
	#endif 
	
	if (getsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char FAR *) &nTTL, &nSize) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		printf("getsockopt failed! Error: %d", WSAGetLastError());
#endif
		return -1;
	}

	return nTTL;
}

//--------------------------------------------------------------------------------
bool UDPManager::SetTTL(int nTTL)
{
	if (m_hSocket == INVALID_SOCKET) return(false);

	// Set the Time-to-Live of the multicast.
	if (setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char FAR *)&nTTL, sizeof (int)) == SOCKET_ERROR)
	{
#ifdef _DEBUG
		printf("setsockopt failed! Error: %d", WSAGetLastError());
#endif
		return false;
	}

	return true;
}


/*
//--------------------------------------------------------------------------------
bool UDPManager::GetInetAddr(LPINETADDR	pInetAddr)
{
if (m_hSocket == INVALID_SOCKET) return(false);

int	iSize= sizeof(SOCKADDR);
return(getsockname(m_hSocket, (LPSOCKADDR)pInetAddr, &iSize) !=	SOCKET_ERROR);
}
*/
