#pragma once

class EndPoint
{
public:
	EndPoint();
	EndPoint(LPCSTR address, uint16_t port);
	~EndPoint();

	std::string ToString();

	static EndPoint Any;

	sockaddr_in m_sockAddrIn;
	std::string m_addrString;
};

