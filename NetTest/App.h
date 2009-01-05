#ifndef APP_H
#define APP_H
#if _MSC_VER > 1000
#pragma once
#endif

#include "../FusionEngine/FusionCommon.h"
#include "../FusionEngine/FusionNetwork.h"
#include "../FusionEngine/FusionRakNetwork.h"
#include "../FusionEngine/FusionRakStream.h"
#include "../FusionEngine/FusionHistory.h"

#include <RakNet/BitStream.h>
#include <RakNet/GetTime.h>

#include "Ship.h"
#include "AuthorityManager.h"

using namespace FusionEngine;

const unsigned int s_HighSendInterval = 1000/10;
const unsigned int s_LowSendInterval = 1000/30;
const unsigned int s_PacketSize = 1000; // 1000kBytes

// Simulation speed
const unsigned int split = 33;

const float s_Speed = 0.05f;


class ClientData
{
public:
	ClientData()
		: m_SendInterval(s_LowSendInterval),
		m_TimeTillSendAllowed(s_LowSendInterval),
		m_AuthorityId(0),
		m_MostRecentTick(0)
	{
	}

	void SetTimeTillSend(float tts);
	void SetSystemHandle(const NetHandle& systemHandle);
	void SetAuthorityId(ObjectID authId);
	void SetMostRecentTick(unsigned long tick);

	void UpdateTimeTillSend(float timePassed);
	bool IsSendAllowed();
	void ResetTimeTillSend();
	const NetHandle& GetSystemHandle();
	ObjectID GetAuthorityId();
	unsigned long GetMostRecentTick();

protected:
	float m_SendInterval;
	float m_TimeTillSendAllowed;
	NetHandle m_SystemHandle;
	ObjectID m_AuthorityId;

	unsigned long m_MostRecentTick;
};

void ClientData::SetTimeTillSend(float tts)
{
	m_TimeTillSendAllowed = tts;
}

void ClientData::SetSystemHandle(const NetHandle& systemHandle)
{
	m_SystemHandle = systemHandle;
}

void ClientData::SetAuthorityId(ObjectID authId)
{
	m_AuthorityId = authId;
}

void ClientData::SetMostRecentTick(unsigned long tick)
{
	m_MostRecentTick = fe_max(tick, m_MostRecentTick);
}

unsigned long ClientData::GetMostRecentTick()
{
	return m_MostRecentTick;
}

void ClientData::UpdateTimeTillSend(float timePassed)
{
	m_TimeTillSendAllowed -= timePassed;
}

bool ClientData::IsSendAllowed()
{
	return m_TimeTillSendAllowed < 0;
}

void ClientData::ResetTimeTillSend()
{
	m_TimeTillSendAllowed = m_SendInterval;
}

const NetHandle& ClientData::GetSystemHandle()
{
	return m_SystemHandle;
}

ObjectID ClientData::GetAuthorityId()
{
	return m_AuthorityId;
}


class TestApp
{
private:
	// TODO: (on client) use (unsigned long)m_Tick instead of m_CommandNumber
	unsigned long m_CommandNumber;

	// Last tick received from server (used to compute how much more adjustment needs to be done)
	unsigned long m_LatestReceivedTick;
	// floor(tick) - m_LastReceivedTick is compared to m_TicksAhead and a smoothed speed is calculated for SimAdjust
	float m_Tick; // Client side
	float m_LastTick; // Used to compute DT
	// How far ahead we should be from the last received tick so that
	//  our packets will arrive at the server /slightly before/ it needs them
	//  ---
	//  Recomputed every time the a packet is received from the server and incremented
	//  every normal tick (ie. one based on the time-in-seconds delta)
	//  --
	//  Recomputation:
	//  Update Exp Smoothed Avg of ping (ESAping)
	//  Update mean jitter -> mean of jitter in last 10 seconds, jitter = lastping - ping
	//  ticksAhead = ESAping + mean jitter -> not sure about this. what needs to be done
	//        here is to bias the ESAping with the jitter. Perhaps the jitter could be used to adjust a smoothing algo
	//  ticksAhead = ticksAhead / tickTime (aka. split) -> 'normalize' ticksAhead
	unsigned long m_TicksAhead;
	float m_SimAdjust;

	long m_SendDelay;

	float m_Tightness;

	float m_WorldWidth, m_WorldHeight;

	typedef std::tr1::shared_ptr<Ship> ShipPtr;
	// For server side (what host owns the given object)
	typedef std::tr1::unordered_map<ObjectID, NetHandle> EntityOwnerMap;
	typedef std::tr1::unordered_multimap<NetHandle, ObjectID> ClientOwnershipMap;
	
	typedef std::tr1::unordered_map<NetHandle, ClientData> ClientMap;

	typedef boost::circular_buffer<std::pair<unsigned long, Action>> ActionBuffer;

	// For both client and server
	typedef std::tr1::unordered_map<ObjectID, ShipPtr> ShipMap;

	struct SendPriority
	{
		bool operator()(const ShipMap::value_type& left, const ShipMap::value_type& right) const
		{
			return left.second->lastSentTick < right.second->lastSentTick;
		}
	};

	Network* m_Network;

	CL_DisplayWindow m_window;
	CL_InputContext m_ic;
	CL_GraphicContext m_gc;

	AuthorityManager m_AuthorityManager;

	ClientMap m_Clients;

	CL_Font_Bitmap font1;
	CL_Font_Bitmap font1small;

	ObjectID nextId;
	ObjectID nextAuthId;

	// Server specific data
	EntityOwnerMap entityOwnership;
	ClientOwnershipMap clientEntities;
	ActionBuffer sentActions;

	// Client specific data
	NetHandle serverHandle;
	ShipMap ships;
	ShipMap myShips;
	CL_Sprite shipSprite;
	CL_SpriteRenderBatch m_SpriteBatch;

	// Used to calculate recent average jitter
	boost::circular_buffer<int> m_PingBuffer;

	CL_SlotContainer m_Slots;


	void get_input(const ShipPtr myShip);

	inline bool is_local(const ShipPtr ship, const ShipMap myShips)
	{
		return ship->local;
		//return myShips.find(ship->id) != myShips.end();
	}

	// Correct prediction errors
	void correct(const ShipPtr ship, unsigned long commandNumber, const Action& correctAction);

	void run_server(unsigned short maxPlayers);
	void run_client(const std::string& host);

	// These methods could be broken out into a seperate class
	void server_receive();
	void server_simulate(float dt);
	void server_updateAuthority(float dt);
	void server_send(float dt);
	void server_render(float dt);

	void client_receive();
	void client_simulate(long dt);
	void client_updateAuthority(float dt);
	void client_send(float dt);
	void client_render(float tickTime, float accumulatorRemaining);

	void draw_rectangle(const CL_Rectf &rect, const CL_Colorf &color);

	void key_down(const CL_InputEvent& ev, const CL_InputState& state);
	void key_up(const CL_InputEvent& ev, const CL_InputState& state);

public:
	int run(const std::vector<CL_String> &args, CL_DisplayWindow& display);

};


void TestApp::get_input(const TestApp::ShipPtr myShip)
{
	if (m_ic.get_keyboard().get_keycode(CL_KEY_UP))
		myShip->up = true;
	else
		myShip->up = false;

	if (m_ic.get_keyboard().get_keycode(CL_KEY_DOWN))
		myShip->down = true;
	else
		myShip->down = false;

	if (m_ic.get_keyboard().get_keycode(CL_KEY_LEFT))
		myShip->left = true;
	else
		myShip->left = false;

	if (m_ic.get_keyboard().get_keycode(CL_KEY_RIGHT))
		myShip->right = true;
	else
		myShip->right = false;
}

int TestApp::run(const std::vector<CL_String> &args, CL_DisplayWindow& display)
{
	m_window = display;
	m_ic = display.get_ic();
	m_gc = display.get_gc();

	bool client = false, server = false;
	int maxPlayers = 16;
	std::string host = "127.0.0.1";

	{
		CL_Rect vp = display.get_viewport();
		m_WorldWidth = (float)vp.get_width();
		m_WorldHeight = (float)vp.get_height();
	}

	for (std::vector<CL_String>::const_iterator it = args.begin() + 1, end = args.end(); it != end; ++it)
	{
		CL_String arg = *it, val;
		if (arg.length() < 3)
			continue;
		switch (arg[1]) 
		{
		case 'c':
			client = true;
			val = arg.substr(2);
			host = std::string(val.begin(), val.end());
			break;
		case 's':
			server = true;
			maxPlayers = CL_StringHelp::text_to_int(arg.substr(2));
			break;

		default:
			CL_Console::write_line("Unknown switch: %1", arg.c_str());
			CL_Console::write_line("Using default settings...");
			break;
		}
	}

	//CL_ResourceManager resources("font.xml");
	font1 = CL_Font_Bitmap(m_gc, L"Tahoma", 18);
	font1small = CL_Font_Bitmap(m_gc, L"Tahoma", 13);
	m_gc.set_font(font1);

	if (!client && !server)
	{
		CL_Console::write_line("Press 'S' in the display window to start as a server");
		CL_Console::write_line("Press 'C' in the display window to start as a client");
		while (1)
		{
			m_gc.clear(CL_Colorf(0.2f, 0.6f, 1.0f));

			m_gc.set_font(font1);
			m_gc.draw_text(3, 13, "Press 'S' to start as a server");
			m_gc.draw_text(3, 30, "Press 'C' to start as a client");

			if (m_ic.get_keyboard().get_keycode('S'))
			{
				server = true;
				break;
			}
			else if (m_ic.get_keyboard().get_keycode('C'))
			{
				client = true;
				break;
			}
			
			if (CL_DisplayMessageQueue::has_messages())
				CL_DisplayMessageQueue::process();
			display.flip();
			CL_System::sleep(33);
		}
	}

	m_Network = new RakNetwork();

	// Connect to input events
	m_Slots.connect(m_ic.get_keyboard().sig_key_down(), this, &TestApp::key_down);
	m_Slots.connect(m_ic.get_keyboard().sig_key_up(), this, &TestApp::key_up);

	m_CommandNumber = 0;
	m_LatestReceivedTick = 0;

	if (server)
		run_server(maxPlayers);
	else
		run_client(host);

	delete m_Network;

	// Ending so soon?
	return 0;
}

void TestApp::run_server(unsigned short maxPlayers)
{
	m_Network->Startup(maxPlayers, 40001, maxPlayers);

	nextId = 0;
	nextAuthId = 2;
	sentActions = boost::circular_buffer<std::pair<unsigned long, Action>>(200);

	//int sendDelay = 0;
	unsigned int lastframe = CL_System::get_time();
	unsigned int frameTime = 0;
	unsigned int accumulator = 0;
	// Loop thing
	while (!m_ic.get_keyboard().get_keycode(CL_KEY_ESCAPE))
	{
		frameTime = CL_System::get_time() - lastframe;
		accumulator += frameTime;
		/*if (frameTime > 132)
		frameTime = 132;*/
		lastframe = CL_System::get_time();

		server_receive();

		while (accumulator >= split)
		{
			accumulator -= split;
			server_simulate(1.f);
		}

		server_updateAuthority(frameTime);

		server_send(frameTime);

		server_render(frameTime);

		if (CL_DisplayMessageQueue::has_messages())
			CL_DisplayMessageQueue::process();
		CL_System::sleep(10);
	}
}

void TestApp::run_client(const std::string& host)
{
	m_Network->Startup(1, 40000, 1);

	m_Tightness = s_DefaultTightness;

	m_SpriteBatch = CL_SpriteRenderBatch(m_gc);

	CL_SpriteDescription sprDesc;
	sprDesc.add_frame(CL_PNGProvider::load(cl_text("Body.png")));

	shipSprite = CL_Sprite(sprDesc, m_gc);

	if (!m_Network->Connect(host, 40001))
		CL_Console::write_line("Connect failed");
	CL_Console::write("Connecting to host at %1...", host.c_str());
	IPacket* p;
	while (1)
	{
		p = m_Network->Receive();
		if (p!=NULL)
		{
			if (p->GetType()==ID_CONNECTION_REQUEST_ACCEPTED)
			{
				serverHandle = p->GetSystemHandle();
				m_Network->DeallocatePacket(p);
				break;
			}
			if (p->GetType()==ID_CONNECTION_ATTEMPT_FAILED)
			{
				CL_Console::write_line(" failed :(");
				m_Network->DeallocatePacket(p);
				throw CL_Exception("Failed to connect");
			}

			m_Network->DeallocatePacket(p);
		}
		CL_System::sleep(30);
	}
	CL_Console::write_line(" done!");


	int sendDelay = 0;
	unsigned int lastframe = CL_System::get_time();
	unsigned int accumulator = 0;

	m_Tick = m_CommandNumber;

	int previousPing = m_Network->GetPing(serverHandle);
	m_PingBuffer.set_capacity(10000/split);


	RakNetTimeNS beginAt;
	// Loop thing
	while (!m_ic.get_keyboard().get_keycode(CL_KEY_ESCAPE))
	{
		accumulator += CL_System::get_time() - lastframe;
		lastframe = CL_System::get_time();
		//if (accumulator > 700)
			//accumulator = 700;

		// Grab debug inputs
		if (m_ic.get_keyboard().get_keycode(CL_KEY_RETURN))
		{
			m_Network->Send(true, MTID_CHALL, "Hi", 2, 
				FusionEngine::LOW_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
		}

		if (m_Tightness > 0.1f && m_ic.get_keyboard().get_keycode(CL_KEY_END))
			m_Tightness -= 0.1f;
		else if (m_Tightness < 0.9f && m_ic.get_keyboard().get_keycode(CL_KEY_HOME))
			m_Tightness += 0.1f;

		// Grab input
		for (ShipMap::iterator it = myShips.begin(), end = myShips.end(); it != end; ++it)
		{
			get_input(it->second);
		}

		const int ping = m_Network->GetLastPing(serverHandle);
		m_PingBuffer.push_back(ping - previousPing);
		previousPing = ping;

		beginAt = RakNet::GetTimeNS();

		client_receive();

		RakNetTimeNS receiveTime = RakNet::GetTimeNS() - beginAt;
		beginAt = RakNet::GetTimeNS();

		long ticks = 0;
		while (accumulator >= split)
		{
			accumulator -= split;

			// Adjust sim speed to keep just ahead of the server
			long curTicksAhead = m_CommandNumber - m_LatestReceivedTick;
			if (curTicksAhead != m_TicksAhead)
				m_Tightness = 0.1f;

			if (curTicksAhead < (long)m_TicksAhead) // Not far enough ahead
				m_SimAdjust = m_SimAdjust + (1.25f - m_SimAdjust) * m_Tightness;
			else if (curTicksAhead > (long)m_TicksAhead) // Too far ahead
				m_SimAdjust = m_SimAdjust + (0.75 - m_SimAdjust) * m_Tightness;
				
			fe_clamp(m_SimAdjust, 0.75f, 1.25f);

			m_Tightness += (0.25f - m_Tightness) * 0.01f;

			m_LastTick = m_Tick;
			m_Tick += m_SimAdjust;

			if (curTicksAhead < -100)
				m_Tick += -curTicksAhead;

			// See how many full integer ticks have passed
			ticks = ((long)m_Tick - (long)m_LastTick);

			client_simulate(ticks);
		}

		m_TicksAhead += ticks; // Keep ahead until we receive another packet from the server

		RakNetTimeNS simTime = RakNet::GetTimeNS() - beginAt;

		client_updateAuthority(ticks * split);
		if ((m_SendDelay -= ticks * split) <= 0)
		{
			m_SendDelay = s_LowSendInterval;
			client_send(ticks * split);
		}

		beginAt = RakNet::GetTimeNS();

		client_render(split, accumulator);

		RakNetTimeNS renderTime = RakNet::GetTimeNS() - beginAt;

		m_gc.draw_text(2, 22, "Receiving took " + CL_StringHelp::uint_to_text(receiveTime));
		m_gc.draw_text(2, 43, "Simulating took " + CL_StringHelp::uint_to_text(simTime));
		m_gc.draw_text(2, 64, "Rendering took " + CL_StringHelp::uint_to_text(renderTime));

		m_gc.draw_text(2, 400, cl_format("Sim speed: %1  Ticks in frame: %2", m_SimAdjust, ticks));
		m_gc.draw_text(250, 432, cl_format("Fake Lag: %1  Max BpS: %2", m_Network->GetDebugLagMin(), m_Network->GetDebugAllowBps()));

		m_window.flip();

		if (CL_DisplayMessageQueue::has_messages())
			CL_DisplayMessageQueue::process();
		CL_System::sleep(30);
	}

	m_Network->Disconnect();
}

void TestApp::server_receive()
{
	IPacket* p;
	p = NULL;
	p = m_Network->Receive();
	while (p!=NULL)
	{
		if (p->GetType()==ID_CONNECTION_REQUEST)
			CL_Console::write_line("Connection request");

		else if (p->GetType()==ID_NEW_INCOMING_CONNECTION)
		{
			CL_Console::write_line("New incomming connection");

			ClientData client;
			client.SetSystemHandle(p->GetSystemHandle());
			client.SetAuthorityId(nextAuthId);
			m_Clients[p->GetSystemHandle()] = client;

			RakNet::BitStream bits;
			bits.Write(nextAuthId);
			bits.Write(m_CommandNumber + 1);
			m_Network->Send(false, MTID_JOINSETUP, bits.GetData(), bits.GetNumberOfBytesUsed(),
				FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());

			CL_Console::write_line("New Client: Auth Id: %1", nextAuthId);

			++nextAuthId;
		}

		else if (p->GetType() == ID_CONNECTION_LOST || p->GetType() == ID_DISCONNECTION_NOTIFICATION)
		{
			CL_Console::write_line("Connection Lost");

			std::pair<ClientOwnershipMap::iterator, ClientOwnershipMap::iterator> _where =
				clientEntities.equal_range(p->GetSystemHandle());
			// Iterate through all the entities owned by this client
			for (ClientOwnershipMap::iterator it = _where.first; it != _where.second; ++it)
			{
				// Remove the ownership marker for this entity from the AuthMan
				m_AuthorityManager.SetEntityOwnership(it->second, 0);
				// Erase this entity from the other ownership mapping (i.e. the entity->owner map)
				entityOwnership.erase(it->second);

				// Tell the other clients
				RakNet::BitStream bits;
				bits.Write(it->second);
				for (ClientOwnershipMap::iterator otherIt = clientEntities.begin(), otherEnd = clientEntities.end();
					otherIt != otherEnd; ++otherIt)
				{
					m_Network->Send(false, MTID_REMOVEENTITY, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
						FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, it->first);
				}

				ships.erase(it->second);
			}
			clientEntities.erase(p->GetSystemHandle());
			m_Clients.erase(p->GetSystemHandle());
		}

		else if (p->GetType() == MTID_CHALL)
		{
			CL_String message(p->GetDataString().c_str());
			CL_Console::write_line("Server received: '%1' at %2 ticks", message, p->GetTime());
		}

		else if (p->GetType() == MTID_ADDENTITY)
		{
			ObjectID clientShipId;
			// Read the temporary ship ID that the client assigned
			{
				RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
				bits.Read(clientShipId);
			}
			// Send the verification
			{
				RakNet::BitStream bits;
				bits.Write(clientShipId);
				bits.Write(nextId);
				m_Network->Send(false, MTID_ADDALLOWED, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
					FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());
			}

			CL_Console::write_line("Add Allowed: ID (for client entity %1): %2 At: %3", clientShipId, nextId, (long)m_CommandNumber);

			ObjectID authId = m_Clients[p->GetSystemHandle()].GetAuthorityId();

			// Tell all the other clients to add this object
			RakNet::BitStream toOldClients;
			toOldClients.Write(nextId);
			toOldClients.Write((bool)true);
			toOldClients.Write(authId);
			for (ClientMap::iterator it = m_Clients.begin(), end = m_Clients.end(); it != end; ++it)
			{
				NetHandle clientHandle = it->first;

				if (clientHandle == p->GetSystemHandle())
					continue;

				m_Network->Send(true, MTID_ADDENTITY, toOldClients.GetData(), toOldClients.GetNumberOfBytesUsed(),
					FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, clientHandle);
			}

			// Tell the new client about the current entities
			for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
			{
				ObjectID entityId = it->first;

				ObjectID oldAuthId = m_AuthorityManager.GetEntityOwnership(entityId);

				RakNet::BitStream toNewClient;
				toNewClient.Write(entityId);
				if (oldAuthId != 0)
				{
					toNewClient.Write((bool)true);
					toNewClient.Write(oldAuthId);
				}
				else
					toNewClient.Write((bool)false);
				m_Network->Send(true, MTID_ADDENTITY, toNewClient.GetData(), toNewClient.GetNumberOfBytesUsed(),
					FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());
			}

			// Create a ship for the new player
			ShipPtr ship(new Ship(nextId));
			ship->sendDelay = 1000;
			ships.insert(ShipMap::value_type(nextId, ship));

			clientEntities.insert(ClientOwnershipMap::value_type(p->GetSystemHandle(), nextId));
			entityOwnership[nextId] = p->GetSystemHandle();

			m_AuthorityManager.SetEntityOwnership(nextId, authId);
			++nextId;
		}

		else if (p->GetType() == MTID_ENTITYMOVE)
		{
			unsigned short nEntitiesInPacket;
			ObjectID object_id;
			unsigned long commandNumber;
			bool containsInput;

			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), true);
			bits.Read(commandNumber);

			ClientData &client = m_Clients.find(p->GetSystemHandle())->second;
			client.SetMostRecentTick(commandNumber);

			bits.Read(nEntitiesInPacket);
			for (int i = 0; i < nEntitiesInPacket; ++i)
			{
				bits.Read(object_id);
				ShipMap::iterator it = ships.find(object_id);
				ShipPtr ship = it->second;

				bits.Read(containsInput);
				if (containsInput)
				{
					Command input;
					bits.Read(input.up);
					bits.Read(input.down);
					bits.Read(input.left);
					bits.Read(input.right);

					ship->commandList.push_back(commandNumber, input);
				}

				Action state;
				bits.Read(state.velocity.x);
				bits.Read(state.velocity.y);

				bits.Read(state.angularVelocity);

				bits.Read(state.position.x);
				bits.Read(state.position.y);

				bits.Read(state.angle);


				AuthoritativeAction authAct(client.GetAuthorityId(), state);
				ship->actionList.push_back(commandNumber, authAct);
				//m_AuthorityManager.InjectState(client.GetAuthorityId(), object_id, state);
			}
		}

		m_Network->DeallocatePacket(p);
		p = NULL;
		p = m_Network->Receive();
	}
}

void TestApp::server_simulate(float ticks)
{
	if (!m_Clients.empty())
	{
		unsigned long mostRecentSyncTick = m_CommandNumber + ticks;
		/*unsigned long mostRecentSyncTick = m_Clients.begin()->second.GetMostRecentTick();
		for (ClientMap::iterator it = m_Clients.begin(), end = m_Clients.end(); it != end; ++it)
		{
			mostRecentSyncTick = fe_min(mostRecentSyncTick, it->second.GetMostRecentTick());
		}*/

		//for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
		//{
		//	ShipPtr ship = it->second;

		//	ship->currentCommand = ship->commandList.begin();
		//	ship->currentAction = ship->actionList.begin();

		//	if (!ship->commandList.empty())
		//	{
		//		for (CommandHistory::iterator next = ship->commandList.begin() + 1, end = ship->commandList.end();
		//			next != end; ++next)
		//		{
		//			if (next->first == m_CommandNumber)
		//				ship->currentCommand = next;
		//		}
		//	}

		//	if (!ship->actionList.empty())
		//	{
		//		for (ActionHistory::iterator next = ship->actionList.begin() + 1, end = ship->actionList.end();
		//			next != end; ++next)
		//		{
		//			if (next->first == m_CommandNumber)
		//				ship->currentAction = next;
		//		}
		//	}
		//}

		// Simulate up to the most recent tick possible
		// mostRecentSyncTick is the minimum of the most recent ticks received from each client
		while (m_CommandNumber < mostRecentSyncTick)
		{
			//frameTimeLeft -= split;
			for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
			{
				ShipPtr ship = it->second;

				if (ship->currentCommand == ship->commandList.end())
					ship->currentCommand = ship->commandList.begin();
				if (ship->currentAction == ship->actionList.end())
					ship->currentAction = ship->actionList.begin();

				// Go to the next command if we've reached it
				{
					CommandHistory::iterator next = ship->currentCommand + 1;
					if (!ship->commandList.empty() && next != ship->commandList.end())
					{
						if (next->first < m_CommandNumber)
						{
							for (CommandHistory::iterator end = ship->commandList.end(); next != end; ++next)
							{
								if (next->first == m_CommandNumber)
									ship->currentCommand = next;
							}
							CL_Console::write_line("Client too far behind, dropped commands");
							//ship->currentCommand = next;
						}
						else if (next->first == m_CommandNumber)
							ship->currentCommand = next;
					}
				}
				// Go to the next action
				{
					ActionHistory::iterator next = ship->currentAction + 1;
					if (!ship->actionList.empty() && next != ship->actionList.end())
					{
						if (next->first < m_CommandNumber)
						{
							for (ActionHistory::iterator end = ship->actionList.end(); next != end; ++next)
							{
								if (next->first == m_CommandNumber)
									ship->currentAction = next;
							}
							CL_Console::write_line("Client too far behind, dropped actions");
							//ship->currentAction = next;
						}
						else if (next->first == m_CommandNumber)
							ship->currentAction = next;
					}
				}

				// Set the ship input to the correct one
				if (ship->currentCommand != ship->commandList.end())
				{
					ship->SetCommand(ship->currentCommand->second);
				}

				// Simulate
				float up = ship->up ? -1.f : 0.f;
				float down = ship->down ? 1.f : 0.f;
				float left = ship->left ? -1.f : 0.f;
				float right = ship->right ? 1.f : 0.f;
				ship->velocity.x = s_Speed * split * (left + right);
				ship->velocity.y = s_Speed * split * (up + down);

				ship->position.x += ship->velocity.x;
				ship->position.y += ship->velocity.y;

				fe_clamp(ship->position.x, 0.f, m_WorldWidth);
				fe_clamp(ship->position.y, 0.f, m_WorldHeight);
			}
			m_CommandNumber++;
		}

		//for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
		//{
		//	ShipPtr ship = it->second;
		//	ship->currentAction = ship->actionList.erase_before(ship->currentAction);
		//	ship->currentCommand = ship->commandList.erase_before(ship->currentCommand);
		//}
	}
}

void TestApp::server_updateAuthority(float dt)
{
	SimulationState simState;
	for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		simState.SetEntityInput(it->first, ship->GetCommand());
		simState.SetEntityAction(it->first, ship->GetAction());
		simState.SetEnabled(it->first, ship->velocity.length() > 0.1f);
	}

	m_AuthorityManager.SetSimulationState(simState);

	for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		//ship->currentCommand = ship->commandList.begin();
		//ship->currentAction = ship->actionList.begin();
		// Inject the input for the current tick (if we have one)
		if (ship->currentCommand != ship->commandList.end() && ship->currentCommand->first == m_CommandNumber - 1)
		{
			Command& cmd = ship->currentCommand->second;
			m_AuthorityManager.InjectInput(ship->id, cmd);
		}
		// Inject the state for the current tick (if we have one)
		if (ship->currentAction != ship->actionList.end() && ship->currentAction->first == m_CommandNumber - 1)
		{
			AuthoritativeAction& authAct = ship->currentAction->second;
			m_AuthorityManager.InjectState(authAct.authId, ship->id, authAct);
		}
	}

	m_AuthorityManager.Update(dt);
	m_AuthorityManager.ReadSimulationState(simState);
	
	for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		ship->SetAction( simState.GetAction(ship->id) );
		ship->SetCommand( simState.GetInput(ship->id) );
	}
}

void TestApp::server_send(float dt)
{
	typedef std::map<ObjectID, ShipPtr> SendPriorityQueue;

	const AuthorityState& authority = m_AuthorityManager.GetAuthorityState();
	// Order entities by send priority
	SendPriorityQueue priority;
	for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		int priorityValue = ship->lastSentTick;
		if (authority.IsRemote(ship->id))
			priorityValue += s_LowSendInterval;

		priority.insert(SendPriorityQueue::value_type(priorityValue, ship));
	}

	// Build and send packets to each client
	for (ClientMap::iterator clientIt = m_Clients.begin(), end = m_Clients.end(); clientIt != end; ++clientIt)
	{
		ClientData &client = clientIt->second;

		// Make sure the send interval has been passed
		client.UpdateTimeTillSend(dt);
		if (!client.IsSendAllowed())
			continue;
		client.ResetTimeTillSend();

		unsigned short entityCount = 0;
		RakNet::BitStream packetBits;
		for (SendPriorityQueue::iterator it = priority.begin(), end = priority.end(); it != end; ++it)
		{
			ShipPtr ship = it->second;

			// Skip entities that this client has authority over
			if (authority.GetEntityAuthorityId(ship->id) == client.GetAuthorityId())
				continue;

			RakNet::BitStream bits;

			bits.Write(ship->id);

			bits.Write((bool)true);
			bits.Write(ship->up);
			bits.Write(ship->down);
			bits.Write(ship->left);
			bits.Write(ship->right);

			bits.Write(ship->velocity.x);
			bits.Write(ship->velocity.y);

			bits.Write(ship->angularVelocity);

			bits.Write(ship->position.x);
			bits.Write(ship->position.y);

			bits.Write(ship->angle);

			// Make sure this data will fit within the maximum packet size
			if (packetBits.GetNumberOfBytesUsed() + bits.GetNumberOfBytesUsed() > s_PacketSize - sizeof(unsigned short) * 2)
				break; // Don't add any more data to the packet if it has reached the max size

			packetBits.Write(&bits);
			++entityCount;

			// Save the send time & action for debug drawing
			sentActions.push_back( std::pair<unsigned long, Action>(m_CommandNumber, ship->GetAction()) );
		}

		RakNet::BitStream fullPacket;
		fullPacket.Write(m_CommandNumber);
		fullPacket.Write(entityCount);
		fullPacket.Write(&packetBits);

		m_Network->Send(true, MTID_ENTITYMOVE, fullPacket.GetData(), fullPacket.GetNumberOfBytesUsed(), 
			FusionEngine::HIGH_PRIORITY, FusionEngine::UNRELIABLE_SEQUENCED, 1, client.GetSystemHandle());
	}
}

void TestApp::server_render(float dt)
{
	m_gc.clear(CL_Colorf(0.2f, 0.6f, 1.0f));

	CL_Colorf drawColor = CL_Colorf::blue;
	drawColor.set_alpha(75);
	CL_Colorf correctionColor = CL_Colorf::red;
	correctionColor.set_alpha(85);
	CL_Colorf normalColor = CL_Colorf::white;
	normalColor.set_alpha(60);
	CL_Colorf sentColor = CL_Colorf::green;
	sentColor.set_alpha(60);
	for (ShipMap::iterator it = ships.begin(); it != ships.end(); ++it)
	{
		ShipPtr ship = it->second;

		// Draw debug info
		for (ActionBuffer::iterator actIt = sentActions.begin(), actEnd = sentActions.end();
			actIt != actEnd; ++actIt)
		{
			Action& act = actIt->second;
			draw_rectangle(CL_Rectf(act.position.x, act.position.y, act.position.x + 10, act.position.y + 10), sentColor);
		}
		for (ActionHistory::iterator histIt = ship->actionList.begin(), histEnd = ship->actionList.end();
			histIt != histEnd; ++histIt)
		{
			AuthoritativeAction& authAct = histIt->second;
			draw_rectangle(CL_Rectf(authAct.position.x, authAct.position.y, authAct.position.x + 10, authAct.position.y + 10), drawColor);
		}
		draw_rectangle(CL_Rectf(ship->position.x, ship->position.y, ship->position.x + 10, ship->position.y + 10), CL_Colorf::darkblue);
		
		m_gc.set_font(font1small);
		EntityOwnerMap::iterator _where = entityOwnership.find(ship->id);
		ClientData& client = m_Clients[_where->second];
		m_gc.draw_text((int)ship->position.x, (int)ship->position.y + 20, cl_format("%1", (unsigned int)client.GetMostRecentTick()));

		CommandHistory::iterator wCommand = ship->currentCommand;
		if (wCommand != ship->commandList.end())
		{
			CL_String ccText = cl_format("Current %1. U%2 D%3 L%4 R%5",
				(long)wCommand->first,
				wCommand->second.up ? "*" : " ", wCommand->second.down ? "*" : " ",
				wCommand->second.left ? "*" : " ", wCommand->second.right ? "*" : " ");
			m_gc.draw_text((int)ship->position.x, (int)ship->position.y + 30, ccText);
		}

		CommandHistory::record_type &nCommand = ship->commandList.newest();
		CL_String lcText = cl_format("Newest %1 (size %2). U%3 D%4 L%5 R%6",
			(long)nCommand.first, ship->commandList.size(),
			nCommand.second.up ? "*" : " " , nCommand.second.down ? "*" : " ",
			nCommand.second.left ? "*" : " ", nCommand.second.right ? "*" : " ");
		m_gc.draw_text((int)ship->position.x, (int)ship->position.y + 40, lcText);
	}

	m_gc.set_font(font1);
	m_gc.draw_text(250, 40, cl_format("Command: %1", (unsigned int)m_CommandNumber));

	m_window.flip();
}

void TestApp::client_receive()
{
	IPacket* p;
	p = NULL;
	p = m_Network->Receive();
	while (p!=NULL)
	{
		if (p->GetType() == ID_REMOTE_NEW_INCOMING_CONNECTION)
		{
			CL_Console::write_line("Another player has joined the current server");
		}

		if (p->GetType() == MTID_REMOVEENTITY)
		{
			ObjectID object_id;
			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
			bits.Read(object_id);

			if (myShips.find(object_id) == myShips.end())
				ships.erase(object_id);
			else
			{
				// For some reason one of the local ships has been deleted; request a new ship
				RakNet::BitStream bits;
				bits.Write(nextId++);
				bits.Write((bool)true);
				m_Network->Send(true, MTID_ADDENTITY, (char *)bits.GetData(), bits.GetNumberOfBytesUsed(),
					FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
			}
		}

		if (p->GetType() == MTID_ADDENTITY)
		{
			ObjectID object_id, authority_id = 0;
			bool owned;

			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
			bits.Read(object_id);
			bits.Read(owned);
			if (owned)
				bits.Read(authority_id);

			if (ships.find(object_id) == ships.end())
			{
				ships[object_id] = ShipPtr(new Ship(object_id));
				if (owned)
					m_AuthorityManager.SetEntityOwnership(object_id, authority_id);
			}
			else
				CL_Console::write_line("Server asked us to re-add player. Request ignored. ID: %1", object_id);

			CL_Console::write_line("New Player: ID: %1, Owner Authority: %2", object_id, authority_id);
		}

		if (p->GetType() == MTID_ADDALLOWED)
		{
			ObjectID my_temp_id, object_id;
			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
			bits.Read(my_temp_id);
			bits.Read(object_id);

			ShipPtr myShip(new Ship(object_id));
			myShip->local = true;
			ships[object_id] = myShip;
			myShips[object_id] = myShip;

			m_AuthorityManager.SetLocalEntityOwnership(object_id);

			CL_Console::write_line("Add Allowed: ID: %1 At: %2", object_id, (long)m_CommandNumber);
		}

		if (p->GetType() == MTID_JOINSETUP)
		{
			{
				ObjectID authorityId;
				RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
				bits.Read(authorityId);
				bits.Read(m_CommandNumber);

				m_Tick = m_CommandNumber;

				m_AuthorityManager.SetLocalAuthorityId(authorityId);
			}

			// Request a ship
			RakNet::BitStream bits;
			bits.Write(nextId++);
			bits.Write((bool)true); // Request ownership
			m_Network->Send(true, MTID_ADDENTITY, (char *)bits.GetData(), bits.GetNumberOfBytesUsed(),
				FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
		}

		if (p->GetType() == MTID_CHALL)
		{
			CL_String message(p->GetDataString().c_str());
			CL_Console::write_line("Received: '%1' at %2 ticks", message, p->GetTime());
		}

		if (p->GetType() == MTID_ENTITYMOVE)
		{
			ObjectID object_id, authority_id;
			unsigned long commandNumber;

			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);

			bits.Read(commandNumber);
			// Used for speed correction
			m_LatestReceivedTick = fe_max(commandNumber, m_LatestReceivedTick);

			// Recalculate ticks ahead
			int smoothedPing = m_Network->GetSmoothedPing(serverHandle);
			//int jitterAvg = std::accumulate(m_PingBuffer.begin(), m_PingBuffer.end(), 0);
			//jitterAvg = jitterAvg / m_PingBuffer.size();

			m_TicksAhead = smoothedPing;// + jitterAvg;
			m_TicksAhead = m_TicksAhead / split + 1;

			unsigned short nEntitiesInPacket = 0;
			bool containsInput;
			bits.Read(nEntitiesInPacket);
			for (int i = 0; i < nEntitiesInPacket; ++i)
			{
				bits.Read(object_id);
				ShipMap::iterator it = ships.find(object_id);
				ShipPtr ship = it->second;

				bits.Read(containsInput);
				if (containsInput)
				{
					Command input;
					bits.Read(input.up);
					bits.Read(input.down);
					bits.Read(input.left);
					bits.Read(input.right);

					m_AuthorityManager.InjectInput(object_id, input);
				}

				Action state;
				bits.Read(state.velocity.x);
				bits.Read(state.velocity.y);

				bits.Read(state.angularVelocity);

				bits.Read(state.position.x);
				bits.Read(state.position.y);

				bits.Read(state.angle);

				m_AuthorityManager.InjectState(authority_id, object_id, state);

				//if (ship->mostRecentCommand >= commandNumber)
				//	ship->mostRecentCommand = fe_max(commandNumber, ship->mostRecentCommand);
			}
		}

		m_Network->DeallocatePacket(p);
		p = NULL;
		p = m_Network->Receive();
	}
}

void TestApp::client_simulate(long ticks)
{
		for (ShipMap::iterator it = myShips.begin(), end = myShips.end(); it != end; ++it)
		{
			ShipPtr myShip = it->second;

			// Predict simulation
			float up = myShip->up ? -1.f : 0.f;
			float down = myShip->down ? 1.f : 0.f;
			float left = myShip->left ? -1.f : 0.f;
			float right = myShip->right ? 1.f : 0.f;
			myShip->velocity.x = s_Speed * ticks * split * (left + right);
			myShip->velocity.y = s_Speed * ticks * split * (up + down);

			myShip->position.x += myShip->velocity.x;
			myShip->position.y += myShip->velocity.y;

			fe_clamp(myShip->position.x, 0.f, m_WorldWidth);
			fe_clamp(myShip->position.y, 0.f, m_WorldHeight);

			//myShip->saveAction(m_CommandNumber);
			//myShip->saveCommand(m_CommandNumber);
		}

		m_CommandNumber += ticks;
}

void TestApp::client_updateAuthority(float dt)
{
	SimulationState simState;
	for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		simState.SetEntityInput(it->first, ship->GetCommand());
		simState.SetEntityAction(it->first, ship->GetAction());
		simState.SetEnabled(it->first, ship->velocity.length() > 0.1f);
	}

	m_AuthorityManager.SetSimulationState(simState);
	m_AuthorityManager.Update(dt);
	m_AuthorityManager.ReadSimulationState(simState);
	
	for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		ship->SetAction( simState.GetAction(ship->id) );
		ship->SetCommand( simState.GetInput(ship->id) );
	}
}

void TestApp::client_send(float dt)
{
	typedef std::set<ShipMap::value_type, SendPriority> SendPriorityQueue;

	if (ships.empty())
		return;

	const AuthorityState &authority = m_AuthorityManager.GetAuthorityState();
	// SendPriorityQueue orders entities by how long it has been since their state was last sent
	SendPriorityQueue priority;
	for (ShipMap::iterator it = ships.begin(), end = ships.end(); it != end; ++it)
	{
		if (authority.IsLocal(it->first))
			priority.insert(*it);
	}

	unsigned short entityCount = 0;
	RakNet::BitStream packetBits;
	for (SendPriorityQueue::iterator it = priority.begin(), end = priority.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		RakNet::BitStream bits;

		// Entity ID
		bits.Write(ship->id);

		// Input
		if (ship->local)
		{
			bits.Write((bool)true);
			bits.Write(ship->up);
			bits.Write(ship->down);
			bits.Write(ship->left);
			bits.Write(ship->right);
		}
		else
			bits.Write((bool)false);

		// State
		bits.Write(ship->velocity.x);
		bits.Write(ship->velocity.y);

		bits.Write(ship->angularVelocity);
		
		bits.Write(ship->position.x);
		bits.Write(ship->position.y);

		bits.Write(ship->angle);

		// Actions only have to be saved for sent commands, since the server wont make corrections
		//  to actions it doesn't know we've taken!
		//myShip->saveAction(m_CommandNumber);
		//myShip->saveCommand(m_CommandNumber);

		// Make sure this data will fit within the maximum packet size (minus sizeof(numInputs and numStates))
		if (packetBits.GetNumberOfBytesUsed() + bits.GetNumberOfBytesUsed() > s_PacketSize - sizeof(unsigned short) * 2)
			break; // Don't add this data to the packet if it's too big

		packetBits.Write(&bits);
		++entityCount;
	}

	// Build the full packet (including count markers)
	RakNet::BitStream fullPacket;
	fullPacket.Write((unsigned long)m_Tick);
	fullPacket.Write(entityCount);
	fullPacket.Write(&packetBits);

	m_Network->Send(true, MTID_ENTITYMOVE, fullPacket.GetData(), fullPacket.GetNumberOfBytesUsed(), 
		FusionEngine::HIGH_PRIORITY, FusionEngine::UNRELIABLE_SEQUENCED, 1, serverHandle);
}

void TestApp::client_render(float tickTime, float accumulatorRemaining)
{
	m_gc.clear(CL_Colorf(0.2f, 0.6f, 1.0f));

	for (ShipMap::iterator it = ships.begin(); it != ships.end(); ++it)
	{
		ShipPtr ship = it->second;
		if (m_ic.get_keyboard().get_keycode('H'))
		{
			for (ActionHistory::iterator actIt = ship->actionList.begin(), actEnd = ship->actionList.end();
				actIt != actEnd; ++actIt)
			{
				AuthoritativeAction &act = actIt->second;
				shipSprite.set_color(CL_Colorf(0.99f, 1.0f, 0.8f, 0.4f));
				shipSprite.draw(m_SpriteBatch, act.position.x, act.position.y);
			}
		}
		shipSprite.set_color(CL_Colorf::white);
		shipSprite.draw(m_SpriteBatch, ship->position.x, ship->position.y);
		m_gc.draw_text((int)ship->position.x, (int)ship->position.y, cl_format("%1", ship->id));
	}

	m_gc.draw_text(290, 16, cl_format("Ping: %1", m_Network->GetPing(serverHandle)));
	m_gc.draw_text(250, 38, cl_format("Tick: %1", m_Tick));
	m_gc.draw_text(210, 51, cl_format("Ticks Ahead Target: %1", (unsigned int)m_TicksAhead));

	m_SpriteBatch.flush();
	//m_window.flip();
}

void TestApp::draw_rectangle(const CL_Rectf &rect, const CL_Colorf &color)
{
	CL_Vec2f positions[] =
	{
		CL_Vec2f(rect.left, rect.top),
		CL_Vec2f(rect.right, rect.top),
		CL_Vec2f(rect.right, rect.bottom),
		CL_Vec2f(rect.left, rect.bottom)
	};
	
	CL_PrimitivesArray vertex_data(m_gc);
	vertex_data.set_positions(positions);
	vertex_data.set_primary_color(color);
	m_gc.draw_primitives(cl_polygon, 4, vertex_data);
}

void TestApp::key_down(const CL_InputEvent& ev, const CL_InputState& state)
{
}

void TestApp::key_up(const CL_InputEvent& ev, const CL_InputState& state)
{
	if (ev.id == 'I')
	{
		if (m_Network->GetDebugLagMin() == 0)
		{
			m_Network->SetDebugLag(100, 0);
		}
		else if (m_Network->GetDebugLagMin() == 100)
		{
			m_Network->SetDebugLag(500, 0);
		}
		else if (m_Network->GetDebugLagMin() == 500)
		{
			m_Network->SetDebugLag(1000, 0);
		}
		else if (m_Network->GetDebugLagMin() == 1000)
		{
			m_Network->SetDebugLag(2000, 0);
		}
		else if (m_Network->GetDebugLagMin() == 2000)
		{
			m_Network->SetDebugLag(0, 0);
		}
	}
	if (ev.id == 'K')
	{
		if (m_Network->GetDebugAllowBps() == 0)
		{
			m_Network->SetDebugPacketLoss(800);
		}
		else if (m_Network->GetDebugAllowBps() == 800)
		{
			m_Network->SetDebugPacketLoss(500);
		}
		else if (m_Network->GetDebugAllowBps() == 500)
		{
			m_Network->SetDebugPacketLoss(0);
		}
	}
}


#endif