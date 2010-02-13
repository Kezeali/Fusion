#include "Common.h"

#include "FusionScriptSound.h"


namespace FusionEngine
{

	///////////////
	// SoundOutput
	SoundOutput::SoundOutput(CL_SoundOutput &output)
		: m_SoundOutput(output)
	{
	}

	void SoundOutput::SetVolume(float volume)
	{
		m_SoundOutput.set_global_volume(volume);
	}

	float SoundOutput::GetVolume()
	{
		return m_SoundOutput.get_global_volume();
	}

	void SoundOutput::StopAll()
	{
		m_SoundOutput.stop_all();
	}

	void SoundOutput::SetModule(ModulePtr &module)
	{
		m_ModuleConnection.disconnect();
		m_ModuleConnection = module->ConnectToBuild( boost::bind(&SoundOutput::OnModuleBuild, this, _1) );
	}

	void SoundOutput::OnModuleBuild(BuildModuleEvent& ev)
	{
		if (ev.type == BuildModuleEvent::PreBuild)
		{
			ev.manager->RegisterGlobalObject("SoundOutput sound", this);
		}
	}

	void SoundOutput::Register(asIScriptEngine *engine)
	{
		int r;
		RefCounted::RegisterType<SoundOutput>(engine, "SoundOutput");
		r = engine->RegisterObjectMethod("SoundOutput",
			"void setVolume(float)",
			asMETHOD(SoundOutput,SetVolume), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("SoundOutput",
			"float getVolume()",
			asMETHOD(SoundOutput,GetVolume), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("SoundOutput",
			"void stopAll()",
			asMETHOD(SoundOutput,StopAll), asCALL_THISCALL);
	}

	/////////////////
	// SoundSession
	SoundSession::SoundSession(CL_SoundBuffer_Session &session)
		: m_Session(session),
		m_PausedPosition(0)
	{
	}

	void SoundSession::Play()
	{
		if (m_PausedPosition > 0)
			m_Session.set_position(m_PausedPosition);
		m_Session.play();
	}

	void SoundSession::Pause()
	{
		if (m_Session.is_playing())
		{
			m_PausedPosition = m_Session.get_position();
			m_Session.stop();
		}
	}

	void SoundSession::Stop()
	{
		m_PausedPosition = 0;
		m_Session.stop();
	}

	void SoundSession::Register(asIScriptEngine *engine)
	{
		int r;
		RefCounted::RegisterType<SoundSession>(engine, "SoundSession");
		r = engine->RegisterObjectMethod("SoundSession",
			"void play()",
			asMETHOD(SoundSession,Play), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("SoundSession",
			"void pause()",
			asMETHOD(SoundSession,Stop), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("SoundSession",
			"void stop()",
			asMETHOD(SoundSession,Stop), asCALL_THISCALL);
	}

	///////////////
	// SoundSample
	SoundSample::SoundSample(ResourceManager *res_man, const std::string &path, int priority, bool is_stream)
		: StreamedResourceUser(res_man, is_stream ? "AUDIO" : "AUDIO:STREAM", path, priority),
		m_Stream(is_stream)
	{
	}

	SoundSession* SoundSample::Play()
	{
		if (m_SoundBuffer.IsLoaded())
		{
			CL_SoundBuffer_Session session = m_SoundBuffer->play();
			return new SoundSession(session);
		}
		else return NULL;
	}

	void SoundSample::Stop()
	{
		//if (m_SoundBuffer.IsLoaded())
		//{
		//	m_SoundBuffer->stop();
		//}
	}

	SoundSession* SoundSample::CreateSession(bool looping)
	{
		if (m_SoundBuffer.IsLoaded())
		{
			CL_SoundBuffer_Session session = m_SoundBuffer->prepare(looping);
			return new SoundSession(session);
		}
		else return NULL;
	}

	void SoundSample::OnStreamOut()
	{
		//m_SoundBuffer->stop();
	}

	void SoundSample::OnResourceLoad(ResourceDataPtr resource)
	{
		m_SoundBuffer.SetTarget(resource);
	}

	void SoundSample::Register(asIScriptEngine *engine)
	{
		int r;
		RefCounted::RegisterType<SoundSample>(engine, "SoundSample");
		r = engine->RegisterObjectMethod("SoundSample",
			"void play()",
			asMETHOD(SoundSample,Play), asCALL_THISCALL);
		r = engine->RegisterObjectMethod("SoundSample",
			"void stop()",
			asMETHOD(SoundSample,Stop), asCALL_THISCALL);
	}

}
