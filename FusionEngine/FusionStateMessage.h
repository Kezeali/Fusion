#ifndef Header_FusionEngine_StateMessage
#define Header_FusionEngine_StateMessage

#if _MSC_VER > 1000
#pragma once
#endif

namespace FusionEngine
{

	/*!
	 * \brief
	 * Allows states to request actions from the StateManager.
	 */
	class StateMessage
	{
	public:
		//! Types of messages
		/*!
		 * Hopefully the names are self explanatory. :)
		 *
		 * \remarks
		 * Use of REMOVESTATE is preffered over QUIT if you want to quit the game,
		 * as this ensures other states can finish their processes before the game
		 * ends. Not that I adhear to that rule :P
		 */
		enum StateMessageType { ADDSTATE, REMOVESTATE, QUEUESTATE, UNQUEUESTATE, QUIT };

	public:
		//! Basic constructor
		StateMessage();
		//! Constructor +type +data
		StateMessage(StateMessageType type, FusionState *data);

	public:
		//! Retreives the state type
		StateMessageType GetType() const;
		//! Retreives the custom data
		FusionState *GetData() const;

	protected:
		//! Stores the message type
		StateMessageType m_Type;
		//! Stores custom message data
		/*!
		 * \remarks MCS - Since all messages pass states ATM, this is typed as a
		 * FusionState, rather than a void*.
		 */
		FusionState *m_Data;

	};

}

#endif