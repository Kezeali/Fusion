
#include "FusionNetworkPacketQueue.h"

namespace FusionEngine
{

	PacketQueue::PacketQueue()
	{
	}

	PacketQueue::~PacketQueue()
	{
		ClearMessages();
	}

	void PacketQueue::Resize(unsigned int channels)
	{
		m_Channels.resize(channels);
	}

	////////////////
	// Add a message
	void PacketQueue::PushMessage(Packet *message, char channel)
	{
		m_Mutex.lock();

		m_Channels[channel].push_back(message);

		m_Mutex.unlock();
	}

	////////////////
	// Get a message
	Packet *PacketQueue::PopMessage(char channel)
	{
		m_Mutex.lock();

		// Make sure the channel exists first
		FSN_ASSERT(m_Channels.size() > channel);

		if (m_Channels[channel].empty())
			return 0;

		Packet *m = m_Channels[channel].front();
		m_Channels[channel].pop_front();
		return m;

		m_Mutex.unlock();
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
