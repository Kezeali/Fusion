// This just creats a ResourceLoader and runs LoadShips(GetInstalledShips()),
//  LoadWeapons(GetInstalledWeapons(), etc on it. Then it exits, thus allowing
//  the ServerEnvironment to start.

	// Setup and run the package syncroniser
	PackSyncState *ps = new PackSyncState();
	ps->MakeServer(m_Hostname, m_Port);
	_pushMessage(new StateMessage(StateMessage::ADDSTATE, ps));