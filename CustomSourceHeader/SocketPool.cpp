#include "stdafx.h"
#include "SocketPool.h"

SocketPool::SocketPool(int sockCount)
{
	for (int i = 0; i < sockCount; i++)
	{
		m_fullSockPtrMap.emplace_back(std::make_unique<Socket>(SockType::TCP));

		m_fullSockPtrMap[i]->m_overlappedStruct.m_id = i;
	}
}

SocketPool::~SocketPool()
{

}

size_t SocketPool::GetFullSockCount() const
{
	return m_fullSockPtrMap.size();
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