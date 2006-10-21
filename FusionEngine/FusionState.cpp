
#include "FusionState.h"

#include "FusionStateMessage.h"
#include "FusionError.h"

using namespace FusionEngine;

StateMessage *FusionState::PopMessage()
{
	StateMessage *ret = m_Messages.front();
	m_Messages.pop_front();

	return ret;
}

void FusionState::_pushMessage(StateMessage *m)
{
	m_Messages.push_back(m);
}

Error *FusionState::GetLastError() const
{
	return m_LastError;
}