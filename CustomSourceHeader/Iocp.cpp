#include "stdafx.h"
#include "Iocp.h"

Iocp::Iocp(UINT8 threadCount)
{
	m_threadCount = threadCount;
	m_Iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_threadCount);
}

Iocp::~Iocp()
{
	CloseHandle(m_Iocp);
}

void Iocp::Add(const Socket* sockPtr) const
{
	if (!CreateIoCompletionPort((HANDLE)sockPtr->GetHandle(), m_Iocp, (ULONG_PTR)sockPtr, m_threadCount))
	{
		throw Exception("IOCP ADD FAILED");
	}
}

void Iocp::Wait(IocpEvents& output, UINT16 timeoutMs)
{
	BOOL r = GetQueuedCompletionStatusEx(m_Iocp, output.m_events, MaxEventCount, (ULONG*)&output.m_eventCount, timeoutMs, FALSE);
	if (!r)
	{
		output.m_eventCount = 0;
	}
}

