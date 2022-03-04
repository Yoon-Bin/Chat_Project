#pragma once

#include "SocketUtil.h"

#define MAXBUFFERSIZE 32

enum class SockType
{
	TCP,
	UDP
};

enum class IOType
{
	READ,
	WRITE,
	ACCEPT,
	CONNECT,
	DISCONNECT
};

typedef struct
{
	WSAOVERLAPPED m_wsaOverlapped;
	WSABUF m_wsaBuf;
	char m_buffer[MAXBUFFERSIZE];
	IOType m_ioType;
	unsigned short m_id;

} OverlappedStruct, *OverlappedStructPtr;

class Socket
{
public:
	Socket() {};
	Socket(SockType sockType);
	~Socket();

	void SetSockOpt(const int level, const int optname, const int optval) const;
	void GetSockOpt(const int level, const int optname) const;
	void Bind(const EndPoint& endPoint);
	void Listen(const int backLog) const;
	void Connect(const EndPoint& endPoint);

	/*void Send();
	void Receive();*/

	void OverlapAcceptEx(const Socket& listenSock);

	void OverlapAcceptEx(const Socket* const clientSock) const;
	void UpdateAcceptContext(const Socket& listenSockPtr) const;
	void OverlapConnectEx(const EndPoint* const endPoint) const;
	bool OverlapDisconnectEx();
	void OverlapWSAsend(const Serializer& se) const;
	void OverlapWSAsend(void* const p) const;
	void OverlapWSArecv();

	void Close();

	SOCKET GetHandle() const;

	mutable unsigned short m_id;

	bool m_isOverlapped = false;

	mutable DWORD m_flag = 0;

	mutable OverlappedStruct	m_overlappedStruct;

	SOCKET m_handle;

	EndPoint ePoint; 
	
	//char m_addrBuffer[100];

private:

	mutable LPFN_ACCEPTEX		AcceptEx;
	mutable LPFN_CONNECTEX		ConnectEx;
	mutable LPFN_DISCONNECTEX	DisconnectEx;
	
};

