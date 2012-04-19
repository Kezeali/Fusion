/*
*  Copyright (c) 2012 Fusion Project Team
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

#ifndef H_FusionComponentProperty
#define H_FusionComponentProperty

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionPropertySignalingSystem.h"

#include <BitStream.h>

namespace FusionEngine
{
	
	//! Entity Component Property interface
	class IComponentProperty
	{
	public:
		virtual void AquireSignalGenerator(PropertySignalingSystem_t& system) = 0;

		virtual void Follow(PropertySignalingSystem_t& system, PropertyID id) = 0;

		//virtual void FireSignal() = 0;
		virtual void Synchronise() = 0;

		virtual void Serialise(RakNet::BitStream& stream) = 0;
		virtual void Deserialise(RakNet::BitStream& stream) = 0;
		virtual bool IsContinuous() const = 0;

		PropertyID GetID() const { return (int)this; }

		//virtual bool IsEqual(IComponentProperty*) const = 0;

		//bool operator==(IComponentProperty* other) const
		//{
		//	return this->IsEqual(other);
		//}
		//bool operator!=(IComponentProperty* other) const
		//{
		//	return !this->IsEqual(other);
		//}

		//void Register(asIScriptEngine* engine)
		//{
		//	int r;
		//	r = engine->RegisterObjectMethod("Prop", "SignalConnection @follow(const string &in)", asMETHOD(this_type, connect), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		//}
	};

}

#endif
