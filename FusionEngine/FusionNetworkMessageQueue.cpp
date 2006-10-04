
#include "FusionNetworkMessageQueue.h"

using namespace FusionEngine;

FusionNetworkMessageQueue::FusionNetworkMessageQueue()
{
	m_Mutex = CL_Mutex::create();
}

MessageQueue *FusionNetworkMessageQueue::_getInMessages(int channel) const
{
	m_Mutex->enter();

	return m_InChannels[channel];

	m_Mutex->notify();
	m_Mutex->leave();
}

void FusionNetworkMessageQueue::_addInMessage(FusionMessage *message, int channel)
{
	m_Mutex->enter();

	m_InChannels[channel].push_back(message);

	m_Mutex->notify();
	m_Mutex->leave();
}

MessageQueue *FusionNetworkMessageQueue::_getOutMessages(int channel)
{
	m_Mutex->enter();

	return m_OutChannels[channel];

	m_Mutex->notify();
	m_Mutex->leave();
}

void FusionNetworkMessageQueue::_addOutMessage(FusionEngine::FusionMessage *message, int channel)
{
	m_Mutex->enter();

	m_OutChannels[channel].push_back(message);

	m_Mutex->notify();
	m_Mutex->leave();
}

FusionMessage *FusionNetworkMessageQueue::_getInMessage(int channel)
{
	m_Mutex->enter();

	return m_InChannels[channel].front();

	m_Mutex->notify();
	m_Mutex->leave();
}

void FusionNetworkMessageQueue::_addEvent(FusionMessage *message)
{
	m_Mutex->enter();

	m_EventList.push_back(message);

	m_Mutex->notify();
	m_Mutex->leave();
}

FusionNetworkMessageQueue::EventList *FusionNetworkMessageQueue::GetEvents()
{
	m_Mutex->enter();

	return m_EventList;

	m_Mutex->notify();
	m_Mutex->leave();
}