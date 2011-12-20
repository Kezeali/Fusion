/*
*  Copyright (c) 2011 Fusion Project Team
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

#ifndef H_FusionInputComponent
#define H_FusionInputComponent

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionEntityComponent.h"
#include "FusionInputComponentInterface.h"

namespace FusionEngine { namespace Components
{

	class Input : public IComponent, public IInput
	{
		friend class InputWorld;
		friend class InputTask;
	public:
		FSN_LIST_INTERFACES((IInput))

		Input();
		virtual ~Input();

	private:
		// IComponent
		std::string GetType() const { return "Input"; }

		//void OnSiblingAdded(const ComponentPtr& com);
		//void OnSiblingRemoved(const ComponentPtr& com);

		//bool SerialiseContinuous(RakNet::BitStream& stream);
		//void DeserialiseContinuous(RakNet::BitStream& stream);
		//bool SerialiseOccasional(RakNet::BitStream& stream, const SerialiseMode force_all);
		//void DeserialiseOccasional(RakNet::BitStream& stream, const SerialiseMode all);

		void SetInputName(const std::string& value) {};
		const std::string& GetInputName() const { return m_InputName; };

		std::string m_InputName;
	};

} }

#endif
