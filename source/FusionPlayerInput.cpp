/*
*  Copyright (c) 2009-2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "FusionStableHeaders.h"

#include "FusionPlayerInput.h"

#include <BitStream.h>

#ifdef _DEBUG
#include "FusionConsole.h"
#endif


namespace FusionEngine
{

	PlayerInput::PlayerInput()
		: m_Changed(false)
	{
	}

	PlayerInput::PlayerInput(const InputDefinitionLoader::InputDefinitionArray &inputs)
		: m_Changed(false)
	{
		for (auto it = inputs.cbegin(), end = inputs.cend(); it != end; ++it)
		{
			const InputDefinitionPtr &input = *it;
			//auto& inputState = m_Inputs[input->Name];
			//inputState.m_InputIndex = std::distance(inputs.cbegin(), it);
			InputState state;
			state.m_Name = input->Name;
			state.m_Analog = input->Analog;
			m_Inputs.push_back(state);
		}
	}

	void PlayerInput::SetActive(const std::string &input, bool active)
	{
		auto& inputsByName = m_Inputs.get<1>();
		auto inputEntry = inputsByName.find(input);
		if (inputEntry != inputsByName.end())
		{
			inputsByName.modify(inputEntry, [this, active](InputState &state) {
				setActive(state, active);
			});
		}
	}

	void PlayerInput::SetPosition(const std::string &input, float position)
	{
		auto& inputsByName = m_Inputs.get<1>();
		auto inputEntry = inputsByName.find(input);
		if (inputEntry != inputsByName.end())
		{
			inputsByName.modify(inputEntry, [this, position](InputState &state) {
				setPosition(state, position);
			});
		}
	}

	void PlayerInput::SetState(const std::string &input, bool active, float position)
	{
		auto& inputsByName = m_Inputs.get<1>();
		auto inputEntry = inputsByName.find(input);
		if (inputEntry != inputsByName.end())
		{
			inputsByName.modify(inputEntry, [this, active, position](InputState &state) {
				setActive(state, active);
				setPosition(state, position);
			});
		}
	}

	bool PlayerInput::IsActive(const std::string &input) const
	{
		auto& inputsByName = m_Inputs.get<1>();
		auto _where = inputsByName.find(input);
		if (_where != inputsByName.end())
			return _where->IsActive();
		else
		{
#ifdef _DEBUG
			SendToConsole("Tried to get state of undefined input");
#endif
			return false;
		}
	}

	float PlayerInput::GetPosition(const std::string &input) const
	{
		auto& inputsByName = m_Inputs.get<1>();
		auto _where = inputsByName.find(input);
		if (_where != inputsByName.end())
			return _where->GetValue();
		else
		{
#ifdef _DEBUG
			SendToConsole("Tried to get state of undefined input");
#endif
			return 0.f;
		}
	}

	bool PlayerInput::HasChanged() const
	{
		return m_Changed;
	}

	void PlayerInput::Serialise(RakNet::BitStream *stream) const
	{
		auto num = (unsigned short)m_Inputs.size();
		stream->Write(num);

		//std::count_if(m_Inputs.cbegin(), m_Inputs.cend(), [](const InputState& state)
		//{
		//	return state.m_ChangedSinceSerialised;
		//});

		for (auto it = m_Inputs.cbegin(), end = m_Inputs.cend(); it != end; ++it)
		{
			const InputState &state = *it;

			//stream->Write(state.m_InputIndex);
			stream->Write(state.IsActive());
			if (state.m_Analog)
				stream->Write(state.GetValue());

			//state.m_ChangedSinceSerialised = false;
		}

		m_Changed = false;
	}

	void PlayerInput::Deserialise(RakNet::BitStream *stream)
	{
		unsigned short num;
		stream->Read(num);

		if (num != m_Inputs.size())
			FSN_EXCEPT(InvalidArgumentException, "Peer has different number of inputs");

		for (auto it = m_Inputs.begin(), end = m_Inputs.end(); it != end; ++it)
		{
			//stream->Read(state.m_InputIndex);
			m_Inputs.modify(it, [this, stream](InputState& state)
			{
				bool active;
				stream->Read(active);
				float position;
				if (state.m_Analog)
					stream->Read(position);

				setActive(state, active);
				if (state.m_Analog)
					setPosition(state, position);
			});
		}

		m_Changed = false;
	}

	void PlayerInput::setActive(PlayerInput::InputState &state, bool active)
	{
		if (state.m_Active != active)
			m_Changed = true;
		state.m_Active = active;
	}

	void PlayerInput::setPosition(PlayerInput::InputState &state, float position)
	{
		if (!fe_fequal(state.m_Value, position, 0.0001f))
			m_Changed = true;
		state.m_Value = position;
	}

}
