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

using namespace FusionEngine;

const unsigned int s_SendInterval = 60;

// Simulation speed
const unsigned int split = 33;

const float s_Speed = 0.05f;


class TestApp
{
private:
	unsigned long m_CommandNumber, m_LatestReceivedTick, m_MostRecentSyncTick;
	unsigned long m_TicksAhead;
	float m_SimAdjust;

	bool m_Interpolate;
	float m_Tightness;

	float m_WorldWidth, m_WorldHeight;

	typedef std::tr1::shared_ptr<Ship> ShipPtr;
	// For server side
	typedef std::tr1::unordered_map<NetHandle, ShipPtr> NetShipMap;
	// For client side
	typedef std::tr1::unordered_map<ObjectID, ShipPtr> ShipMap;

	Network* m_Network;

	CL_DisplayWindow m_window;
	CL_InputContext m_ic;
	CL_GraphicContext m_gc;

	CL_Font_Bitmap font1;

	unsigned int nextId;

	// Server specific data
	NetShipMap clientShips;
	boost::circular_buffer<std::pair<unsigned long, Action>> corrections;

	// Client specific data
	NetHandle serverHandle;
	ShipMap ships;
	ShipMap myShips;
	CL_Sprite shipSprite;
	CL_SpriteRenderBatch m_SpriteBatch;


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
	void client_simulate(float dt);
	void client_updateAuthority(float dt);
	void client_send(float dt);
	void client_render(float dt);

	void draw_rectangle(const CL_Rectf &rect, const CL_Colorf &color);

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

void TestApp::correct(const ShipPtr ship, unsigned long commandNumber, const Action& correctAction)
{
	std::cout << "Correcting action " << commandNumber << "...";
	//ship->rewindAction(commandNumber);
	ship->cullActions(); // Current action is confirmed (erase previous actions)
	ship->x = correctAction.x;
	ship->y = correctAction.y;
	ship->saveAction(commandNumber);
	ship->rewindCommand(commandNumber + 1);
	for (unsigned long t = commandNumber + 1; t <= m_CommandNumber; t++)
	{
		ship->nextCommand(t);

		// Redo simulation
		float up = ship->up ? -1.f : 0.f;
		float down = ship->down ? 1.f : 0.f;
		float left = ship->left ? -1.f : 0.f;
		float right = ship->right ? 1.f : 0.f;
		ship->x += s_Speed * split * (left + right);
		ship->y += s_Speed * split * (up + down);

		fe_clamp(ship->x, 0.f, m_WorldWidth);
		fe_clamp(ship->y, 0.f, m_WorldHeight);

		ship->saveAction(commandNumber);
	}
	// Make sure we're back on the current command
	ship->nextCommand();

	std::cout << " Server: (" << commandNumber << ")" << correctAction.x << "," << correctAction.y
		<< "  Current local (" << m_CommandNumber << ")" << ship->x << "," << ship->y << std::endl;
}

int TestApp::run(const std::vector<CL_String> &args, CL_DisplayWindow& display)
{
	m_window = display;
	m_ic = display.get_ic();
	m_gc = display.get_gc();

	CL_Rect viewport = display.get_viewport();
	viewport.translate(2, 10);
	m_gc.set_viewport(viewport);

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
		CL_String arg = *it;
		if (arg.length() < 3)
			continue;
		switch (arg[1]) 
		{
		case 'c':
			client = true;
			arg = arg.substr(2);
			host = std::string(arg.begin(), arg.end());
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
	CL_Font_Bitmap font1 = CL_Font_Bitmap(m_gc, L"Tahoma", 18);
	m_gc.set_font(font1);

	if (!client && !server)
	{
		std::cout << "Press 'S' in the display window to start as a server" << std::endl;
		std::cout << "Press 'C' in the display window to start as a client" << std::endl;
		while (1)
		{
			m_gc.clear(CL_Colorf(0.2f, 0.6f, 1.0f));

			m_gc.set_font(font1);
			m_gc.draw_text(3, 2, "Press 'S' to start as a server");
			m_gc.draw_text(3, 20, "Press 'C' to start as a client");

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
	corrections = boost::circular_buffer<std::pair<unsigned long, Action>>(200);

	//int sendDelay = 0;
	unsigned int lastframe = CL_System::get_time();
	unsigned int frameTime = 0;
	// Loop thing
	while (!m_ic.get_keyboard().get_keycode(CL_KEY_ESCAPE))
	{
		frameTime += CL_System::get_time() - lastframe;
		/*if (frameTime > 132)
		frameTime = 132;*/
		lastframe = CL_System::get_time();

		server_receive();

		server_simulate(frameTime);

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

	m_Interpolate = true;
	m_Tightness = s_DefaultTightness;

	m_SpriteBatch = CL_SpriteRenderBatch(m_gc);

	CL_SpriteDescription sprDesc;
	sprDesc.add_frame(CL_PNGProvider::load(cl_text("Body.png")));

	shipSprite = CL_Sprite(sprDesc, m_gc);

	if (!m_Network->Connect(host, 40001))
		std::cout << "Connect failed";
	std::cout << "Connecting to host at " + host + "..." << std::endl;
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
				std::cout << "Failed :(";
				m_Network->DeallocatePacket(p);
				throw CL_Exception("Failed to connect");
			}

			m_Network->DeallocatePacket(p);
		}
		CL_System::sleep(30);
	}
	std::cout << "done!" << std::endl;

	// Request a ship
	{
		RakNet::BitStream bits;
		bits.Write(nextId++);
		m_Network->Send(true, MTID_ADDPLAYER, (char *)bits.GetData(), bits.GetNumberOfBytesUsed(),
			FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
	}

	int sendDelay = 0;
	unsigned int lastframe = CL_System::get_time();
	unsigned int frameTime = 0;

	RakNetTimeNS beginAt;
	// Loop thing
	while (!m_ic.get_keyboard().get_keycode(CL_KEY_ESCAPE))
	{
		frameTime += CL_System::get_time() - lastframe;
		if (frameTime > 700)
			frameTime = 700;
		lastframe = CL_System::get_time();

		beginAt = RakNet::GetTimeNS();

		client_receive();

		RakNetTimeNS receiveTime = RakNet::GetTimeNS() - beginAt;
		beginAt = RakNet::GetTimeNS();

		if (m_ic.get_keyboard().get_keycode(CL_KEY_RETURN))
		{
			m_Network->Send(true, MTID_CHALL, "Hi", 2, 
				FusionEngine::LOW_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
		}

		if (m_Tightness > 0.1f && m_ic.get_keyboard().get_keycode(CL_KEY_END))
			m_Tightness -= 0.1f;
		else if (m_Tightness < 0.9f && m_ic.get_keyboard().get_keycode(CL_KEY_HOME))
			m_Tightness += 0.1f;

		if (m_ic.get_keyboard().get_keycode('I'))
			m_Interpolate = true;
		else if (m_ic.get_keyboard().get_keycode('K'))
			m_Interpolate = false;

		if (myShips.empty())
			continue;

		// Adjust sim speed to keep just ahead of the server
		long tickDelta = m_LatestReceivedTick - m_CommandNumber;
		if (tickDelta != m_TicksAhead)
		{
			m_Tightness = 0.1f;
		}
		m_SimAdjust = m_SimAdjust + (m_SimAdjust - 1.25f) * m_Tightness;
		m_Tightness += (0.25f - m_Tightness) * 0.01f;
		fe_clamp(m_SimAdjust, 0.75f, 1.25f);

		client_simulate(frameTime);

		RakNetTimeNS simTime = RakNet::GetTimeNS() - beginAt;
		beginAt = RakNet::GetTimeNS();

		client_render(frameTime);

		RakNetTimeNS renderTime = RakNet::GetTimeNS() - beginAt;

		m_gc.draw_text(2, 0, "Receiving took " + CL_StringHelp::uint_to_text(receiveTime));
		m_gc.draw_text(2, 22, "Simulating took " + CL_StringHelp::uint_to_text(simTime));
		m_gc.draw_text(2, 44, "Rendering took " + CL_StringHelp::uint_to_text(renderTime));

		if (CL_DisplayMessageQueue::has_messages())
			CL_DisplayMessageQueue::process();
		//CL_System::sleep(0);
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
			std::cout << "Connection request" << std::endl;

		else if (p->GetType()==ID_NEW_INCOMING_CONNECTION)
		{
			std::cout << "New incomming connection" << std::endl;

			RakNet::BitStream bits;
			bits.Write((m_CommandNumber+1));
			m_Network->Send(false, MTID_STARTTICK, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
				FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());
		}

		else if (p->GetType() == ID_CONNECTION_LOST || p->GetType() == ID_DISCONNECTION_NOTIFICATION)
		{
			std::cout << "Connection Lost" << std::endl;

			NetShipMap::iterator _where = clientShips.find(p->GetSystemHandle());
			if (_where != clientShips.end())
			{
				// Tell the other clients
				RakNet::BitStream bits;
				bits.Write(_where->second->id);
				for (NetShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
				{
					m_Network->Send(false, MTID_REMOVEPLAYER, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
						FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, it->first);
				}

				clientShips.erase(_where);
			}
		}

		else if (p->GetType() == MTID_CHALL)
		{
			std::cout << "Server received: '" << p->GetDataString() << "' at " << p->GetTime() << " ticks" << std::endl;
		}

		else if (p->GetType() == MTID_ADDPLAYER)
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

			std::cout << "Add Allowed: ID (for client entity " << clientShipId << "): " << nextId << " At: " << m_CommandNumber << std::endl;

			// Tell all the other clients to add this object
			RakNet::BitStream toOldPlayers;
			toOldPlayers.Write(nextId);
			for (NetShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
			{
				NetHandle clientHandle = (*it).first;
				ShipPtr ship = it->second;

				// Tell the new player about all the old players being iterated
				RakNet::BitStream toNewPlayer;
				toNewPlayer.Write(ship->id);
				m_Network->Send(true, MTID_ADDPLAYER, (char*)toNewPlayer.GetData(), toNewPlayer.GetNumberOfBytesUsed(),
					FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());

				// Tell all the old players being iterated about the new player
				m_Network->Send(true, MTID_ADDPLAYER, (char*)toOldPlayers.GetData(), toOldPlayers.GetNumberOfBytesUsed(),
					FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, clientHandle);
			}

			// Create a ship for the new player
			ShipPtr ship(new Ship(nextId++));
			ship->sendDelay = 1000;
			clientShips[p->GetSystemHandle()] = ship;
		}

		else if (p->GetType() == MTID_IMPORTANTMOVE)
		{
			ObjectID object_id;
			unsigned long commandNumber;

			const NetHandle &systemHandle = p->GetSystemHandle();
			ShipPtr ship = clientShips[systemHandle];

			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), true);
			bits.Read(object_id);

			if (object_id != ship->id)
				std::cout << "Warning: A client is sending data for another client's ship" << std::endl;

			bits.Read(commandNumber);

			bool up, down, left, right;
			bits.Read(up);
			bits.Read(down);
			bits.Read(left);
			bits.Read(right);

			float x, y;

			bits.Read(x);
			bits.Read(y);

			std::cout << commandNumber << " (important) " << x << ", " << y << std::endl;

			ship->mostRecentCommand = fe_max(commandNumber, ship->mostRecentCommand);
			ship->currentTick = fe_max(commandNumber, ship->currentTick);

			//ship->up = up;
			//ship->down = down;
			//ship->left = left;
			//ship->right = right;

			ship->saveCommand(commandNumber, Command(up, down, left, right));

			// Lag compensation (check that the given ship is more-or-less in sync)
			//if (needsCorrection/* || ship->checkAction(commandNumber, x, y)*/)

			std::cout << " to " << ship->x << ", " << ship->y << std::endl;
		}

		else if (p->GetType() == MTID_ENTITYMOVE)
		{
			ObjectID object_id;
			unsigned long commandNumber;
			unsigned long tick;

			const NetHandle &systemHandle = p->GetSystemHandle();
			ShipPtr ship = clientShips[systemHandle];

			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), true);
			bits.Read(object_id);

			if (object_id != ship->id)
				std::cout << "Warning: A client is sending data for another client's ship" << std::endl;

			bits.Read(tick); // the client's current tick when this packet was sent
			bits.Read(commandNumber); // the important command required for this tick

			//Command previousCommand = ship->GetCommand();
			bool up, down, left, right;
			bits.Read(up);
			bits.Read(down);
			bits.Read(left);
			bits.Read(right);

			//ship->saveCommand(commandNumber, Command(up, down, left, right));

			//bool needsCorrection = ship->checkCommand(commandNumber, ship->GetCommand());

			// Make sure the required important command has been received
			if (ship->mostRecentCommand >= commandNumber)
				ship->currentTick = fe_max(tick, ship->currentTick);
			// Sanity check
			//fe_clamp(ship->currentTick, (unsigned long)0, m_CommandNumber);

			float x, y;

			bits.Read(x);
			bits.Read(y);

			//if (previousCommand != ship->GetCommand())
			//{
			//	std::cout << commandNumber << " (important)" << (needsCorrection ? " correcting" : "") << std::endl;
			//}
		}

		m_Network->DeallocatePacket(p);
		p = NULL;
		p = m_Network->Receive();
	}
}

void TestApp::server_simulate(float dt)
{
	if (!clientShips.empty())
	{
		unsigned long mostRecentSyncTick = clientShips.begin()->second->currentTick;
		for (NetShipMap::iterator it = clientShips.begin(), end = clientShips.end(); it != end; ++it)
		{
			ShipPtr ship = it->second;
			mostRecentSyncTick = fe_min(mostRecentSyncTick, ship->currentTick);
			ship->currentCommand = ship->commandList.begin();
		}

		// Simulate up to the most recent tick possible
		// mostRecentSyncTick is the minimum of the most recent tick received from each client
		while (m_CommandNumber <= mostRecentSyncTick)
		{
			//frameTimeLeft -= split;
			for (NetShipMap::iterator it = clientShips.begin(), end = clientShips.end(); it != end; ++it)
			{
				ShipPtr ship = it->second;

				ship->nextCommand(m_CommandNumber);

				// Simulate
				float up = ship->up ? -1.f : 0.f;
				float down = ship->down ? 1.f : 0.f;
				float left = ship->left ? -1.f : 0.f;
				float right = ship->right ? 1.f : 0.f;
				ship->x += s_Speed * split * (left + right);
				ship->y += s_Speed * split * (up + down);

				fe_clamp(ship->x, 0.f, m_WorldWidth);
				fe_clamp(ship->y, 0.f, m_WorldHeight);

				ship->saveAction(m_CommandNumber);
				// Go to the next received command
				//ship->nextCommand(m_CommandNumber);

				//++ship->currentTick;
			}

			m_CommandNumber++;
		}
	}
}

void TestApp::server_updateAuthority(float dt)
{
}

void TestApp::server_send(float dt)
{
	for (NetShipMap::iterator it = clientShips.begin(), end = clientShips.end(); it != end; ++it)
	{
		ShipPtr ship = it->second;

		if ((ship->sendDelay -= dt) <= 0)
		{
			ship->sendDelay = s_SendInterval;
			// Send data

			RakNet::BitStream bits;
			bits.Write(ship->id);
			bits.Write(m_CommandNumber-1);

			bits.Write(ship->up);
			bits.Write(ship->down);
			bits.Write(ship->left);
			bits.Write(ship->right);

			bits.Write(ship->x);
			bits.Write(ship->y);

			corrections.push_back(std::pair<unsigned long, Action>(m_CommandNumber, Action(ship->x, ship->y)));

			//if (ship->currentCommand->first == m_CommandNumber)
			//{
			//	RakNet::BitStream correctionBits;
			//	bits.Write(ship->id);
			//	bits.Write(commandNumber);

			//	bits.Write(ship->x);
			//	bits.Write(ship->y);

			//	m_Network->Send(true, MTID_CORRECTION, (char*)correctionBits.GetData(), correctionBits.GetNumberOfBytesUsed(),
			//		FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE_ORDERED, 1, systemHandle);
			//}

			for (NetShipMap::iterator client = clientShips.begin(); client != clientShips.end(); ++client)
			{
				NetHandle clientHandle = (*client).first;
				m_Network->Send(true, MTID_ENTITYMOVE, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
					FusionEngine::HIGH_PRIORITY, FusionEngine::UNRELIABLE_SEQUENCED, 1, clientHandle);
			}

			// Heartbeat
			NetHandle clientHandle = it->first;
			m_Network->Send(true, MTID_STARTSYNC, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
				FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE_SEQUENCED, 2, clientHandle);
		} // end if (should send)
	}
}

void TestApp::server_render(float dt)
{
	m_gc.clear(CL_Colorf(0.2f, 0.6f, 1.0f));

	CL_Colorf drawColor = CL_Colorf::azure;
	drawColor.set_alpha(80);
	CL_Colorf correctionColor = CL_Colorf::red;
	drawColor.set_alpha(85);
	for (NetShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
	{
		ShipPtr ship = it->second;

		// Draw debug info
		for (ActionHistory::iterator actIt = corrections.begin(), actEnd = corrections.end();
			actIt != actEnd; ++actIt)
		{
			Action& act = actIt->second;
			draw_rectangle(CL_Rectf(act.x, act.y, act.x + 10, act.y + 10), correctionColor);
		}
		for (ActionHistory::iterator actIt = ship->actionList.begin(), actEnd = ship->actionList.end();
			actIt != actEnd; ++actIt)
		{
			// Draw history buffers
			Action& act = actIt->second;
			draw_rectangle(CL_Rectf(act.x, act.y, act.x + 10, act.y + 10), drawColor);

			CommandHistory::iterator whatCommand = ship->commandList.find(actIt->first);
			if (whatCommand != ship->commandList.end())
			{
				CL_String commandStr = cl_format("%1 %2%3%4%5", 
					(unsigned int)actIt->first,
					(whatCommand->second.up ? "up" : ""),
					(whatCommand->second.down ? "down" : ""),
					(whatCommand->second.left ? "left" : ""),
					(whatCommand->second.right ? "right" : ""));
				m_gc.draw_text((int)act.x-16, (int)act.y-2, commandStr);
			}
		}
		draw_rectangle(CL_Rectf(ship->x, ship->y, ship->x + 10, ship->y + 10), CL_Colorf::darkblue);
		m_gc.draw_text((int)ship->x, (int)ship->y + 14, cl_format("%1", (unsigned int)ship->currentTick));

		ship->commandList.clear();
	}

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
			std::cout << "Another player has joined the current server" << std::endl;
		}

		if (p->GetType() == MTID_REMOVEPLAYER)
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
				m_Network->Send(true, MTID_ADDPLAYER, (char *)bits.GetData(), bits.GetNumberOfBytesUsed(),
					FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
			}
		}

		if (p->GetType() == MTID_ADDPLAYER)
		{
			ObjectID object_id;
			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
			bits.Read(object_id);

			if (ships.find(object_id) == ships.end())
				ships[object_id] = ShipPtr(new Ship(object_id));
			else
				std::cout << "Server asked us to re-add player. Request ignored. ID: " << object_id << std::endl;

			std::cout << "New Player: ID: " << object_id << std::endl;
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

			std::cout << "Add Allowed: ID: " << object_id << " At: " << m_CommandNumber << std::endl;
		}

		if (p->GetType() == MTID_STARTTICK)
		{
			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
			bits.Read(m_CommandNumber);
		}

		if (p->GetType() == MTID_CHALL)
			std::cout << "Received: '" << p->GetDataString() << "' at " << p->GetTime() << " ticks" << std::endl;

		if (p->GetType() == MTID_CORRECTION)
		{
			ObjectID object_id;
			unsigned long commandNumber;

			RakStream bits(p);
			bits.Read(object_id);

			ShipMap::iterator _who = ships.find(object_id);
			if (_who != ships.end())
			{
				ShipPtr ship = _who->second;
				bits.Read(commandNumber);

				float x = 0.f, y = 0.f;
				bits.Read(x);
				bits.Read(y);

				ship->cullActions();
				ship->cullCommands(commandNumber);
				correct(ship, commandNumber, Action(x, y));
			}
		}

		if (p->GetType() == MTID_ENTITYMOVE)
		{
			ObjectID object_id;
			unsigned long commandNumber;

			RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
			bits.Read(object_id);

			bits.Read(commandNumber);

			m_LatestReceivedTick = fe_max(commandNumber, m_LatestReceivedTick);

			ShipMap::iterator _where = ships.find(object_id);
			if (_where != ships.end())
			{
				ShipPtr ship = _where->second;

				Command received;
				bits.Read(received.up);
				bits.Read(received.down);
				bits.Read(received.left);
				bits.Read(received.right);

				CommandHistory::iterator checkCmd = ship->commandList.lower_bound(commandNumber);
				if (checkCmd != ship->commandList.end() && received != checkCmd->second)
					std::cout << commandNumber << " Server out of sync" << std::endl;

				float x = 0.f, y = 0.f;
				bits.Read(x);
				bits.Read(y);

				ship->x = x;
				ship->y = y;
				ship->interp_x.update(x);
				ship->interp_y.update(y);

				//if (is_local(ship, myShips))
				//{
				//	//ship->cullActions();
				//	ship->cullCommands(commandNumber);
				//	if (ship->checkAction(commandNumber, Action(x, y), 1.0f))
				//		correct(ship, commandNumber, Action(x, y));
				//}
				//else
				//{
				//	if (ship->mostRecentCommand < commandNumber)
				//	{
				//		ship->mostRecentCommand = commandNumber;
				//		/*ship->net_x = x;
				//		ship->net_y = y;*/
				//		
				//		//! \todo build interpolation into a network var class (replace ship::x/ship::y with said class)
				//		if (m_Interpolate && (!ship->local || !(ship->up && ship->down && ship->left && ship->right)))
				//		{
				//			if (abs(m_Tightness - s_DefaultTightness) < 0.009f && abs(ship->x - x) > 0.1f)
				//				m_Tightness = s_SmoothTightness;

				//			ship->x = ship->x + (x - ship->x) * m_Tightness;
				//			ship->y = ship->y + (y - ship->y) * m_Tightness;

				//			m_Tightness += (s_DefaultTightness - m_Tightness) * 0.01f;
				//		}
				//		else
				//		{
				//			ship->x = x;
				//			ship->y = y;
				//		}
				//	}
				//}

				//if (commandNumber > m_CommandNumber)
				//	m_CommandNumber = commandNumber;
			}
		}

		m_Network->DeallocatePacket(p);
		p = NULL;
		p = m_Network->Receive();
	}
}

void TestApp::client_simulate(float dt)
{
	long frameTimeLeft = dt;
	while (frameTimeLeft >= split)
	{
		frameTimeLeft -= split;
		for (ShipMap::iterator it = myShips.begin(), end = myShips.end(); it != end; ++it)
		{
			ShipPtr myShip = it->second;

			get_input(myShip);

			// Predict simulation
			float up = myShip->up ? -1.f : 0.f;
			float down = myShip->down ? 1.f : 0.f;
			float left = myShip->left ? -1.f : 0.f;
			float right = myShip->right ? 1.f : 0.f;
			myShip->x += s_Speed * split * (left + right);
			myShip->y += s_Speed * split * (up + down);

			fe_clamp(myShip->x, 0.f, m_WorldWidth);
			fe_clamp(myShip->y, 0.f, m_WorldHeight);

			myShip->saveAction(m_CommandNumber);
			//myShip->saveCommand(m_CommandNumber);
		}

		++m_CommandNumber;
	} // ends while (frameTimeLeft)
}

void TestApp::client_updateAuthority(float dt)
{}

void TestApp::client_send(float dt)
{
	for (ShipMap::iterator it = myShips.begin(), end = myShips.end(); it != end; ++it)
	{
		ShipPtr myShip = it->second;

		bool importantCommand = false;
		if (!myShip->commandList.empty())
			importantCommand = myShip->commandList.newest().second != myShip->GetCommand();
		if (importantCommand || (myShip->sendDelay -= split) <= 0)
		{
			myShip->sendDelay = s_SendInterval;

			RakNet::BitStream bits;
			bits.Write(myShip->id);
			bits.Write(m_CommandNumber);
			if (!importantCommand)
				bits.Write(myShip->mostRecentCommand);

			bits.Write(myShip->up);
			bits.Write(myShip->down);
			bits.Write(myShip->left);
			bits.Write(myShip->right);

			bits.Write(myShip->x);
			bits.Write(myShip->y);

			// Actions only have to be saved for sent commands, since the server wont make corrections
			//  to actions it doesn't know we've taken!
			//myShip->saveAction(m_CommandNumber);
			myShip->saveCommand(m_CommandNumber);

			if (importantCommand)
			{
				myShip->mostRecentCommand = m_CommandNumber;
				std::cout << m_CommandNumber << " (important)" << std::endl;
			}

			m_Network->Send(true, importantCommand ? MTID_IMPORTANTMOVE : MTID_ENTITYMOVE, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(), 
				FusionEngine::HIGH_PRIORITY, importantCommand ? FusionEngine::RELIABLE_ORDERED : FusionEngine::UNRELIABLE_SEQUENCED, 1, serverHandle);
		}
	}
}

void TestApp::client_render(float dt)
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
				Action &act = actIt->second;
				shipSprite.set_color(CL_Colorf(0.99f, 1.0f, 0.8f, 0.4f));
				shipSprite.draw(m_SpriteBatch, act.x, act.y);
			}
		}
		shipSprite.set_color(CL_Colorf::white);
		shipSprite.draw(m_SpriteBatch, ship->x, ship->y);
		m_gc.draw_text((int)ship->x, (int)ship->y, cl_format("%1", ship->id));
	}

	m_gc.draw_text(290, 16, cl_format("Ping: %1", m_Network->GetPing(serverHandle)));
	m_gc.draw_text(250, 40, cl_format("Command: %1", (unsigned int)m_CommandNumber));

	m_SpriteBatch.flush();
	m_window.flip();
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


#endif