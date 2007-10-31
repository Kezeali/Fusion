#include "../FusionEngine/Common.h"

#include "../FusionEngine/FusionNetwork.h"
#include "../FusionEngine/FusionRakNetwork.h"

#include <RakNet/BitStream.h>

using namespace FusionEngine;


static const float MOVE_SPEED = 1;
static const float TICKTIME = 16; // Milis per tick - 16 comes out at ~60FPS, I think
static const int PRED_HISTORY = 1000;


static inline float TimeToTicks(float dt)
{
	return (int)( 0.5f + (float)(dt) / TICKTIME );
}

static inline float TicksToTime(float ticks)
{
	return TICKTIME * ticks;
}

// Returns the given time rounded to the nearest time which will fit evenly into ticks
static inline float RoundTimeToTickIntervals(float dt)
{
	//essentually: return TicksToTime(TimeToTicks(dt))
	return TICKTIME * TimeToTicks(dt);
}


typedef std::map<NetHandle, Ship*> ClientShipMap;
typedef std::map<ObjectID, Ship*> ShipMap;

class Ship;
struct Ship::State;
struct Ship::Input;

struct Move
{
	// ?
	int command_number;
	// Tick at which the move occors
	int tick;
	Ship::State state;
	Ship::Input input;
};

class Ship
{
	friend class Environment;
public:
	struct State
	{
		float xvel, yvel;
		int x, y;
		State()
		{
			x = y = 0;
			xvel = yvel = 0.0f;
		}

		bool operator==(const State &other) const
		{
			return x==other.x && y==other.y &&
				xvel==other.xvel && yvel==other.yvel;
		}

		/// inequality operator (primary quantities only)
		bool operator!=(const State &other) const
		{
			return !(*this==other);
		}

	};

	struct Input
	{
		bool up, down, left, right;
		Input()
		{
			up = down = left = right = false;
		}
	};

public:
	Ship()
	{
		m_Moves.resize(PRED_HISTORY);
		m_ImportantMoves.resize(PRED_HISTORY);
	}

	// Legacy?
	Ship(ObjectID id)
		: m_ID(id)
	{
		m_Moves.resize(PRED_HISTORY);
		m_ImportantMoves.resize(PRED_HISTORY);
	}

	Ship(int size)
	{
		m_Moves.resize(size);
		m_ImportantMoves.resize(size);
	}


	void SaveMove(const Move &move)
	{

		// determine if important move

		bool important = true;

		if (!moves.empty())
		{
			const Move &previous = m_Moves.newest();

			important = move.input.left!=previous.input.left || 
				move.input.right!=previous.input.right ||
				move.input.up!=previous.input.up ||
				move.input.down!=previous.input.down;
		}

		if (important)
			m_ImportantMoves.add(move);

		// add move to history

		m_Moves.add(move);
	}

	void SetMoveStateToCurrent(int i)
	{
		m_Moves[i].state = m_Current;
	}

	void ClearMoves(int to)
	{
		// discard out of date important moves
		while (!importantMoves->empty() && importantMoves->oldest().command_number < to)
			importantMoves->remove();

		// discard out of date moves
		while (!moves->empty() && moves->oldest().command_number < to)
			moves->remove();
	}

	void CommandAccepted(int cmd, const Ship::State& state)
	{
		ClearMoves(cmd);
		m_Moves.oldest().state = state;
	}

	bool ShouldPredict() const
	{
		return true;
	}

	/// get important moves in a std::vector form
	void ImportantMoveArray(std::vector<Move> &array)
	{
		const int size = m_ImportantMoves.size();

		array.resize(size);

		int i = m_ImportantMoves.tail;

		for (int j=0; j<size; j++)
		{
			array[j] = m_ImportantMoves[i];
			m_ImportantMoves.next(i);
		}
	}

	void Update(const Input& input)
	{
		m_Previous = m_Current;
		if (input.up)
			m_Current.y = MOVE_SPEED;
		if (input.down)
			m_Current.y = -MOVE_SPEED;
		if (input.left)
			m_Current.xvel = -MOVE_SPEED;
		if (input.right)
			m_Current.xvel = MOVE_SPEED;
	}

	void PhysicsSimulate()
	{
		m_Current.x += TIMESCALE * m_Current.xvel;
		m_Current.y += TIMESCALE * m_Current.yvel;

		m_SimulationTick++;
	}

	void Snap(const State& snap)
	{
		m_Previous = m_Current;
		m_Current = snap;
	}

	const State &GetState() const
	{
		return m_Current;
	}

	void SetInput(const Input& input)
	{
		m_Input = input;
	}

	const Input &GetInput() const
	{
		return m_Input;
	}

	void SetID(const ObjectID& id)
	{
		m_ID = id;
	}

	ObjectID GetID() const
	{
		return m_ID;
	}

private:
	ObjectID m_ID;

	State m_Previous;
	State m_Current;

	Input m_Input;

	float m_SimulationTick;

	int m_PredictedCommandCount;
	int m_LastAcknowlegedCommand;

protected:
	/// circular buffer class
	struct CircularBuffer
	{
		int head;
		int tail;

		CircularBuffer()
		{
			head = 0;
			tail = 0;
		}

		void resize(int size)
		{
			head = 0;
			tail = 0;
			moves.resize(size);
		}

		int size()
		{
			int count = head - tail;
			if (count<0)
				count += (int) moves.size();
			return count;
		}

		void add(const Move &move)
		{
			moves[head] = move;
			next(head);

		}

		void remove()
		{
			assert(!empty());
			next(tail);
		}

		Move& oldest()
		{
			assert(!empty());
			return moves[tail];
		}

		Move& newest()
		{
			assert(!empty());
			int index = head-1;
			if (index==-1)
				index = (int) moves.size() - 1;
			return moves[index];
		}

		bool empty() const
		{
			return head==tail;
		}

		void next(int &index)
		{
			index ++;
			if (index>=(int)moves.size()) 
				index -= (int)moves.size();
		}

		void previous(int &index)
		{
			index --;
			if (index<0)
				index += (int)moves.size();
		}

		Move& operator[](int index)
		{
			assert(index>=0);
			assert(index<(int)moves.size());
			return moves[index];
		}

	private:

		std::vector<Move> moves;
	};

protected:
	CircularBuffer m_Moves;                       ///< stores all recent moves
	CircularBuffer m_ImportantMoves;              ///< stores recent *important* moves

};

class Environment
{
public:
	Environment()
	{
	}

public:
	// Server update
	void Update(unsigned int dt)
	{
		for (ShipMap::iterator it = m_Ships.begin(); it != m_Ships.end(); ++it)
		{
			(*it).second->Update(m_Input);
		}
	}

	void Correct(unsigned int dt, int i)
	{
		for (ShipMap::iterator it = m_Ships.begin(); it != m_Ships.end(); ++it)
		{
			(*it).second->Update(m_Input);
			// Update history
			(*it).second->SetMoveToCurrentState(i);
		}
	}

	// Client side update
	void PredictUpdate(unsigned int dt)
	{
		for (ShipMap::iterator it = m_Ships.begin(); it != m_Ships.end(); ++it)
		{
			Ship* ship = (*it).second;
			if (ship->ShouldPredict())
			{
				(*it).second->Update(m_Input);

				Move move;
				move.command_number = m_CommandNumber;
				move.ticks = TimeToTicks(dt);
				move.input = m_Input;
				move.state = ship->GetState();
				(*it).second->SaveMove(move);
			}
		}
	}

	// TODO: Put all command counts other than latest command in the ship objects
	int m_PredictedCommandCount;
	void PerformPrediction(int latest_command)
	{
		int commandDelta = latest_command - acknowledged_command;

		if (commandDelta != m_PredictedCommandCount)
		{
		}

		for (int i = m_PredictedCommandCount; i < latest_command; i++)
		{
			for (ShipMap::iterator it = m_Ships.begin(); it != m_Ships.end(); ++it)
			{
				Ship* ship = (*it).second;

				if (ship->m_Moves.empty())
					return;

				// compare correction state with move history state
				if (ship->GetState() != ship->m_Moves.oldest().state)
				{
					// discard corrected move
					moves->remove();

					// save current (local) scene data
					unsigned int localTime = m_CommandNumber;
					Ship::Input localInput = m_Input;

					// rewind to correction and replay moves
					m_CommandNumber = cmd_num;
					m_Input = input;
					m_Ship->Snap(state);

					m_Replaying = true;

					int i = moves->tail;

					while (i!=moves->head)
					{
						while ( < m_Moves[i].)
							Update(TIMESCALE, i);

						m_Input = m_Moves[i].input;
						m_Moves[i].state = m_Ship->GetState();
						m_Moves->next(i);
					}

					Update(TIMESCALE);

					m_Replaying = false;

					// restore saved input

					m_Input = localInput;
				}
			}
		}
	}

protected:
	//typedef std::map<ObjectID, History> ObjectHistoryMap;
	//ObjectHistoryMap m_Histories;

	ShipMap m_Ships;
	Ship::Input m_Input;

};

//typedef std::vector<ObjectID> ObjectList;
//typedef std::map<NetHandle, ObjectList> ClientObjectListMap
//typedef std::map<ObjectID, Ship*> ObjectShipMap;


class TestApp : public CL_ClanApplication
{
	CL_CommandLine m_Argp;

	ClientShipMap m_ClientShips;
	Environment m_Ships;

	CL_Surface *m_ShipSurface;

	int m_CommandNumber;
	bool m_Replaying;

	Network* m_Network;

	Ship* AddShip(const NetHandle& handle, ObjectID oid)
	{
		return new Ship(oid);
	}

	virtual int main(int argc, char **argv)
	{
		CL_SetupDisplay disp_setup;
		CL_SetupGL gl_setup;

		CL_ConsoleWindow console("Net Test");
		console.redirect_stdio();

		CL_DisplayWindow display("Net Test: Display", 640, 480);

		m_Argp.add_option('c', "client", "Host name / IP", "Connects to the given server");
		m_Argp.add_option('s', "server", "Max Players", "Starts a server");
		m_Argp.parse_args(argc, argv);

		bool client = false, server = false;
		int maxPlayers = 16;
		std::string host = "127.0.0.1";
		m_CommandNumber = 0;
		m_Replaying = false;

		
		while (m_Argp.next())
		{
			switch (m_Argp.get_key()) 
			{
			case 'c':
				client = true;
				host = m_Argp.get_argument();
				break;
			case 's':
				server = true;
				maxPlayers = CL_String::to_int(m_Argp.get_argument());
				break;

			default:
				std::cout << "Unknown commandline: " << m_Argp.get_key() << " " << m_Argp.get_argument() << std::endl;
				std::cout << "Using default settings..." << std::endl;
				break;
			}
		}

		if (!client && !server)
		{
			std::cout << "Press 'S' in the display window to start as a server" << std::endl;
			std::cout << "Press 'C' in the display window to start as a client" << std::endl;
			while (1)
			{
				if (CL_Keyboard::get_keycode('S'))
				{
					server = true;
					break;
				}
				else if (CL_Keyboard::get_keycode('C'))
				{
					client = true;
					break;
				}
				CL_System::keep_alive(50);
			}
		}

		m_Network = new RakNetwork();

		CL_ResourceManager resources("font.xml");
		CL_Font font1("Font1", &resources);

		if (server)
		{
			m_Network->Startup(maxPlayers, 40001, maxPlayers);

			IPacket* p;

			try
			{
				int sendDelay = 0;
				unsigned int lastframe = CL_System::get_time();
				unsigned int split = 0;
				// Loop thing
				while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
				{
					display.get_gc()->clear(CL_Color(180, 220, 255));

					split = CL_System::get_time() - lastframe;
					lastframe = CL_System::get_time();

					p = NULL;
					p = m_Network->Receive();
					if (p!=NULL)
					{
						if (p->GetType()==ID_CONNECTION_REQUEST)
							std::cout << "Connection request" << std::endl;
						if (p->GetType()==ID_NEW_INCOMING_CONNECTION)
						{
							std::cout << "New incomming connection" << std::endl;
							// Here I use m_CommandNumber because it's the easyiest way to get a UID
							//m_Clients[p->GetSystemHandle()].push_back(m_CommandNumber);
							//m_ClientShips[m_CommandNumber] = new Ship();
						}

						if (p->GetType() == ID_CONNECTION_LOST || p->GetType() == ID_DISCONNECTION_NOTIFICATION)
						{
							ClientShipMap::iterator it = m_ClientShips.find(p->GetSystemHandle());
							if (it != m_ClientShips.end())
								delete (*it).second;
							//for (ClientShipMap::iterator it = m_ClientShips.begin(); it != m_ClientShips.end(); ++it)
							//	delete (*it).second;
						}

						if (p->GetType() == MTID_CHALL)
							std::cout << "Server received: '" << p->GetDataString() << "' at " << p->GetTime() << " ticks" << std::endl;

						if (p->GetType() == MTID_ADDPLAYER)
						{
							m_ClientShips[p->GetSystemHandle()] = new Ship(m_CommandNumber);

							// Send the verification
							RakNet::BitStream bits;
							bits.Write(m_CommandNumber);
							m_Network->Send(false, MTID_ADDALLOWED, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
								FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());

							// Tell all the other clients to add this object
							for (ClientShipMap::iterator it = m_ClientShips.begin(); it != m_ClientShips.end(); ++it)
							{
								NetHandle clientHandle = (*it).first;
								Ship* ship = (*it).second;
								// Tell the new player about all the old players
								if (ship->GetID() != m_CommandNumber)
								{
									RakNet::BitStream temp;
									temp.Write(ship->GetID());
									m_Network->Send(true, MTID_ADDPLAYER, (char*)temp.GetData(), temp.GetNumberOfBytesUsed(),
										FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());
								}
								// Tell all the old players about the new player
								if (clientHandle != p->GetSystemHandle())
									m_Network->Send(true, MTID_ADDPLAYER, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
									FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, clientHandle);
							}
						}

						if (p->GetType() == MTID_SHIPFRAME)
						{
							ObjectID object_id;
							int time;
							Ship::Input input;
							Ship::State state;

							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
							bits.Read(object_id);

							bits.Read(time);

							bits.Read(input.up);
							bits.Read(input.down);
							bits.Read(input.left);
							bits.Read(input.right);

							//bits.Read(state.x);
							//bits.Read(state.y);

							m_ClientShips[p->GetSystemHandle()]->SetInput(input);
						}

						m_Network->DeallocatePacket(p);
					}


					for (ClientShipMap::iterator it = m_ClientShips.begin(); it != m_ClientShips.end(); ++it)
					{
						Ship* ship = (*it).second;

						// Simulate
						ship->Update(ship->GetInput(), TIMESCALE);

						// Draw debug info
						display.get_gc()->draw_rect(CL_Rectf(ship->GetState().x, ship->GetState().y, ship->GetState().x + 10, ship->GetState().y + 10), CL_Color::black);
						display.get_gc()->fill_rect(CL_Rectf(ship->GetState().x, ship->GetState().y, ship->GetState().x + 10, ship->GetState().y + 10), CL_Color::darkblue);

						if ((sendDelay -= split) <= 0)
						{
							sendDelay = 120;
							// Send data
							Ship::Input input = ship->GetInput();
							Ship::State state = ship->GetState();

							RakNet::BitStream bits;
							bits.Write(ship->GetID());
							bits.Write(m_CommandNumber);

							bits.Write(input.up);
							bits.Write(input.down);
							bits.Write(input.left);
							bits.Write(input.right);

							bits.Write(state.x);
							bits.Write(state.y);

							for (ClientShipMap::iterator client = m_ClientShips.begin(); client != m_ClientShips.end(); ++client)
							{
								NetHandle clientHandle = (*client).first;
								m_Network->Send(true, MTID_SHIPFRAME, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
									FusionEngine::HIGH_PRIORITY, FusionEngine::UNRELIABLE_SEQUENCED, 1, clientHandle);
							}
						} // end if (should send)
					}
				}


					m_CommandNumber++;
					display.flip();
					CL_System::keep_alive();
				}

			}
			catch (CL_Error& e)
			{
				std::cout << e.message << std::endl;
				console.wait_for_key();
			}
		}

		//////////////////////
		// Client
		else
		{
			m_Network->Startup(1, 40000, 1);

			IPacket* p;

			NetHandle serverHandle;

			
			m_ShipSurface = new CL_Surface("Body.png");
			m_Ship = new Ship();

			if (!m_Network->Connect(host, 40001))
				std::cout << "Connect failed";
			std::cout << "Connecting to host at " + host + "..." << std::endl;
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
						console.display_close_message();
						return 0;
					}

					m_Network->DeallocatePacket(p);
				}
				CL_System::sleep(30);
			}
			std::cout << "done!" << std::endl;


			try
			{
				int sendDelay = 0;
				unsigned int lastframe = CL_System::get_time();
				unsigned int split = 0;
				// Loop thing
				while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
				{
					display.get_gc()->clear(CL_Color(180, 220, 255));

					split = CL_System::get_time() - lastframe;
					lastframe = CL_System::get_time();

					p = NULL;
					p = m_Network->Receive();
					if (p!=NULL)
					{
						if (p->GetType() == ID_REMOTE_NEW_INCOMING_CONNECTION)
						{
							std::cout << "Another player has joined the current server" << std::endl;
						}

						if (p->GetType() == MTID_ADDPLAYER)
						{
							ObjectID object_id;
							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
							bits.Read(object_id);

							if (m_Ships.find(object_id) == m_Ships.end())
								m_Ships[object_id] = new Ship(object_id);
						}

						if (p->GetType() == MTID_ADDALLOWED)
						{
							ObjectID object_id;
							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
							bits.Read(object_id);

							m_Ship->SetID(object_id);

							m_Ships[object_id] = m_Ship;
						}

						if (p->GetType() == MTID_CHALL)
							std::cout << "Received: '" << p->GetDataString() << "' at " << p->GetTime() << " ticks" << std::endl;

						if (p->GetType() == MTID_SHIPFRAME)
						{
							ObjectID object_id;
							int time;
							Ship::Input input;
							Ship::State state;

							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
							bits.Read(object_id);

							bits.Read(time);

							bits.Read(input.up);
							bits.Read(input.down);
							bits.Read(input.left);
							bits.Read(input.right);

							bits.Read(state.x);
							bits.Read(state.y);

							if (object_id == m_Ship->GetID())
								correction(time, state, input);
							else
							{
								if (m_Ships.find(object_id) == m_Ships.end())
								{
									m_Ships[object_id]->SetInput(input);
									m_Ships[object_id]->Snap(state);
								}
							}
						}

						m_Network->DeallocatePacket(p);
					}

					if (CL_Keyboard::get_keycode(CL_KEY_RETURN))
					{
						m_Network->Send(true, MTID_CHALL, "Hi", 2, FusionEngine::LOW_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
					}


					if (CL_Keyboard::get_keycode(CL_KEY_UP))
						m_Input.up = true;
					else if (CL_Keyboard::get_keycode(CL_KEY_DOWN))
						m_Input.down = true;
					else
					{
						m_Input.up = false;
						m_Input.down = false;
					}

					if (CL_Keyboard::get_keycode(CL_KEY_LEFT))
						m_Input.left = true;
					else if (CL_Keyboard::get_keycode(CL_KEY_RIGHT))
						m_Input.right = true;
					else
					{
						m_Input.left = false;
						m_Input.right = false;
					}

					m_Ship->Update(m_Input, 1);

					if ((sendDelay -= split) <= 0)
					{
						sendDelay = 100;
						RakNet::BitStream bits;
						bits.Write(m_Ship->GetID());
						bits.Write(m_CommandNumber);

						bits.Write(m_Input.up);
						bits.Write(m_Input.down);
						bits.Write(m_Input.left);
						bits.Write(m_Input.right);

						bits.Write(m_Ship->GetState().x);
						bits.Write(m_Ship->GetState().y);

						m_Network->Send(true, MTID_SHIPFRAME, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(), 
							FusionEngine::HIGH_PRIORITY, FusionEngine::UNRELIABLE_SEQUENCED, 1, serverHandle);
					}

					for (ShipMap::iterator it = m_Ships.begin(); it != m_Ships.end(); ++it)
					{
						Ship *ship = (*it).second;
						m_ShipSurface->draw(ship->GetState().x, ship->GetState().y);
					}

					font1.draw(320, 16, "Ping: " + CL_String::from_int(m_Network->GetPing(serverHandle)));

					m_CommandNumber++;
					display.flip();
					CL_System::keep_alive();
				}

				m_Network->Disconnect();

			}
			catch (CL_Error& e)
			{
				std::cout << e.message << std::endl;
				console.wait_for_key();
			}
		}

		delete m_Network;
		delete m_Ship;
		delete m_ShipSurface;

		// The end?
		return 0;
	}
} app;
