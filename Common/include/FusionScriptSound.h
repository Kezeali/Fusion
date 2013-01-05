/*
  Copyright (c) 2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef H_FusionEngine_ScriptSound
#define H_FusionEngine_ScriptSound

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <boost/signals2/connection.hpp>
#include <string>
#include <ClanLib/sound.h>

#include "FusionRefCounted.h"
#include "FusionStreamedResourceUser.h"

#include "FusionResourcePointer.h"
#include "FusionScriptReference.h"

namespace FusionEngine
{

	//! clan::SoundOutput wrapper
	class SoundOutput : public RefCounted, noncopyable
	{
	public:
		SoundOutput(clan::SoundOutput &output);

		void SetVolume(float volume);
		float GetVolume();

		void StopAll();

		void RegisterSingleton(asIScriptEngine *engine);

		static void Register(asIScriptEngine *engine);

	protected:
		clan::SoundOutput m_SoundOutput;
	};

	//! Ref-counted wrapper for clan::SoundBuffer_Session
	class SoundSession : public RefCounted, noncopyable
	{
	public:
		SoundSession(clan::SoundBuffer_Session &session);

		void Play();
		void Pause();
		void Stop();

		static void Register(asIScriptEngine *engine);

	protected:
		clan::SoundBuffer_Session m_Session;

		int m_PausedPosition;

	};

	//! Wrapper for clan::SoundBuffer resource
	class SoundSample : public StreamedResourceUser, public RefCounted, noncopyable
	{
	public:
		SoundSample(ResourceManager *res_man, const std::string &path, int priority, bool is_stream);

		//! Plays the sample - returns a SoundSession
		SoundSession* Play();
		void Stop();

		//! Creates a sound session without playing
		SoundSession* CreateSession(bool looping);

		//! Stops playback
		void OnStreamOut();

		void OnResourceLoad(ResourceDataPtr resource);

		//! Stops all playback
		//void OnResourceUnload();

		static void Register(asIScriptEngine *engine);

	protected:
		ResourcePointer<clan::SoundBuffer> m_SoundBuffer;
		bool m_Stream;

		//boost::signals2::connection m_ResourceUnloadConnection;
	};

}

#endif