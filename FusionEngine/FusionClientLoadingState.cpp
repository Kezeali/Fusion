// The loading state should first connect to the server and find out where the 
//  fileserver is, then use that data to initialise the pack sync client.
//  once the pack sync client has run, it should create the resource loader
//  and ask the server what ships to load, the server will then send
//  VerifyPackage packets, with which the resource loader will verify
//  each package. LoadVerified will then be called on the resource loader.
//  Finally, the state will remove itself allowing the state manager to run
//  the next state in the queue, which should be ClientEnvironment.

// This class should run a gui, so at each stage indicated above, it can update
//  a progress bar. Obviously, it won't be able to tell how far completed the
//  PackSync is, but pack sync has it's own GUI, so that is a non-issue.
//  Also, it won't be able to get the progress of LoadVerified, but that
//  makes console messages, so the interested user can check those out if they
//  really want to :P Anyway, Source does it like that (doesn't show the progr-
//  ess of data loading, just says "Loading Resources"), so I feel assured.

	// In constructor
	FusionState(true)... // loading state is always blocking

	// Setup and run the package syncroniser
	PackSyncState *ps = new PackSyncState();
	ps->MakeClient(m_Hostname, m_Port);
	_pushMessage(new StateMessage(StateMessage::ADDSTATE, ps));

	// ... And when finished:
	_pushMessage(new StateMessage(StateMessage::REMOVESTATE, this));