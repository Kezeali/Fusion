
#include "FusionNetworkPacketQueue.h"

namespace FusionEngine
{

	PacketQueue::FusionNetworkMessageQueue()
	{
		m_Mutex = CL_Mutex::create();
	}

	PacketQueue::~FusionNetworkMessageQueue()
	{
		ClearMessages();

		delete m_Mutex;
	}

	void PacketQueue::Resize(unsigned int channels)
	{
		m_Channels.resize(channels);
	}

	////////////////
	// Add a message
	void PacketQueue::PushMessage(FusionMessage *message, char channel)
	{
		m_Mutex->enter();

		m_Channels[channel].push_back(message);

		m_Mutex->notify();
		m_Mutex->leave();
	}

	////////////////
	// Get a message
	Packet *PacketQueue::PopMessage(char channel)
	{
		m_Mutex->enter();

		// Make sure the channel exists first
		cl_assert(m_Channels.size() > channel);

		if (m_Channels[channel].empty())
			return 0;

		Packet *m = m_Channels[channel].front();
		m_Channels[channel].pop_front();
		return m;

		m_Mutex->notify();
		m_Mutex->leave();
	}

	////////////////
	// Clear messages
	void PacketQueue::ClearMessages()
	{
		// Each channel
		ChannelList::iterator ch_it = m_Channels.begin();
		for (; ch_it != m_Channels.end(); ++ch_it)
		{
			// Each Message
			MessageQueue::iterator m_it = (*ch_it).begin();
			for (; m_it != (*ch_it).end(); ++m_it)
			{
				delete (*m_it);
			}
		}

		m_Channels.clear();
	}

}
