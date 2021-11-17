#include "stdafx.h"
#include "SocketPool.h"

SocketPool::SocketPool(int sockCount, bool isBinded, SockType protocol)
{
	for (int i = 0; i < sockCount; i++)
	{
		m_fullSockPtrVector.emplace_back(std::make_unique<Socket>(protocol));

		m_fullSockPtrVector[i]->m_overlappedStruct.m_id = i;
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

//size_t SocketPool::GetUsableSockCount() const
//{
//	return m_usableSockPtrStack.size();
//}
//
//void SocketPool::PushUsableSockPtr(Socket* sockPtr)
//{
//	m_usableSockPtrStack.push(sockPtr);
//}
//
//void SocketPool::PopUsableSockPtr()
//{
//	m_usableSockPtrStack.pop();
//}
//
//Socket* SocketPool::GetUsableSockPtr()
//{
//	Socket* sockPtr = m_usableSockPtrStack.front();
//
//	return sockPtr;
//}