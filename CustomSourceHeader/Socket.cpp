#include "stdafx.h"
#include "Socket.h"

Socket::Socket(SockType socketType)
{
	if (socketType == SockType::TCP)
	{
		m_handle = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	else
	{
		m_handle = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}

	RtlZeroMemory(&m_overlappedStruct.m_wsaOverlapped, sizeof(m_overlappedStruct.m_wsaOverlapped));
	//RtlZeroMemory(&m_receiveBuffer, sizeof(m_receiveBuffer));
}

Socket::~Socket()
{
	printf("%s\n", "¼ÒÄÏ ¼Ò¸êÀÚ");
	closesocket(m_handle);
}

void Socket::SetSockOpt(const int level, const int optname, const int optval) const
{
	if (setsockopt(m_handle, level, optname, (char*)&optval, sizeof(optval)) < 0)
	{
		std::stringstream ss;

		ss << "SetSockOpt Error : " << GetSockOptNameAsString(optname).c_str() << "\n" << GetLastErrorAsString();

		throw Exception(ss.str().c_str());
	}

}

void Socket::GetSockOpt(const int level, const int optname) const
{
	bool optval;
	int optlen = sizeof(optval);

	if (getsockopt(m_handle, level, optname, (char*)&optval, &optlen) < 0)
	{
		std::stringstream ss;

		throw Exception(GetLastErrorAsString().c_str());
	}
	std::cout << "option : " << GetSockOptNameAsString(optname).c_str() << ",\t value : " << optval << std::endl;

}

void Socket::Bind(const EndPoint& endPoint)
{
	if (bind(m_handle, (sockaddr*)&endPoint.m_sockAddrIn, sizeof(endPoint.m_sockAddrIn)) < 0)
	{
		std::stringstream ss;

		ss << "Bind Fail : " << GetLastErrorAsString();

		throw Exception(ss.str().c_str());
	}
	else
	{
		printf(("Bind OK\n"));
	}
}

void Socket::Listen(const int backLog) const
{
	if (listen(m_handle, backLog) < 0)
	{
		std::stringstream ss;

		ss << "Listen Fail : " << GetLastErrorAsString();

		throw Exception(ss.str().c_str());
	}
	else
	{
		printf(("Listen OK\n"));
	}
}


void Socket::OverlapAcceptEx(const Socket& listenSock)
{
	DWORD bytes;
	UUID uuid = WSAID_ACCEPTEX;

	m_overlappedStruct.m_ioType = IOType::ACCEPT;

	if (AcceptEx == NULL)
	{
		WSAIoctl(
			m_handle,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&uuid,
			sizeof(uuid),
			&AcceptEx,
			sizeof(AcceptEx),
			&bytes,
			NULL,
			NULL
		);

		if (AcceptEx == NULL)
		{
			throw Exception("AcceptEx Fail");
		}
	}

	char TestBuffer[100];
	DWORD ignored = 0;

	bool result = AcceptEx(
		listenSock.m_handle,
		m_handle,
		&TestBuffer,
		0,
		50,
		50,
		&ignored,
		&m_overlappedStruct.m_wsaOverlapped
	);

	if (!result && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;

		ss << "OverlapAccept Error : " << GetLastErrorAsString().c_str();

		throw Exception(ss.str().c_str());
	}
}


void Socket::OverlapAcceptEx(const Socket* const clientSock) const
{
	DWORD bytes;
	UUID uuid = WSAID_ACCEPTEX;

	clientSock->m_overlappedStruct.m_ioType = IOType::ACCEPT;

	if (AcceptEx == NULL)
	{
		WSAIoctl(
			m_handle,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&uuid,
			sizeof(uuid),
			&AcceptEx,
			sizeof(AcceptEx),
			&bytes,
			NULL,
			NULL
		);

		if (AcceptEx == NULL)
		{
			throw Exception("AcceptEx Fail");
		}
	}
	char TestBuffer[100];
	bool result = AcceptEx(
		m_handle,
		clientSock->m_handle,
		&TestBuffer,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		NULL,
		&clientSock->m_overlappedStruct.m_wsaOverlapped
	);

	if (!result && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;

		ss << "OverlapAccept Error : " << GetLastErrorAsString().c_str();

		throw Exception(ss.str().c_str());
	}
}

void Socket::UpdateAcceptContext(const Socket& listenSockPtr) const
{
	sockaddr_in* ignore1;
	sockaddr_in* ignore3;
	int ignore2 = 0;
	int ignore4 = 0;

	char ignore[3000] = { 0, };

	char address[128] = { 0, };

	//RtlZeroMemory(&ignore, sizeof(ignore));

	GetAcceptExSockaddrs(
		&ignore,
		NULL,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		(sockaddr**)&ignore1,
		&ignore2,
		(sockaddr**)&ignore3,
		&ignore4
	);

	/*char strArr[20];
	inet_ntop(AF_INET,&(ignore1->sin_addr),strArr,20);

	printf("%s\n", strArr);*/

	/*inet_ntop(AF_INET, &ignore3->sin_addr.S_un.S_addr, address, sizeof(address));

	printf("%s\n", &address);*/

	int result = setsockopt(
		m_handle,
		SOL_SOCKET,
		SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listenSockPtr.m_handle,
		sizeof(listenSockPtr.m_handle)
	);

	if (result != 0)
	{
		std::stringstream ss;

		ss << "UpdateAcceptContext Failed : " << GetLastErrorAsString();

		throw Exception(ss.str().c_str());
	}

}

void Socket::OverlapConnectEx(const EndPoint* const endPoint) const
{
	DWORD bytes;
	UUID uuid = WSAID_CONNECTEX;
	
	m_overlappedStruct.m_ioType = IOType::CONNECT;

	if (ConnectEx == NULL)
	{
		WSAIoctl(
			m_handle,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&uuid,
			sizeof(uuid),
			&ConnectEx,
			sizeof(ConnectEx),
			&bytes,
			NULL,
			NULL
		);
	}

	bool result = ConnectEx(
		m_handle,
		(sockaddr*)&endPoint->m_sockAddrIn,
		sizeof(endPoint->m_sockAddrIn),
		NULL,
		NULL,
		NULL,
		&m_overlappedStruct.m_wsaOverlapped
	);

	if (result != 0 && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;

		ss << "OverlapConnect Error : " << GetLastErrorAsString().c_str();

		throw Exception(ss.str().c_str());
	}
}

bool Socket::OverlapDisconnectEx()
{
	DWORD bytes;
	UUID uuid = WSAID_DISCONNECTEX;

	m_overlappedStruct.m_ioType = IOType::DISCONNECT;

	if (DisconnectEx == NULL)
	{
		WSAIoctl(
			m_handle,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&uuid,
			sizeof(uuid),
			&DisconnectEx,
			sizeof(DisconnectEx),
			&bytes,
			NULL,
			NULL
		);
	}

	bool result = DisconnectEx(
		m_handle,
		&m_overlappedStruct.m_wsaOverlapped,
		TF_REUSE_SOCKET,
		0
	);

	if (!result && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;

		ss << "DisconnectEx Error : " << GetLastErrorAsString().c_str();

		throw Exception(ss.str().c_str());
	}

	return result;
}

void Socket::Connect(const EndPoint& endPoint)
{
	if (connect(m_handle, (sockaddr*)&endPoint.m_sockAddrIn, sizeof(endPoint.m_sockAddrIn)) < 0)
	{
		std::stringstream ss;

		ss << "Connect Fail : " << GetLastErrorAsString();

		throw Exception(ss.str().c_str());
	}
}

void Socket::OverlapWSAsend(void* const p) const
{
	m_flag = 0;

	char* buf = reinterpret_cast<char*>(p);

	OverlappedStruct* ovlpStruct = new OverlappedStruct;

	RtlZeroMemory(&ovlpStruct->m_wsaOverlapped, sizeof(ovlpStruct->m_wsaOverlapped));

	ovlpStruct->m_ioType = IOType::WRITE;
	ovlpStruct->m_wsaBuf.buf = buf;
	ovlpStruct->m_wsaBuf.len = static_cast<char>(buf[0]);

	int result = WSASend(
		m_handle,
		&ovlpStruct->m_wsaBuf,
		1,
		NULL,
		m_flag,
		&ovlpStruct->m_wsaOverlapped,
		NULL
	);

	if(result != 0 && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;

		ss << "WSASend Failed : " << GetLastErrorAsString().c_str();

		throw Exception(ss.str().c_str());
	}
}

void Socket::OverlapWSArecv()
{
	m_flag = 0;

	m_overlappedStruct.m_ioType = IOType::READ;
	m_overlappedStruct.m_wsaBuf.buf = &m_overlappedStruct.m_buffer[0];
	m_overlappedStruct.m_wsaBuf.len = MAXBUFFERSIZE;

	int result = WSARecv(
		m_handle,
		&m_overlappedStruct.m_wsaBuf,
		1,
		NULL,
		&m_flag,
		&m_overlappedStruct.m_wsaOverlapped,
		NULL
	);

	if(result != 0 && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;

		ss << "WSARecv Failed : " << GetLastErrorAsString().c_str();

		throw Exception(ss.str().c_str());
	}
}

void Socket::Close()
{
	closesocket(m_handle);
}

SOCKET Socket::GetHandle() const
{
	return m_handle;
}
