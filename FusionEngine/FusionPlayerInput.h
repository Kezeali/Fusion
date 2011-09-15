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

#ifndef H_FusionEngine_EntityInput
#define H_FusionEngine_EntityInput

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include "FusionInputDefinitionLoader.h"
#include <RakNetTypes.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace FusionEngine
{

	class PlayerInput
	{
	public:
		struct InputState
		{
			InputState()
				: m_Active(false), m_Value(0.0f), m_ActiveRatio(0.0f), m_ActiveNow(false), m_ActiveFirst(false)
			{}
			InputState(const std::string& name, bool active, float value, float active_ratio)
				: m_Name(name), m_Active(active), m_Value(value), m_ActiveRatio(active_ratio), m_ActiveNow(active), m_ActiveFirst(active)
			{}

			uint16_t m_InputIndex;
			std::string m_Name;

			mutable bool m_ChangedSinceSerialised;

			bool m_Active; // Button / key was active during the step
			float m_Value; // Axis value, cursor position
			float m_ActiveRatio; // 0.0 to 1.0: indicates whether the button was pressed / released during the step

			// Used internally
			//unsigned char m_Step;
			bool m_ActiveNow; // Input is active right now
			bool m_ActiveFirst;
			//unsigned char m_TimesActivated; // Times activated during this step
			//unsigned char m_TimesChanged; // Times activated + times deactivated during this step

			inline bool IsActive() const { return m_Active; }
			inline float GetValue() const { return m_Value; }
			inline float GetActiveRatio() const { return m_ActiveRatio; }
		};

		typedef boost::multi_index_container<
			InputState,
			boost::multi_index::indexed_by<
			boost::multi_index::random_access<>, // index
			boost::multi_index::ordered_unique<boost::multi_index::member<InputState, std::string, &InputState::m_Name>> // name
			>> InputMap;

	public:
		PlayerInput();

		PlayerInput(const InputDefinitionLoader::InputDefinitionArray &inputs);

		void SetActive(const std::string &input, bool active);
		void SetPosition(const std::string &input, float value);

		void SetState(const std::string &input, bool active, float position);

		bool IsActive(const std::string &input) const;
		float GetPosition(const std::string &input) const;

		bool HasChanged() const;

		void Serialise(RakNet::BitStream *to) const;
		void Deserialise(RakNet::BitStream *from);

	protected:
		InputMap m_Inputs;

		mutable bool m_Changed;

		void setActive(InputState &state, bool active);
		void setPosition(InputState &state, float position);

	};

	typedef std::shared_ptr<PlayerInput> PlayerInputPtr;

}

#endif
