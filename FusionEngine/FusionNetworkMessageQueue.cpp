
#include "FusionNetworkMessageQueue.h"

using namespace FusionEngine;

FusionNetworkMessageQueue::FusionNetworkMessageQueue()
{
	m_Mutex = CL_Mutex::create();
}

FusionNetworkMessageQueue::~FusionNetworkMessageQueue()
{
	ClearEvents();
	ClearInMessages();
	ClearOutMessages();

	delete m_Mutex;
}

//! Converts the channel given in a packet to a channel index within the channel list.
inline int NetToLocalChan(int channel)
{
	// The conversion is only relavant to channels above 1
	return (channel == 1) ? 1 : channel - (channel / 2 - 1);
}

void FusionNetworkMessageQueue::Resize(unsigned short channels)
{
	m_InChannels.reserve(channels);
	m_OutChannels.reserve(channels);
}

///////////////////
// Get all Messages
FusionNetworkMessageQueue::MessageQueue *FusionNetworkMessageQueue::_getInMessages(int channel)
{
	m_Mutex->enter();

	return &m_InChannels[NetToLocalChan(channel)];

	m_Mutex->notify();
	m_Mutex->leave();
}

FusionNetworkMessageQueue::MessageQueue *FusionNetworkMessageQueue::_getOutMessages(int channel)
{
	m_Mutex->enter();

	return &m_OutChannels[NetToLocalChan(channel)];

	m_Mutex->notify();
	m_Mutex->leave();
}

////////////////
// Add a message
void FusionNetworkMessageQueue::_addInMessage(FusionMessage *message, int channel)
{
	m_Mutex->enter();

	m_InChannels[NetToLocalChan(channel)].push_back(message);

	m_Mutex->notify();
	m_Mutex->leave();
}

void FusionNetworkMessageQueue::_addOutMessage(FusionEngine::FusionMessage *message, int channel)
{
	m_Mutex->enter();

	m_OutChannels[NetToLocalChan(channel)].push_back(message);

	m_Mutex->notify();
	m_Mutex->leave();
}

////////////////
// Get a message
FusionMessage *FusionNetworkMessageQueue::_getInMessage(int channel)
{
	m_Mutex->enter();

	int lchan = NetToLocalChan(channel);

	// Make sure the channel exists first
	assert(m_OutChannels.size() > lchan);

	if (m_InChannels[lchan].empty())
		return 0;

	FusionMessage *m = m_InChannels[lchan].front();
	m_InChannels[NetToLocalChan(channel)].pop_front();
	return m;

	m_Mutex->notify();
	m_Mutex->leave();
}

FusionMessage *FusionNetworkMessageQueue::_getOutMessage(int channel)
{
	m_Mutex->enter();

	assert(m_OutChannels.size() > channel);

	if (m_OutChannels[channel].empty())
		return 0;

	FusionMessage *m = m_OutChannels[channel].front();
	m_OutChannels[channel].pop_front();
	return m;

	m_Mutex->notify();
	m_Mutex->leave();
}

/////////
// Events
void FusionNetworkMessageQueue::_addEvent(FusionMessage *message)
{
	m_Mutex->enter();

	m_EventList.push_back(message);

	m_Mutex->notify();
	m_Mutex->leave();
}

FusionMessage *FusionNetworkMessageQueue::GetEvent()
{
	m_Mutex->enter();

	if (m_EventList.empty())
		return 0;

	FusionMessage *m = m_EventList.back();
	m_EventList.pop_back();
	return m;

	m_Mutex->notify();
	m_Mutex->leave();
}

const FusionNetworkMessageQueue::EventList FusionNetworkMessageQueue::GetEvents()
{
	m_Mutex->enter();

	return m_EventList;

	m_Mutex->notify();
	m_Mutex->leave();
}

void FusionNetworkMessageQueue::ClearEvents()
{
	EventList::iterator it = m_EventList.begin();

	for (; it != m_EventList.end(); ++it)
	{
		delete (*it);
	}

	m_EventList.clear();
}

void FusionNetworkMessageQueue::ClearInMessages()
{
	// Each in channel
	ChannelList::iterator ch_it = m_InChannels.begin();
	for (; ch_it != m_InChannels.end(); ++ch_it)
	{
		// Each Message
		MessageQueue::iterator m_it = (*ch_it).begin();
		for (; m_it != (*ch_it).end(); ++m_it)
		{
			delete (*m_it);
		}
	}

	m_InChannels.clear();
}

void FusionNetworkMessageQueue::ClearOutMessages()
{
	// Each out channel
	ChannelList::iterator ch_it = m_OutChannels.begin();
	for (; ch_it != m_OutChannels.end(); ++ch_it)
	{
		// Each Message
		MessageQueue::iterator m_it = (*ch_it).begin();
		for (; m_it != (*ch_it).end(); ++m_it)
		{
			delete (*m_it);
		}
	}

	m_OutChannels.clear();
}