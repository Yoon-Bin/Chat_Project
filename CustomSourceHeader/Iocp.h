#pragma once

#include "Socket.h"

#define MAXEVENTCOUNT 1000

class IocpEvents
{
public:
	// GetQueuedCompletionStatus으로 꺼내온 이벤트들
	OVERLAPPED_ENTRY m_events[MAXEVENTCOUNT];
	UINT16 m_eventCount = 0;
};

class Iocp
{
public:
	static const UINT16 MaxEventCount = MAXEVENTCOUNT;

	Iocp(UINT8 threadCount);
	~Iocp();

	void Add(const Socket* sockPtr) const;

	void Wait(IocpEvents& output, UINT16 timeoutMs);

	HANDLE m_Iocp;
private:
	UINT8 m_threadCount = 0;

	
};


