#include "stdafx.h"
#include "EndPoint.h"

EndPoint::EndPoint()
{
	RtlZeroMemory(&m_sockAddrIn, sizeof(m_sockAddrIn));
	m_sockAddrIn.sin_family = AF_INET;
}

EndPoint::EndPoint(LPCSTR address, uint16_t port)
{
	RtlZeroMemory(&m_sockAddrIn, sizeof(m_sockAddrIn));

	m_sockAddrIn.sin_family = AF_INET;

	if (inet_pton(AF_INET, address, &m_sockAddrIn.sin_addr) != 1)
	{
		throw Exception("inet_pton Fail");
	}
	m_sockAddrIn.sin_port = htons(port);

	m_addrString = address;
	
	m_addrString.append(", ").append(std::to_string(port));
}

EndPoint::~EndPoint()
{

}

EndPoint EndPoint::Any;

std::string EndPoint::ToString()
{
	return m_addrString;
}
