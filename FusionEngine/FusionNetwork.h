/*
 Copyright (c) 2007 Fusion Project Team

 This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

 Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software in a
 product, an acknowledgment in the product documentation would be
 appreciated but is not required.

 2. Altered source versions must be plainly marked as such, and must not
 be misrepresented as being the original software.

 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef Header_FusionEngine_NetworkGeneric
#define Header_FusionEngine_NetworkGeneric

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

// Fusion
#include "FusionNetworkTypes.h"

namespace FusionEngine
{

	//! Fusion Packet priority enumeration
	enum NetPriority
	{
		HIGH_PRIORITY = 1,
		MEDIUM_PRIORITY,
		LOW_PRIORITY
	};

	//! Fusion Packet reliability enumeration
	enum NetReliability
	{
		UNRELIABLE,
		UNRELIABLE_SEQUENCED,
		RELIABLE,
		RELIABLE_ORDERED,
		RELIABLE_SEQUENCED
	};

	typedef unsigned int NetTime;

	typedef std::string NetHandle;

	class IPacket
	{
	public:
		//! Returns the packet data (after the header) as a string
		virtual std::string GetDataString() = 0;
		//! Returns the packet data after the header
		virtual const char* GetData() const = 0;
		//! Returns the data length
		virtual unsigned int GetLength() const = 0;
		//! Returns the Message Identifier for the packet
		virtual unsigned char GetType() const = 0;
		//! Returns true if the packet is timestamped
		virtual bool IsTimeStamped() const = 0;
		//! Returns the timestamp value
		virtual NetTime GetTime() const = 0;
		//! Returns the UID for the system that sent this message
		virtual const NetHandle& GetSystemHandle() const = 0;

	};

	//! Default IPacket implementation
	class NetPacket : public IPacket
	{
	public:
		NetHandle m_Origin;
		bool m_TimeStamped;
		NetTime m_Time;
		unsigned char m_Type;
		unsigned char m_Channel;
		unsigned char* m_Data;
		unsigned int m_Length;

	public:
		NetPacket(NetHandle origin, bool timestamped, NetTime time, char type, const char* data, unsigned int length)
			: m_TimeStamped(timestamped),
			m_Time(time),
			m_Type(type),
			m_Length(length),
			m_Origin(origin)
		{
			m_Data = new unsigned char[length];
			memcpy(m_Data, data, length);
		}
		//! Virtual desturctor
		virtual ~NetPacket()
		{
			delete[] m_Data;
		}

		virtual std::string GetDataString()
		{
			return std::string(m_Data, m_Data + m_Length);
		}
		virtual const char* GetData() const
		{
			return (char*) m_Data;
		}
		virtual unsigned int GetLength() const
		{
			return m_Length;
		}
		virtual unsigned char GetType() const
		{
			return m_Type;
		}
		virtual bool IsTimeStamped() const
		{
			return m_TimeStamped;
		}
		virtual NetTime GetTime() const
		{
			return m_Time;
		}
		virtual const NetHandle& GetSystemHandle() const
		{
			return m_Origin;
		}
	};

	/*!
	 * \brief
	 * Network Interface
	 */
	class Network
	{
	public:
		//! Basic constructor.
		Network();

		//! Virtual destructor
		virtual ~Network();

	public:
		//! Starts the network
		virtual bool Startup(unsigned short maxConnections, unsigned short incommingPort, unsigned short maxIncommingConnections = 0) = 0;
		//! Connects to a server
		virtual bool Connect(const std::string &host, unsigned short port) = 0;
		//! Disconnects cleanly
		virtual void Disconnect() = 0;

		//! Sends a packet containing ONLY the given data
		virtual bool SendRaw(char *data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel,
			const NetHandle& destination);

		//! Sends a message
		/*!
		 * Special send wrapper which adds an extra type ID to the packet
		 *
		 * \param[in] timestamped
		 * True if a timestamp should be added
		 *
		 * \param[in] type
		 * The extra type ID
		 *
		 * \param[in] subtype
		 * The type ID
		 *
		 * \param[in] data
		 * The data send
		 *
		 * \param[in] length
		 * The length of the data
		 *
		 */
		bool SendToChannel(bool timestamped, char type, char subtype, char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel, 
			const NetHandle& destination);

		//! Wrapper to deal with RakNet (and it's stupid use of unsigned char*)
		bool Send(bool timestamped, char type, unsigned char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel,
			const NetHandle& destination);

		//! Sends data.
		/*!
		 * The implementation should format and send the message as
		 * described by the given parameters, or as close to the
		 * description as possible.
		 *
		 * \returns
		 * True if the data was sent
		 */
		virtual bool Send(bool timestamped, char type, char* data, unsigned int length,
			NetPriority priority, NetReliability reliability, char channel,
			const NetHandle& destination) = 0;

		//! Gets packets from the network.
		virtual IPacket* Receive() = 0;

		//! Puts the given packet back on the receive buffer
		virtual void PushBackPacket(IPacket* packet, bool toHead = false);
		//! Deletes the given packet
		virtual void DeallocatePacket(IPacket* packet);

		//! Returns the latest ping to the given NetID
		virtual int GetPing(const NetHandle& handle);

		//! Adds some lag time to communications
		/*!
		 * \param minLagMilis
		 * The minimum amount of lag time applied to communications
		 *
		 * \param variance
		 * Extra lag time which may be randomly applied
		 */
		virtual void SetDebugLag(unsigned int minLagMilis, unsigned int variance) {}
		//! Drops data as it exceeds the given amount
		/*!
		 * \param allowBps
		 * Maximum bits per second before packet loss
		 */
		virtual void SetDebugPacketLoss(double allowBps) {}

		//! Returns the current fake lag setting
		virtual unsigned int GetDebugLagMin() const { return 0; }
		//! Returns the current fake lag variance setting
		virtual unsigned int GetDebugLagVariance() const { return 0; }
		//! Returns the current fake packet loss setting
		virtual double GetDebugAllowBps() const { return 0.0; }
	};

}

#endif
