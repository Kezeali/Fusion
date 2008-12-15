#include "../FusionEngine/Common.h"

#include "../FusionEngine/FusionNetwork.h"
#include "../FusionEngine/FusionRakNetwork.h"

#include <RakNet/BitStream.h>

#define BOOST_CB_DISABLE_DEBUG // Allows overwritten CB iterators to remain valid
#include <boost/circular_buffer.hpp>

using namespace FusionEngine;

const float s_DefaultTightness = 0.25f;
const float s_SmoothTightness = 0.1f;

const unsigned int s_SendInterval = 63;

template <class T>
class Record
{
public:
	typedef unsigned long time_type;

	Record()
		: time(0)
	{
	}
	Record(time_type _time, const T& _value)
		: time(_time), value(_value)
	{
	}

	time_type time;
	T value;
};

template<class T>
class History
{
public:
	typedef Record<T> record_type;
	typedef typename record_type::time_type time_type;

	typedef boost::circular_buffer<record_type> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;

	typedef std::tr1::unordered_map<time_type, size_type> timetoindex_map_type;

	History()
		: m_Front(0)
	{
	}

	History(size_type _size)
		: m_Front(0)
	{
		m_Data.set_capacity(_size);
		//m_TimeToIndex.resize(_size);
	}

	void set_capacity(size_type _size)
	{
		m_Data.set_capacity(_size);
		//m_TimeToIndex.resize(_size);
	}

	record_type& operator[](size_type _Keyval)
	{
		return m_Data[_Keyval];
	}

	iterator find(time_type _time)
	{
		//if (m_Data.empty())
		//	return m_Data.end();

		//size_type index = m_TimeToIndex[_time];
		//if (index >= m_Data.size())
		//	return m_Data.end();

		//return m_Data.begin() + index;
		
		if (m_Data.empty())
			return m_Data.end();

		size_type test = m_Data.size();
		if (m_Data.size() == 1)
			return m_Data.begin();

		size_type low = 0;
		size_type high = m_Data.size() - 1;
		size_type mid = (low + high) / 2;
		iterator midrecord;
		while (low <= high)
		{
			mid = (low + high) / 2;
			midrecord = m_Data.begin() + mid;
			if (midrecord->time > _time)
			{
				if (mid == 0) break;
				high = mid - 1;
			}
			else if (midrecord->time < _time)
				low = mid + 1;
			else
				return midrecord; // found
		}
		return m_Data.end(); // not found
	}

	iterator find_closest(time_type _time)
	{
		if (m_Data.empty())
			return m_Data.end();

		size_type test = m_Data.size();
		if (m_Data.size() == 1)
			return m_Data.begin();

		size_type low = 0;
		size_type high = m_Data.size() - 1;
		size_type mid = (low + high) / 2;
		iterator midrecord;
		while (low <= high)
		{
			mid = (low + high) / 2;
			midrecord = m_Data.begin() + mid;
			if (midrecord->time > _time)
			{
				if (mid == 0) break;
				high = mid - 1;
			}
			else if (midrecord->time < _time)
				low = mid + 1;
			else
				return midrecord; // found
		}
		return midrecord; // not found (return closest)
	}

	void push(time_type _time, const T& _value)
	{
		m_Data.push_front(record_type(_time, _value));
		//m_TimeToIndex[_time] = m_Front;
		m_Front = fe_wrap(m_Front + 1, (size_type)0, m_Data.capacity()); // This figures out where the next record will go in the circular buffer
	}

	// Pops off records until 'back' is at the given time
	void pop(time_type _time)
	{
		if (m_Data.empty())
			return;

		while (back().time < _time)
		{
			if (m_Data.size() == 1) break;
			m_Data.pop_back();
		}
	}

	void pop()
	{
		m_Data.pop_back();
	}

	void pop_front(time_type _time)
	{
		if (m_Data.empty())
			return;

		while (front().time > _time)
		{
			if (m_Data.size() == 1) break;
			m_Data.pop_front();
		}

		fe_clamp(m_Front, (size_type)0, m_Data.size());
	}

	void erase_after(iterator _front)
	{
		if (m_Data.empty() || _front == m_Data.end())
			return;

		m_Data.erase(_front + 1, m_Data.end());

		fe_clamp(m_Front, (size_type)0, m_Data.size());
	}

	bool empty()
	{
		return m_Data.empty();
	}

	size_type size()
	{
		return m_Data.size();
	}

	void clear()
	{
		m_Data.clear();
	}

	record_type& back()
	{
		return m_Data.back();
	}

	record_type& front()
	{
		return m_Data.front();
	}

	iterator begin()
	{
		return m_Data.begin();
	}

	const_iterator begin() const
	{
		return m_Data.begin();
	}

	iterator end()
	{
		return m_Data.end();
	}

	const_iterator end() const
	{
		return m_Data.end();
	}

	void previous(iterator &it)
	{
		if (it == begin())
			it = end() - 1;
		else
			--it;
	}
	
	size_type m_Front;
	container_type m_Data;
	timetoindex_map_type m_TimeToIndex;
};

class TestApp : public CL_ClanApplication
{
	CL_CommandLine m_Argp;

	Network* m_Network;

	bool m_Interpolate;
	float m_Tightness;
	
	struct Action
	{
		float x, y;
		bool valid;
		Action()
			: x(0.f), y(0.f), valid(false)
		{
		}
		Action(float _x, float _y)
			: x(_x), y(_y), valid(true)
		{
		}
	};
	typedef History<Action> ActionHistory;
	struct Command
	{
		Command()
			: up(false), down(false), left(false), right(false)
		{
		}

		Command(bool _up, bool _down, bool _left, bool _right)
			: up(_up), down(_down), left(_left), right(_right)
		{
		}

		bool up, down, left, right;
	};
	typedef History<Command> CommandHistory;

	class Ship
	{
	public:
		int sendDelay;
		ObjectID id;
		float x, y, net_x, net_y;
		bool up, down, left, right;

		unsigned long mostRecentCommand;
		//unsigned int currentCommand;
		ActionHistory actionList;
		CommandHistory commandList;
		ActionHistory::iterator currentAction;
		CommandHistory::iterator currentCommand;

		Ship()
			: id(0),
			x(0.f), y(0.f),
			net_x(0.f), net_y(0.f),
			sendDelay(0),
			mostRecentCommand(0)
		{
			actionList.set_capacity(1000);
			commandList.set_capacity(1000);
			currentAction = actionList.begin();
			currentCommand = commandList.begin();
		}

		Ship(ObjectID _id)
			: id(_id),
			x(0.f), y(0.f),
			net_x(0.f), net_y(0.f),
			sendDelay(0),
			mostRecentCommand(0)
		{
			actionList.set_capacity(1000);
			commandList.set_capacity(1000);
			currentAction = actionList.begin();
			currentCommand = commandList.begin();
		}

		void saveCommand(unsigned long tick)
		{
			commandList.push(tick, Command(up, down, left, right));
		}

		void rewindCommand(unsigned long tick)
		{
			if (commandList.empty())
				return;

			currentCommand = commandList.find_closest(tick);
			if (currentCommand == commandList.end())
				return;

			if (currentCommand->time > tick && currentCommand != commandList.begin())
				--currentCommand;

			// Confirm the found command
			//if (currentCommand->time > tick)
			//	currentCommand = commandList.begin();
			//for (CommandHistory::iterator it = currentCommand, end = commandList.end(); it != end && it->time < tick; ++it)
			//{
			//	// Find the command closest to the given tick
			//	if (tick - it->time < tick - currentCommand->time)
			//		currentCommand = it;
			//}
			//if (currentCommand == commandList.end())
			//	return;

			currentCommand = commandList.begin();
			for (CommandHistory::iterator it = currentCommand, end = commandList.end(); it != end && it->time < tick; ++it)
			{
				// Find the command closest to the given tick
				if (tick - it->time < tick - currentCommand->time)
					currentCommand = it;
			}
			if (currentCommand == commandList.end())
				return;

			Command &command = currentCommand->value;
			up = command.up;
			down = command.down;
			left = command.left;
			right = command.right;
		}

		void nextCommand()
		{
			if (commandList.empty())
				return;

			//commandList.next(currentCommand);
			if (++currentCommand == commandList.end())
				return;

			Command &command = currentCommand->value;
			up = command.up;
			down = command.down;
			left = command.left;
			right = command.right;
		}

		// Finds the command closest to the given time
		void nextCommand(unsigned long tick)
		{
			if (commandList.empty())
				return;

			if (currentCommand == commandList.end())
				return;

			//CommandHistory::iterator end = currentCommand;
			//commandList.previous(end);
			//CommandHistory::iterator nextCommand = currentCommand;
			//commandList.next(nextCommand);
			//while (nextCommand != end && nextCommand->time < tick)
			//{
			//	// Find the command closest to the given tick
			//	if (tick - nextCommand->time < tick - currentCommand->time)
			//		currentCommand = nextCommand;
			//	commandList.next(nextCommand);
			//}

			// Make sure we start the search before the requested tick
			if (currentCommand->time > tick)
				currentCommand = commandList.begin();

			for (CommandHistory::iterator it = currentCommand, end = commandList.end(); it != end && it->time < tick; ++it)
			{
				// Find the command closest to the given tick
				if (tick - it->time < tick - currentCommand->time)
					currentCommand = it;
			}

			if (currentCommand == commandList.end())
				return;

			Command &command = currentCommand->value;
			up = command.up;
			down = command.down;
			left = command.left;
			right = command.right;
		}

		void saveAction(unsigned long tick)
		{
			actionList.push(tick, Action(x, y));
		}

		// Returns true if the stored action at the given tick is different to the given x, y values
		bool checkAction(unsigned long tick, float x, float y)
		{
			if (actionList.empty())
				return false;

			ActionHistory::iterator _where = actionList.find(tick);
			if (_where == actionList.end())
				return false;

			Action &action = _where->value;
			return !fe_fequal(action.x, x) || !fe_fequal(action.y, y);
		}

		// Returns true if a suitable action was found
		bool checkRewindAction(unsigned long tick, float x, float y)
		{
			if (actionList.empty())
				return false;

			currentAction = actionList.find_closest(tick);
			if (currentAction == actionList.end())
				return false;

			Action &action = currentAction->value;
			if (!fe_fequal(action.x, x, 0.5f) || !fe_fequal(action.y, y, 0.5f))
				return false;
			
			actionList.erase_after(currentAction);

			x = action.x;
			y = action.y;

			return true;
		}

		// Returns true if an action was found
		bool rewindAction(unsigned long tick)
		{
			if (actionList.empty())
				return false;

			currentAction = actionList.find(tick);
			if (currentAction == actionList.end())
				return false;

			actionList.erase_after(currentAction);
			Action &action = currentAction->value;
			x = action.x;
			y = action.y;

			return true;
		}

		/*void nextAction()
		{
			if (actionList.empty())
				return;

			Action &action = ++currentAction->value;
			x = action.x;
			y = action.y;
		}*/
	};

	unsigned long m_CommandNumber;

	// For server side
	typedef std::tr1::unordered_map<NetHandle, Ship> ClientShipMap;

	// For client side
	typedef std::tr1::unordered_map<ObjectID, Ship> ShipMap;

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

		// Simulation speed
		const unsigned int split = 16;

		if (server)
		{
			m_Network->Startup(maxPlayers, 40001, maxPlayers);

			IPacket* p;
			ClientShipMap clientShips;
			unsigned int nextId = 0;

			try
			{
				//int sendDelay = 0;
				unsigned int lastframe = CL_System::get_time();
				unsigned int frameTime = 0;
				// Loop thing
				while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
				{
					display.get_gc()->clear(CL_Color(180, 220, 255));

					frameTime += CL_System::get_time() - lastframe;
					if (frameTime > 80)
						frameTime = 80;
					lastframe = CL_System::get_time();

					p = NULL;
					p = m_Network->Receive();
					if (p!=NULL)
					{
						if (p->GetType()==ID_CONNECTION_REQUEST)
							std::cout << "Connection request" << std::endl;

						else if (p->GetType()==ID_NEW_INCOMING_CONNECTION)
						{
							std::cout << "New incomming connection" << std::endl;
						}

						else if (p->GetType() == ID_CONNECTION_LOST || p->GetType() == ID_DISCONNECTION_NOTIFICATION)
						{
							std::cout << "Connection Lost" << std::endl;

							ClientShipMap::iterator _where = clientShips.find(p->GetSystemHandle());
							if (_where != clientShips.end())
							{
								// Tell the other clients
								RakNet::BitStream bits;
								bits.Write(_where->second.id);
								for (ClientShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
								{
									m_Network->Send(false, MTID_REMOVEPLAYER, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
										FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, it->first);
								}

								clientShips.erase(_where);
							}
						}

						else if (p->GetType() == MTID_CHALL)
							std::cout << "Server received: '" << p->GetDataString() << "' at " << p->GetTime() << " ticks" << std::endl;

						else if (p->GetType() == MTID_ADDPLAYER)
						{
							// Send the verification
							RakNet::BitStream bits;
							bits.Write(m_CommandNumber + 1);
							bits.Write(nextId);
							m_Network->Send(false, MTID_ADDALLOWED, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
								FusionEngine::HIGH_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());

							std::cout << "Add Allowed: ID: " << nextId << " At: " << m_CommandNumber << std::endl;

							// Tell all the other clients to add this object
							for (ClientShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
							{
								NetHandle clientHandle = (*it).first;
								Ship &ship = (*it).second;

								// Tell the new player about all the old player being iterated
								RakNet::BitStream temp;
								temp.Write(ship.id);
								m_Network->Send(true, MTID_ADDPLAYER, (char*)temp.GetData(), temp.GetNumberOfBytesUsed(),
									FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, p->GetSystemHandle());

								// Tell all the old player being iterated about the new player
								m_Network->Send(true, MTID_ADDPLAYER, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
									FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, clientHandle);
							}

							// Create a ship for the new player
							clientShips[p->GetSystemHandle()] = Ship(nextId++);
						}

						else if (p->GetType() == MTID_SHIPFRAME)
						{
							ObjectID object_id;
							unsigned long time;

							const NetHandle &systemHandle = p->GetSystemHandle();
							Ship &ship = clientShips[systemHandle];

							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), true);
							bits.Read(object_id);

							if (object_id != ship.id)
								std::cout << "Warning: A client is sending data for another client's ship" << std::endl;

							bits.Read(time);

							bits.Read(ship.up);
							bits.Read(ship.down);
							bits.Read(ship.left);
							bits.Read(ship.right);

							ship.saveCommand(time);

							//ship.lastReceivedCommand = fe_max(time, ship.lastReceivedCommand);

							float x, y;

							bits.Read(x);
							bits.Read(y);

							// Lag compensation (check that the given ship is more-or-less in sync)
							if (ship.checkAction(time, x, y))
							{
								typedef std::list<Ship*> rewindList;
								rewindList rewindable;
								// The rewind can only happen if there is a suitable stored action
								if (ship.rewindAction(time))
								{
									//ship.rewindCommand(time);
									rewindable.push_back(&ship);

									// Rewind everyone
									for (ClientShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
									{
										Ship &rewindShip = it->second;
										if (rewindShip.id == ship.id) // out-of-sync ship has already been rewound
											continue;

										// We can only rewind ships that we have data for at the given tick
										if (rewindShip.rewindAction(time))
										{
											//rewindShip.clearActionsAfter(time);
											rewindShip.rewindCommand(time);
											rewindable.push_back(&(it->second));
										}
									}
									// Simulate up to the current time
									for (unsigned int t = time; t < m_CommandNumber; t++)
									{
										//ship.saveCommand(t);
										for (rewindList::iterator it = rewindable.begin(), end = rewindable.end(); it != end; ++it)
										{
											Ship *simShip = (*it);
											//if (simShip->id != ship.id)
											simShip->nextCommand(t);

											float up = simShip->up ? -1.f : 0.f;
											float down = simShip->down ? 1.f : 0.f;
											float left = simShip->left ? -1.f : 0.f;
											float right = simShip->right ? 1.f : 0.f;
											simShip->x += 0.3f * split * (left + right);
											simShip->y += 0.3f * split * (up + down);

											fe_clamp(simShip->x, 0.f, (float)display.get_width());
											fe_clamp(simShip->y, 0.f, (float)display.get_height());

											simShip->saveAction(t);
										}
									}
								}
							}
						}

						m_Network->DeallocatePacket(p);
					}


					long frameTimeLeft = frameTime;
					while (frameTimeLeft >= split)
					{
						frameTimeLeft -= split;
						for (ClientShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
						{
							Ship &ship = (*it).second;

							// Simulate
							float up = ship.up ? -1.f : 0.f;
							float down = ship.down ? 1.f : 0.f;
							float left = ship.left ? -1.f : 0.f;
							float right = ship.right ? 1.f : 0.f;
							ship.x += 0.3f * split * (left + right);
							ship.y += 0.3f * split * (up + down);

							fe_clamp(ship.x, 0.f, (float)display.get_width());
							fe_clamp(ship.y, 0.f, (float)display.get_height());

							//ship.saveCommand(m_CommandNumber);
							ship.saveAction(m_CommandNumber);
						}

						m_CommandNumber++;
					}

					for (ClientShipMap::iterator it = clientShips.begin(); it != clientShips.end(); ++it)
					{
						Ship &ship = (*it).second;

						// Draw debug info
						display.get_gc()->draw_rect(CL_Rectf(ship.x, ship.y, ship.x + 10, ship.y + 10), CL_Color::black);
						display.get_gc()->fill_rect(CL_Rectf(ship.x, ship.y, ship.x + 10, ship.y + 10), CL_Color::darkblue);

						if ((ship.sendDelay -= split) <= 0)
						{
							ship.sendDelay = s_SendInterval;
							// Send data

							RakNet::BitStream bits;
							bits.Write(ship.id);
							bits.Write(m_CommandNumber);

							bits.Write(ship.up);
							bits.Write(ship.down);
							bits.Write(ship.left);
							bits.Write(ship.right);

							bits.Write(ship.x);
							bits.Write(ship.y);

							for (ClientShipMap::iterator client = clientShips.begin(); client != clientShips.end(); ++client)
							{
								NetHandle clientHandle = (*client).first;
								m_Network->Send(true, MTID_SHIPFRAME, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(),
									FusionEngine::HIGH_PRIORITY, FusionEngine::UNRELIABLE_SEQUENCED, 1, clientHandle);
							}
						} // end if (should send)
					}

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

			m_Interpolate = true;
			m_Tightness = s_DefaultTightness;
			
			CL_Surface *shipSurface = new CL_Surface("Body.png");
			Ship *myShip = NULL;
			ShipMap ships;

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

			// Request a ship
			m_Network->Send(true, MTID_ADDPLAYER, "", 0,
				FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);

			try
			{
				int sendDelay = 0;
				unsigned int lastframe = CL_System::get_time();
				unsigned int frameTime = 0;
				// Loop thing
				while (!CL_Keyboard::get_keycode(CL_KEY_ESCAPE))
				{
					display.get_gc()->clear(CL_Color(180, 220, 255));

					frameTime += CL_System::get_time() - lastframe;
					if (frameTime > 64)
						frameTime = 64;
					lastframe = CL_System::get_time();

					p = NULL;
					p = m_Network->Receive();
					if (p!=NULL)
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

							if (myShip == NULL)
								break;

							if (object_id != myShip->id)
								ships.erase(object_id);
							else
								// For some reason the local player's ship has been deleted; request a new ship
								m_Network->Send(true, MTID_ADDPLAYER, "", 0,
									FusionEngine::MEDIUM_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
						}

						if (p->GetType() == MTID_ADDPLAYER)
						{
							ObjectID object_id;
							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
							bits.Read(object_id);

							if (myShip != NULL && object_id != myShip->id && ships.find(object_id) == ships.end())
								ships[object_id] = Ship(object_id);
						}

						if (p->GetType() == MTID_ADDALLOWED)
						{
							ObjectID object_id;
							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
							bits.Read(m_CommandNumber);
							bits.Read(object_id);

							ships[object_id] = Ship(object_id);
							myShip = &ships[object_id];

							std::cout << "Add Allowed: ID: " << object_id << " At: " << m_CommandNumber << std::endl;
						}

						if (p->GetType() == MTID_CHALL)
							std::cout << "Received: '" << p->GetDataString() << "' at " << p->GetTime() << " ticks" << std::endl;

						if (p->GetType() == MTID_SHIPFRAME)
						{
							ObjectID object_id;
							unsigned long commandNumber;

							RakNet::BitStream bits((unsigned char*)p->GetData(), p->GetLength(), false);
							bits.Read(object_id);

							bits.Read(commandNumber);

							Ship &ship = ships[object_id];

							bits.Read(ship.up);
							bits.Read(ship.down);
							bits.Read(ship.left);
							bits.Read(ship.right);

							float x = 0.f, y = 0.f;
							bits.Read(x);
							bits.Read(y);

							if (ship.mostRecentCommand < commandNumber)
							{
								ship.mostRecentCommand = commandNumber;
								ship.net_x = x;
								ship.net_y = y;

								//! \todo build interpolation into a network var class (replace ship.x/ship.y with said class)
								if (m_Interpolate && object_id == myShip->id)
								{
									if (abs(m_Tightness - s_DefaultTightness) < 0.009f && abs(ship.x - x) > 0.1f)
										m_Tightness = s_SmoothTightness;

									ship.x = ship.x + (x - ship.x) * m_Tightness;
									ship.y = ship.y + (y - ship.y) * m_Tightness;

									m_Tightness += (s_DefaultTightness - m_Tightness) * 0.01f;
								}
								else
								{
									ship.x = x;
									ship.y = y;
								}
							}
						}

						m_Network->DeallocatePacket(p);
					}

					if (CL_Keyboard::get_keycode(CL_KEY_RETURN))
					{
						m_Network->Send(true, MTID_CHALL, "Hi", 2, 
							FusionEngine::LOW_PRIORITY, FusionEngine::RELIABLE, 0, serverHandle);
					}

					if (m_Tightness > 0.1f && CL_Keyboard::get_keycode(CL_KEY_END))
						m_Tightness -= 0.1f;
					else if (m_Tightness < 0.9f && CL_Keyboard::get_keycode(CL_KEY_HOME))
						m_Tightness += 0.1f;

					if (CL_Keyboard::get_keycode('I'))
						m_Interpolate = true;
					else if (CL_Keyboard::get_keycode('K'))
						m_Interpolate = false;

					if (myShip == NULL)
						continue;

					if (CL_Keyboard::get_keycode(CL_KEY_UP))
						myShip->up = true;
					else if (CL_Keyboard::get_keycode(CL_KEY_DOWN))
						myShip->down = true;
					else
					{
						myShip->up = false;
						myShip->down = false;
					}

					if (CL_Keyboard::get_keycode(CL_KEY_LEFT))
						myShip->left = true;
					else if (CL_Keyboard::get_keycode(CL_KEY_RIGHT))
						myShip->right = true;
					else
					{
						myShip->left = false;
						myShip->right = false;
					}

					long frameTimeLeft = frameTime;
					while (frameTimeLeft >= split)
					{
						frameTimeLeft -= split;
						// Predict simulation
						float up = myShip->up ? -1.f : 0.f;
						float down = myShip->down ? 1.f : 0.f;
						float left = myShip->left ? -1.f : 0.f;
						float right = myShip->right ? 1.f : 0.f;
						myShip->x += 0.3f * split * (left + right);
						myShip->y += 0.3f * split * (up + down);

						m_CommandNumber++;
					}

					if ((myShip->sendDelay -= frameTime) <= 0)
					{
						myShip->sendDelay = s_SendInterval;

						RakNet::BitStream bits;
						bits.Write(myShip->id);
						bits.Write(m_CommandNumber);

						bits.Write(myShip->up);
						bits.Write(myShip->down);
						bits.Write(myShip->left);
						bits.Write(myShip->right);

						bits.Write(myShip->x);
						bits.Write(myShip->y);

						myShip->saveAction(m_CommandNumber);
						myShip->saveCommand(m_CommandNumber);

						m_Network->Send(true, MTID_SHIPFRAME, (char*)bits.GetData(), bits.GetNumberOfBytesUsed(), 
							FusionEngine::HIGH_PRIORITY, FusionEngine::UNRELIABLE_SEQUENCED, 1, serverHandle);
					}

					for (ShipMap::iterator it = ships.begin(); it != ships.end(); ++it)
					{
						Ship &ship = (*it).second;
						shipSurface->draw(ship.x, ship.y);
					}

					font1.draw(320, 16, "Ping: " + CL_String::from_int(m_Network->GetPing(serverHandle)));

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

			delete shipSurface;
		} // if client

		delete m_Network;

		// The end?
		return 0;
	}
} app;
