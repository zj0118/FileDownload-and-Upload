#include "Thread.h"
#include <functional>

CThread::CThread()
{

}

CThread::~CThread()
{

}

void CThread::Start()
{
	if (!m_spThread)
		m_spThread.reset(new std::thread(std::bind(&CThread::ThreadProc, this)));
}

void CThread::Join()
{
	if (m_spThread && m_spThread->joinable())
		m_spThread->join();
}

void CThread::ThreadProc()
{
	Run();
}
