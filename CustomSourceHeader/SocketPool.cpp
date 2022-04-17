#include "stdafx.h"
#include "SocketPool.h"

SocketPool::SocketPool(int sockCount, bool isBinded, SockType protocol)
{
	for (int i = 0; i < sockCount; i++)
	{
		m_fullSockPtrVector.emplace_back(std::make_unique<Socket>(protocol));

		m_fullSockPtrVector[i]->m_overlappedStruct.m_id = i;
		m_fullSockPtrVector[i]->m_id = i;
	}

	switch (isBinded)
	{
	case true:
		for (auto& sock : m_fullSockPtrVector)
		{
			sock->Bind(EndPoint::Any);
		}
		break;
	case false:
		break;
	}

	
}

SocketPool::~SocketPool()
{

}

size_t SocketPool::GetFullSockCount() const
{
	return m_fullSockPtrVector.size();
}
