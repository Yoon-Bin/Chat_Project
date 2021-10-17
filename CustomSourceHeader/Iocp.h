#pragma once

#include "Socket.h"

class IocpEvents
{
public:
	// GetQueuedCompletionStatus���� ������ �̺�Ʈ��
	OVERLAPPED_ENTRY m_events[1000];
	UINT16 m_eventCount = 0;
};

class Iocp
{
public:
	static const UINT16 MaxEventCount = 1000;

	Iocp(UINT8 threadCount);
	~Iocp();

	void Add(const Socket& socket, LPCVOID userPtr) const;

	void Wait(IocpEvents& output, UINT16 timeoutMs);

	HANDLE m_Iocp;
private:
	UINT8 m_threadCount = 0;

	
};


