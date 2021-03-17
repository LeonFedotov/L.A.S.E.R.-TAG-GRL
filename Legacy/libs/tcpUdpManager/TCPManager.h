#ifndef  ___TCPMANAGER__H__
#define  ___TCPMANAGER__H__

#pragma once

//////////////////////////////////////////////////////////////////////////////////////
// Original author: ???????? we think Christian Naglhofer  
// Crossplatform port by: Theodore Watson May 2007
// Changes: Mac (and should be nix) equivilant functions and data types for  
// win32 calls, artificial nix version of GetTickCount() used for timestamp 
//
//////////////////////////////////////////////////////////////////////////////////////


/*************************************************************** 
	USAGE
****************************************************************

TCP Socket Client:
------------------

1) create()
2) connect()
3) send()
...
x) close()

optional: 
SetTimeoutSend()


TCP Socket Server:
------------------

1) create()
2) bind()
3) listen()
4) accept()
5) receive()
...
x) close()

optional: 
SetTimeoutAccept()
SetTimeoutReceive()

****************************************************************/
#include "ofConstants.h"
#include <string.h>
#include <wchar.h>
#include <stdio.h>

#ifndef TARGET_WIN32
	
	//unix includes - works for osx should be same for *nix
	#include <ctype.h>	
	#include <netdb.h>
	#include <string.h>
	#include <fcntl.h>
	#include <time.h>
	#include <errno.h>
	#include <unistd.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>			
	#include <sys/timeb.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>	
	
	//variable defines
	#define USHORT	unsigned short
	#define DWORD	unsigned long
	#define ULONG	unsigned long
	#define WORD	unsigned short 
	#define BOOL	bool
		
	//other types
	#define FD_SET fd_set
	#define TIMEVAL timeval
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1	
	#define LPSOCKADDR struct sockaddr *
	#define LPSOCKADDR_IN struct sockaddr_in *
	#define SOCKADDR sockaddr
	#define SOCKADDR_IN sockaddr_in
	#define LPHOSTENT struct hostent *
	#define SOCKET int
	#define FAR 
	#define SO_MAX_MSG_SIZE TCP_MAXSEG

#else
	//windows includes
	#include <winsock2.h>
	#include <Ws2tcpip.h>		// TCP/IP annex needed for multicasting
#endif

//--------------------------------------------------------------------------------
class InetAddr : public sockaddr_in
{
public:
  // constructors
	InetAddr() { 
		memset(this, 0, sizeof(InetAddr));
		sin_family= AF_INET;
		sin_port= 0;
		sin_addr.s_addr= 0;
	}; 
  
	InetAddr(const SOCKADDR& sockAddr) { 
		memcpy(this, &sockAddr, sizeof(SOCKADDR)); 
	};
	
	InetAddr(const SOCKADDR_IN& sin) { 
		memcpy(this, &sin, sizeof(SOCKADDR_IN)); 
	};
	
	InetAddr(const ULONG ulAddr, const USHORT ushPort= 0) {  
		memset(this, 0, sizeof(InetAddr));
		sin_family= AF_INET;
		sin_port= htons(ushPort);
		sin_addr.s_addr= htonl(ulAddr);
	};
  
  InetAddr(const wchar_t* pStrIP, const USHORT usPort= 0) {  
		char szStrIP[32];
		
		#ifdef TARGET_WIN32 
			WideCharToMultiByte(CP_ACP, 0, pStrIP, (int)wcslen(pStrIP) + 1, szStrIP, 32, 0, 0);
		#else						
			//theo note:
			//do we need to set the codepage here first? 
			//or is the default one okay?
			wcstombs(szStrIP, pStrIP, 32);
		#endif
		
		memset(this, 0, sizeof(InetAddr));
		sin_family= AF_INET;
		sin_port= htons(usPort);
		sin_addr.s_addr= inet_addr(szStrIP); 
	}

	InetAddr(const char* pStrIP, const USHORT usPort= 0) {  
		memset(this, 0, sizeof(InetAddr));
		sin_family= AF_INET;
		sin_port= htons(usPort);
		sin_addr.s_addr= inet_addr(pStrIP); 
	} 	
	/// returns the address in dotted-decimal format
	char* DottedDecimal() { return inet_ntoa(sin_addr); } 
	USHORT GetPort() const { return ntohs(sin_port); }
	ULONG GetIpAddr() const { return ntohl(sin_addr.s_addr); }
	/// operators added for efficiency
	const InetAddr& operator=(const SOCKADDR& sa) { 
		memcpy(this, &sa, sizeof(SOCKADDR));
		return *this; 
	}
	const InetAddr& operator=(const SOCKADDR_IN& sin) {
		memcpy(this, &sin, sizeof(SOCKADDR_IN));
		return *this; 
	}
	operator SOCKADDR() { return *((LPSOCKADDR)this); }
	operator LPSOCKADDR() { return (LPSOCKADDR)this; }
	operator LPSOCKADDR_IN() { return (LPSOCKADDR_IN) this; }
};

typedef const InetAddr*		LPCINETADDR;
typedef InetAddr*			LPINETADDR;


//--------------------------------------------------------------------------------
/// Socket constants.
#define SOCKET_TIMEOUT      SOCKET_ERROR - 1
#define NO_TIMEOUT          0xFFFF
#define DEFAULT_TIMEOUT     NO_TIMEOUT


//--------------------------------------------------------------------------------
/// Implementation of a TCP socket.
class TCPManager
{
public:
	TCPManager();
	virtual ~TCPManager() { 
		if ((m_hSocket)&&(m_hSocket != INVALID_SOCKET)) Close(); 
	};
	
	bool Close();
	bool Create();
	bool Listen(int iMaxConnections);
	bool Connect(char *pAddrStr, USHORT usPort);
	bool Bind(USHORT usPort);
	bool Accept(TCPManager& sock);
	//sends the data, but it is not guaranteed that really all data will be sent
	int  Send(const char* pBuff, const int iSize);
	//all data will be sent guaranteed. 
	int  SendAll(const char* pBuff, const int iSize);	
	int  Receive(char* pBuff, const int iSize);
	int  ReceiveAll(char* pBuff, const int iSize);
	int  Write(const char* pBuff, const int iSize);
	bool GetRemoteAddr(LPINETADDR pIntAddr);
	bool GetInetAddr(LPINETADDR pInetAddr);
	void SetTimeoutSend(int timeoutInSeconds);
	void SetTimeoutReceive(int timeoutInSeconds);
	void SetTimeoutAccept(int timeoutInSeconds);
	int  GetTimeoutSend();
	int  GetTimeoutReceive();
	int  GetTimeoutAccept();
	bool SetReceiveBufferSize(int sizeInByte);
	bool SetSendBufferSize(int sizeInByte);
	int  GetReceiveBufferSize();
	int  GetSendBufferSize();
	int  GetMaxConnections();
	bool SetNonBlocking(bool useNonBlocking);
	bool CheckHost(const char *pAddrStr);
	void CleanUp();

protected:
  int m_iListenPort;
  int m_iMaxConnections;
  SOCKET m_hSocket;
  DWORD m_dwTimeoutSend;
  DWORD m_dwTimeoutReceive;
  DWORD m_dwTimeoutAccept;
  bool nonBlocking;
  static bool m_bWinsockInit;
};

#endif // ___TCPMANAGER__H__
