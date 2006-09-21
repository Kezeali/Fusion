
#include "FusionNetworkMessageQueue.h"

using namespace FusionEngine;

FusionNetworkMessageQueue::FusionNetworkMessageQueue()
{
	m_Mutex = CL_Mutex::create();
}

const MessageQueue &FusionNetworkMessageQueue::_getInMessages(int channel) const
{
	m_Mutex->enter();
	return m_InChannels[channel];

	m_Mutex->notify();
	m_Mutex->leave();
}

void FusionNetworkMessageQueue::_addInMessage(FusionMessage *message, int channel)
{
	m_Mutex->enter();
	m_InChannels[channel].push(message);

	m_Mutex->notify();
	m_Mutex->leave();
}

const MessageQueue &FusionNetworkMessageQueue::_getOutMessages(int channel)
{
	m_Mutex->enter();
	return m_OutChannels[channel];

	m_Mutex->notify();
	m_Mutex->leave();
}

void FusionNetworkMessageQueue::_addOutMessage(FusionEngine::FusionMessage *message,
																							 int channel)
{
	m_Mutex->enter();
	m_OutChannels[channel].push(message);

	m_Mutex->notify();
	m_Mutex->leave();
}