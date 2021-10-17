#include "stdafx.h"
#include "Socket.h"
//extern unordered_map<int, const char*> sockOptTable;

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
	RtlZeroMemory(&m_receiveBuffer, sizeof(m_receiveBuffer));
}

Socket::~Socket()
{
	closesocket(m_handle);
}

void Socket::SetSockOpt(int level, int optname, int optval)
{
	if (setsockopt(m_handle, level, optname, (char*)&optval, sizeof(optval)) < 0)
	{
		std::stringstream ss;

		ss << "SetSockOpt Error : " << GetSockOptNameAsString(optname).c_str() << "\n" << GetLastErrorAsString();

		throw Exception(ss.str().c_str());
	}

}

void Socket::GetSockOpt(int level, int optname)
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

void Socket::Listen(int backLog)
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


bool Socket::OverlapAcceptEx(Socket* clientSock)
{
	DWORD bytes;
	UUID uuid = WSAID_ACCEPTEX;

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
	bool ret = AcceptEx(
		m_handle,
		clientSock->GetHandle(),
		&TestBuffer,
		0,
		50,
		50,
		&ignored,
		&m_overlappedStruct.m_wsaOverlapped
	);

	if (!ret && WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;

		ss << "OvelapAccept Error : " << GetLastErrorAsString().c_str();

		throw Exception(ss.str().c_str());
	}

	return ret;
}

bool Socket::OverlapConnectEx(Socket& clientSock, EndPoint& endPoint)
{
	DWORD bytes;
	UUID uuid = WSAID_CONNECTEX;
	
	if (ConnectEx == NULL)
	{
		WSAIoctl(
			clientSock.GetHandle(),
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

	ConnectEx(
		clientSock.GetHandle(),
		(sockaddr*)&endPoint.m_sockAddrIn,
		sizeof(endPoint.m_sockAddrIn),
		NULL,
		NULL,
		NULL,
		&m_overlappedStruct.m_wsaOverlapped
	);
	return 0;
}

bool Socket::OverlapDisconnectEx(Socket& sock)
{
	DWORD bytes;
	UUID uuid = WSAID_DISCONNECTEX;

	if (DisconnectEx == NULL)
	{
		WSAIoctl(
			sock.GetHandle(),
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

	DisconnectEx(
		sock.GetHandle(),
		&sock.m_overlappedStruct.m_wsaOverlapped,
		TF_REUSE_SOCKET,
		0
	);

	return 0;
}

int Socket::UpdateAcceptContext(Socket& listenSock)
{
	sockaddr_in *ignore1;
	sockaddr_in *ignore3;
	int ignore2, ignore4;

	char ignore[3000];

	RtlZeroMemory(&ignore, sizeof(ignore));

	GetAcceptExSockaddrs(ignore,
		0,
		sizeof(sockaddr_in)+16,
		sizeof(sockaddr_in) + 16,
		(sockaddr**)&ignore1,
		&ignore2,
		(sockaddr**)&ignore3,
		&ignore4);

	int ret = setsockopt(
		m_handle,
		SOL_SOCKET,
		SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listenSock.m_handle,
		sizeof(listenSock.m_handle)
	);

	if (ret != 0)
	{
		std::stringstream ss;

		ss << "UpdateAcceptContext Failed : " << GetLastErrorAsString();

		throw Exception(ss.str().c_str());
	}

	return ret;
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

void Socket::OverlapWSAsend(Socket sock)
{
	m_flag = 0;

	m_overlappedStruct.m_ioType = IOType::WRITE;
	m_overlappedStruct.m_wsaBuf.buf = m_receiveBuffer;
	m_overlappedStruct.m_wsaBuf.len = MAXBUFFERSIZE;

	if(WSASend(
		sock.m_handle,
		&m_overlappedStruct.m_wsaBuf,
		1,
		NULL,
		m_flag,
		&m_overlappedStruct.m_wsaOverlapped,
		NULL
	) != 0 && WSAGetLastError() != ERROR_IO_PENDING)
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
	m_overlappedStruct.m_wsaBuf.buf = m_receiveBuffer;
	m_overlappedStruct.m_wsaBuf.len = MAXBUFFERSIZE;

	if (WSARecv(
		m_handle,
		&m_overlappedStruct.m_wsaBuf,
		1,
		NULL,
		&m_flag,
		&m_overlappedStruct.m_wsaOverlapped,
		NULL
	)!=0&&WSAGetLastError() != ERROR_IO_PENDING)
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
