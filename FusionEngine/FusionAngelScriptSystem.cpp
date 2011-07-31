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

#include "FusionStableHeaders.h"

#include "FusionAngelScriptSystem.h"

#include "FusionAngelScriptComponent.h"
#include "FusionEntity.h"
#include "FusionException.h"
#include "FusionExceptionFactory.h"
#include "FusionScriptReference.h"
#include "FusionXML.h"

#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>

#include <regex>

namespace FusionEngine
{

	//! Exception while preprocessing a component script
	class PreprocessorException : public Exception
	{
	public:
		PreprocessorException(const std::string& description, const std::string& origin, const char* file, long line)
			: Exception(description, origin, file, line) {}
	};

	void ASScript::Yield()
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			ctx->Suspend();
		}
	}

	void ASScript::CreateCoroutine(asIScriptFunction *fn)
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			auto engine = ctx->GetEngine();

			if (fn == nullptr)
			{
				ctx->SetException("Tried to create a coroutine for a null function-pointer");
				return;
			}

			const auto objectType = fn->GetObjectType();
			const bool method = objectType != nullptr;

			if (method && objectType != m_ScriptObject.GetScriptObject()->GetObjectType())
			{
				const std::string thisTypeName = m_ScriptObject.GetScriptObject()->GetObjectType()->GetName();
				ctx->SetException(("Tried to create a coroutine for a method from another class. This class: " + thisTypeName + ", Method: " + fn->GetDeclaration()).c_str());
				return;
			}

			auto coCtx = engine->CreateContext();
			coCtx->Prepare(fn->GetId());
			if (method)
				coCtx->SetObject(m_ScriptObject.GetScriptObject());

			m_ActiveCoroutines.push_back(coCtx);
			coCtx->Release();
		}
	}

	void ASScript::CreateCoroutine(const std::string& functionName)
	{
		auto ctx = asGetActiveContext();
		if (ctx)
		{
			auto engine = ctx->GetEngine();

			int funcId = -1;
			bool method = false;

			std::string decl = "void " + functionName + "()";

			auto _where = m_ScriptMethods.find(decl);
			if (_where != m_ScriptMethods.end())
			{
				funcId = _where->second;
				method = true;
			}
			else
			{
				funcId = m_ScriptObject.GetScriptObject()->GetObjectType()->GetMethodIdByDecl(decl.c_str());
				if (funcId >= 0)
					method = true;
				else
				{
					funcId = m_Module->GetASModule()->GetFunctionIdByDecl(decl.c_str());
					method = false;
				}
			}
			if (funcId < 0)
			{
				// No function matching the decl
				ctx->SetException(("Function '" + decl + "' doesn't exist").c_str());
				return;
			}

			auto coCtx = engine->CreateContext();
			coCtx->Prepare(funcId);
			if (method)
				coCtx->SetObject(m_ScriptObject.GetScriptObject());

			m_ActiveCoroutines.push_back(coCtx);
			coCtx->Release();
		}
	}

	AngelScriptSystem::AngelScriptSystem(const std::shared_ptr<ScriptManager>& manager)
		: m_ScriptManager(manager)
	{
		auto engine = m_ScriptManager->GetEnginePtr();

		{
			int r = engine->RegisterFuncdef("void coroutine_t()"); FSN_ASSERT(r >= 0);
		}

		{
			int r;
			ASScript::RegisterType<ASScript>(engine, "ASScript");
			r = engine->RegisterObjectMethod("ASScript", "void yield()", asMETHOD(ASScript, Yield), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(coroutine_t @)", asMETHODPR(ASScript, CreateCoroutine, (asIScriptFunction*), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
			r = engine->RegisterObjectMethod("ASScript", "void createCoroutine(const string &in)", asMETHODPR(ASScript, CreateCoroutine, (const std::string&), void), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		}
	}

	ISystemWorld* AngelScriptSystem::CreateWorld()
	{
		return new AngelScriptWorld(this, m_ScriptManager);
	}

	AngelScriptWorld::AngelScriptWorld(IComponentSystem* system, const std::shared_ptr<ScriptManager>& manager)
		: ISystemWorld(system),
		m_ScriptManager(manager)
	{
		m_Engine = m_ScriptManager->GetEnginePtr();

		m_ASTask = new AngelScriptTask(this, m_ScriptManager);
	}

	AngelScriptWorld::~AngelScriptWorld()
	{
		delete m_ASTask;
	}

	std::vector<std::string> AngelScriptWorld::GetTypes() const
	{
		static const std::string types[] = { "ASScript" };
		return std::vector<std::string>(types, types + sizeof(types));
	}

	std::shared_ptr<IComponent> AngelScriptWorld::InstantiateComponent(const std::string& type)
	{
		return InstantiateComponent(type, Vector2::zero(), 0.f, nullptr, nullptr);
	}

	std::shared_ptr<IComponent> AngelScriptWorld::InstantiateComponent(const std::string& type, const Vector2&, float, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data)
	{
		if (type == "ASScript")
		{
			auto com = std::make_shared<ASScript>();
			return com;
		}
		return std::shared_ptr<IComponent>();
	}

	void AngelScriptWorld::OnPrepare(const std::shared_ptr<IComponent>& component)
	{
		// TODO: request the module from the resource manager here (it will either be building right now, if it had changed, or it the resource manager will begin loading it)
		component->Ready();
	}

	void AngelScriptWorld::OnActivation(const std::shared_ptr<IComponent>& component)
	{
		auto scriptComponent = std::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			m_ActiveScripts.push_back(scriptComponent);
		}
	}

	void AngelScriptWorld::OnDeactivation(const std::shared_ptr<IComponent>& component)
	{
		auto scriptComponent = std::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			// Find and remove the deactivated script
			auto _where = std::find(m_ActiveScripts.begin(), m_ActiveScripts.end(), scriptComponent);
			if (_where != m_ActiveScripts.end())
			{
				_where->swap(m_ActiveScripts.back());
				m_ActiveScripts.pop_back();
			}
		}
	}

	ISystemTask* AngelScriptWorld::GetTask()
	{
		return m_ASTask;
	}

	void AngelScriptWorld::MergeSerialisedDelta(const std::string& type, RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta)
	{
		if (type == "ASScript")
		{
			ASScript::DeltaSerialiser_t::copyChanges(result, current_data, delta);
		}
	}

	AngelScriptWorld::ComponentScriptInfo::ComponentScriptInfo()
	{
	}

	AngelScriptWorld::ComponentScriptInfo::ComponentScriptInfo(const ComponentScriptInfo &other)
		: ClassName(other.ClassName),
		Properties(other.Properties),
		UsedComponents(other.UsedComponents)
	{
	}

	AngelScriptWorld::ComponentScriptInfo::ComponentScriptInfo(ComponentScriptInfo &&other)
	{
		ClassName = std::move(other.ClassName);
		Properties = std::move(other.Properties);
		UsedComponents = std::move(other.UsedComponents);
	}

	AngelScriptWorld::ComponentScriptInfo& AngelScriptWorld::ComponentScriptInfo::operator= (const ComponentScriptInfo &other)
	{
		ClassName = other.ClassName;
		Properties = other.Properties;
		UsedComponents = other.UsedComponents;
		return *this;
	}

	AngelScriptWorld::ComponentScriptInfo& AngelScriptWorld::ComponentScriptInfo::operator= (ComponentScriptInfo &&other)
	{
		ClassName = std::move(other.ClassName);
		Properties = std::move(other.Properties);
		UsedComponents = std::move(other.UsedComponents);
		return *this;
	}

	AngelScriptTask::AngelScriptTask(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager)
		: ISystemTask(sysworld),
		m_AngelScriptWorld(sysworld),
		m_ScriptManager(script_manager)
	{
	}

	AngelScriptTask::~AngelScriptTask()
	{
	}

	static std::pair<asETokenClass, int> ParseNextToken(asIScriptEngine* engine, const std::string& script, size_t &pos)
	{
		int tokenLength;
		asETokenClass tokenClass = engine->ParseToken(script.data() + pos, script.size() - pos, &tokenLength);
		return std::make_pair(tokenClass, tokenLength);
	}

	static std::string ParsePreprocessorStringValue(asIScriptEngine* engine, const std::string& script, size_t &pos)
	{
		int tokenLength;
		asETokenClass tokenClass;
		std::tie(tokenClass, tokenLength) = ParseNextToken(engine, script, pos);

		if (tokenClass == asTC_WHITESPACE)
		{
			const char token = script[pos];
			pos += tokenLength;
			if (token != '\n' && token != '\r')
				std::tie(tokenClass, tokenLength) = ParseNextToken(engine, script, pos);
			else
				return ""; // end of line, no value
		}

		if (tokenClass == asTC_VALUE && tokenLength > 2 && script.at(pos) == '"')
		{
			std::string value = script.substr(pos + 1, tokenLength - 2);
			pos += tokenLength;
			return value;
		}

		return "";
	}

	static std::string ParsePreprocessorIdentifier(asIScriptEngine* engine, const std::string& script, size_t &pos)
	{
		int tokenLength;
		asETokenClass tokenClass;
		std::tie(tokenClass, tokenLength) = ParseNextToken(engine, script, pos);

		if (tokenClass == asTC_WHITESPACE)
		{
			const char token = script[pos];
			pos += tokenLength;
			if (token != '\n' && token != '\r')
				std::tie(tokenClass, tokenLength) = ParseNextToken(engine, script, pos);
			else
				return ""; // end of line, no value
		}

		if (tokenClass == asTC_IDENTIFIER)
		{
			std::string value = script.substr(pos, tokenLength);
			pos += tokenLength;
			return value;
		}

		return "";
	}

	static std::vector<std::string> ParsePreprocessorDirective(asIScriptEngine* engine, const std::string& script, size_t &pos)
	{
		std::vector<std::string> fields;
		int tokenLength;
		auto tokenClass = engine->ParseToken(script.data() + pos, script.size() - pos, &tokenLength);
		if (tokenClass == asTC_IDENTIFIER)
		{
			std::string token(script.data() + pos, script.data() + pos + tokenLength);
			fields.push_back(token);
			if (token == "uses")
			{
				pos += tokenLength;

				auto field = ParsePreprocessorIdentifier(engine, script, pos);
				while (!field.empty())
				{
					fields.push_back(field);
					field = ParsePreprocessorIdentifier(engine, script, pos);
				}
			}
		}
		
		return fields;
	}

	static void OverwriteCode(std::string& script, size_t start, size_t len)
	{
		FSN_ASSERT(script.length() >= start + len);
		for (auto character = script.begin() + start, end = character + len; character != end; ++character)
		{
			if (*character != '\n' && *character != '\r')
				*character = ' ';
		}
	}

	static void ParsePublicProperties(AngelScriptWorld::ComponentScriptInfo& scriptInfo, asIScriptEngine* engine, std::string& script, size_t &pos)
	{
		int tokenLength;
		while (pos < script.length() && script[pos] != ';' && script[pos] != '{')
		{
			engine->ParseToken(&script[pos], 0, &tokenLength);
			pos += tokenLength;
		}

		// No class definition?
		if (script[pos] == ';')
		{
			return;
		}

		//std::set<std::string> builtInTypes;
		std::regex r("^(?:bool|(?:[u]|)int(?:8|16|32|64|)|float|double)$");

		if (pos < script.length() && script[pos] == '{')
		{
			pos += 1;

			// Find the end of the statement block
			bool newStatement = false;
			int level = 1;
			while (level > 0 && pos < (int)script.size())
			{
				asETokenClass t = engine->ParseToken(&script[pos], 0, &tokenLength);
				if (t == asTC_KEYWORD)
				{
					if (script[pos] == '{')
					{
						level++;
						pos += tokenLength;
						continue;
					}
					else if (script[pos] == '}')
					{
						level--;
						newStatement = true;
						pos += tokenLength;
						continue;
					}
				}

				if (level == 1)
				{
					if(newStatement &&
						(t == asTC_IDENTIFIER
						|| (t == asTC_KEYWORD && std::regex_match(&script[pos], &script[pos] + tokenLength, r)))
						)
					{
						newStatement = false;

						size_t start = pos;
						pos += tokenLength;

						t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);

						// template types
						if (t == asTC_KEYWORD && script[pos] == '<')
						{
							pos += tokenLength;
							t = engine->ParseToken(&script[pos], 0, &tokenLength);

							if (t == asTC_WHITESPACE)
							{
								pos += tokenLength;
								t = engine->ParseToken(&script[pos], 0, &tokenLength);
							}

							if (t == asTC_IDENTIFIER || t == asTC_KEYWORD)
							{
								pos += tokenLength;
								t = engine->ParseToken(&script[pos], 0, &tokenLength);
							}
							else
							{
								pos += tokenLength;
								continue;
							}

							if (t == asTC_WHITESPACE)
							{
								pos += tokenLength;
							}

							if (script[pos] == '>')
							{
								++pos;
							}
							else
							{
								pos += tokenLength;
								continue;
							}
						}

						if (t == asTC_WHITESPACE)
						{
							pos += tokenLength;
							t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
						}

						if (t == asTC_KEYWORD)
						{
							if (script[pos] == '@')
							{
								pos += tokenLength;
								t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
							}
							else
							{
								pos += tokenLength;
								continue;
							}
						}

						if (t == asTC_WHITESPACE)
						{
							pos += tokenLength;
							t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
						}

						if (t == asTC_IDENTIFIER)
						{
							pos += tokenLength;

							if (t == asTC_WHITESPACE)
							{
								t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
								pos += tokenLength;
							}

							if (script[pos] == ';')
							{
								scriptInfo.Properties.push_back(script.substr(start, pos - start));
								newStatement = true;
								++pos;
								continue;
							}
						}
						else
							pos += tokenLength;
					}
					else if (t == asTC_KEYWORD && script[pos] == ';')
					{
						newStatement = true;
						++pos;
						continue;
					}
					else
						pos += tokenLength;
				}
				else
					pos += tokenLength;

				if (newStatement && t != asTC_WHITESPACE)
					newStatement = false; // No longer the first meaningful token on this line
			}
		}
		else
			pos += 1;
	}

	static bool EatToken(asIScriptEngine *engine, std::string &script, size_t &pos, asETokenClass token_class)
	{
		int tokenLength;
		auto tokenClassEncountered = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
		if (tokenClassEncountered == token_class)
		{
			pos += tokenLength;
			return true;
		}
		else
			return false;
	}

	static AngelScriptWorld::ComponentScriptInfo ParseComponentScript(asIScriptEngine* engine, std::string& script)
	{
		AngelScriptWorld::ComponentScriptInfo scriptInfo;

		size_t pos = 0;
		while (pos < script.size())
		{
			auto token = ParseNextToken(engine, script, pos);
			if (/*token.first == asTC_UNKNOWN && */script.at(pos) == '#')
			{
				size_t start = pos++;

				auto preprocessorDirective = ParsePreprocessorDirective(engine, script, pos);

				if (!preprocessorDirective.empty())
				{
					if (preprocessorDirective[0] == "uses")
					{
						if (preprocessorDirective.size() >= 2)
						{
							scriptInfo.UsedComponents.insert(std::make_pair(preprocessorDirective[1], (preprocessorDirective.size() > 2) ? preprocessorDirective[2] : ""));
						}
						else
						{
							size_t directiveLength = pos - start;
							if (directiveLength > 1 && (script[pos-1] == '\r' || script[pos-1] == '\n'))
								--directiveLength;
							FSN_EXCEPT(PreprocessorException, "Invalid #uses directive: '" + script.substr(start, directiveLength) + "'");
						}
					}
				}
				else
					FSN_EXCEPT(PreprocessorException, "Invalid (empty) preprocessor directive");

				OverwriteCode(script, start, pos - start);
			}
			else if (token.first == asTC_KEYWORD && script.substr(pos, token.second) == "class")
			{
				pos += token.second;
				bool componentClass = false;

				// Check for "\s+"
				token = ParseNextToken(engine, script, pos);
				if (token.first != asTC_WHITESPACE)
					FSN_EXCEPT(PreprocessorException, "Failed to pre-parse class interface");
				pos += token.second;
				// Check for a class name
				token = ParseNextToken(engine, script, pos);
				if (token.first != asTC_IDENTIFIER)
					FSN_EXCEPT(PreprocessorException, "Failed to pre-parse class interface");
				auto className = script.substr(pos, token.second);
				pos += token.second;

				// Check the base-class
				EatToken(engine, script, pos, asTC_WHITESPACE);

				token = ParseNextToken(engine, script, pos);
				if (token.first == asTC_KEYWORD && script[pos] == ':')
				{
					pos += token.second;

					EatToken(engine, script, pos, asTC_WHITESPACE);

					token = ParseNextToken(engine, script, pos);
					if (token.first == asTC_IDENTIFIER && script.substr(pos, token.second) == "ScriptComponent")
					{
						componentClass = true;
					}
					else
						FSN_EXCEPT(PreprocessorException, "Failed to pre-parse class interface");
				}

				if (componentClass)
				{
					scriptInfo.ClassName = className;
					ParsePublicProperties(scriptInfo, engine, script, pos);
				}
			}
			else
				pos += token.second;
		}

		return scriptInfo;
	}

	void AngelScriptTask::Update(const float delta)
	{
		auto& scripts = m_AngelScriptWorld->m_ActiveScripts;

		std::set<ModulePtr> modulesToBuild;

		tbb::spin_mutex mutex;

		auto execute_scripts = [&](const tbb::blocked_range<size_t>& r)
		{
			//auto ctx = m_ScriptManager->CreateContext();
			for (size_t i = r.begin(), end = r.end(); i != end; ++i)
			{
				auto& script = scripts[i];

				if (script->m_ModuleBuilt)
				{
					if (script->m_Module->IsBuilt())
					{
						auto objectType = script->m_Module->GetASModule()->GetObjectTypeByIndex(0);
						if (objectType->GetBaseType() == nullptr || std::string(objectType->GetBaseType()->GetName()) != "ScriptComponent")
						{
							SendToConsole("First class defined in " + script->GetScriptPath() + " isn't derived from ScriptComponent");
							script->m_ModuleBuilt = false;
							continue;
						}
						//script->m_ScriptObject = script->m_Module->CreateObject(objectType->GetTypeId());
						auto f = ScriptUtils::Calling::Caller::FactoryCaller(objectType, "");
						if (f)
						{
							f.SetThrowOnException(true);

							script->addRef();
							try
							{
								auto obj = *static_cast<asIScriptObject**>( f() );
								if (obj)
								{
									script->m_ScriptObject = ScriptObject(obj);
									auto setAppObj = ScriptUtils::Calling::Caller(obj, "void _setAppObj(ASScript @)");
									if (setAppObj)
									{
										setAppObj.SetThrowOnException(true);
										setAppObj(script.get());
									}

									auto parentEntity = script->GetParent();

									std::map<std::string, std::shared_ptr<IComponent>> convenientComponents;
									auto& interfaces = parentEntity->GetInterfaces();
									for (auto it = interfaces.cbegin(), end = interfaces.cend(); it != end; ++it)
									{
										const auto& interfaceName = it->first;
										for (auto cit = it->second.begin(), cend = it->second.end(); cit != cend; ++cit)
										{
											const auto convenientIdentifier = fe_newlower(cit->first);

											convenientComponents[convenientIdentifier] = cit->second;
										}
									}
									// Init the members with available sibling components
									auto objType = obj->GetObjectType()->GetBaseType();
									for (int i = 0, count = objType->GetPropertyCount(); i < count; ++i)
									{
										const char* name = 0; bool isPrivate = false; int offset = 0;// bool isReference = false;
										objType->GetProperty(i, &name, 0, &isPrivate, &offset/*, &isReference*/);
										if (!isPrivate)
										{
											auto _where = convenientComponents.find(name);
											if (_where != convenientComponents.end())
											{
												//FSN_ASSERT(isReference);
												auto propAddress = reinterpret_cast<void*>(reinterpret_cast<asBYTE*>(obj) + offset);
												IComponent** component = static_cast<IComponent**>(propAddress);
												_where->second->addRef();
												*component = (_where->second.get());
											}
											else
											{
												std::string propDecl = objType->GetPropertyDeclaration(i);
												SendToConsole(script->GetScriptPath() + " expected a sibling to fit '" + propDecl + "' but " + parentEntity->GetName() + " doesn't have any such component");
											}
										}
									}
								}
							}
							catch (ScriptUtils::Exception& e)
							{
								script->release();
								SendToConsole(e.m_Message);
							}
						}
					}

					script->m_ModuleBuilt = false;
				}

				if (script->m_ReloadScript)
				{
					script->m_ScriptObject.Release();

					auto moduleName = script->GetScriptPath();

					tbb::spin_mutex::scoped_lock lock(mutex);
					auto module = m_ScriptManager->GetModule(moduleName.c_str());
					script->m_Module = module;
					if (!module->IsBuilt() && modulesToBuild.find(module) == modulesToBuild.end())
					{
						// TODO: preprocess to get the interfaces used
						std::string scriptText;
						OpenString_PhysFS(scriptText, script->m_Path);

						try
						{
							auto engine = m_ScriptManager->GetEnginePtr();
							auto scriptInfo = ParseComponentScript(engine, scriptText);

							// Generate script component interface
							{
								std::string scriptComponentInterface;
								scriptComponentInterface += "class " + scriptInfo.ClassName + "\n{\n";
								//for (auto it = scriptInfo.Properties.cbegin(), end = scriptInfo.Properties.cend(); it != end; ++it)
								for (size_t i = 0; i < scriptInfo.Properties.size(); ++i)
								{
									std::string type, identifier;
									std::stringstream str;
									str << i;
									std::string propIndex = str.str();
									scriptComponentInterface += type + " get_" + identifier + "() const { return app_obj.getProperty(" + propIndex + "); }";
								}
							}

							m_AngelScriptWorld->m_ScriptInfo[scriptInfo.ClassName] = std::move(scriptInfo);

							if (module->AddCode(script->m_Path, scriptText) >= 0)
							{
								std::set<std::string> usedIdentifiers;
								std::vector<std::string> scriptComponentInterfaces;
								std::string convenientComponentProperties;
								for (auto it = scriptInfo.UsedComponents.cbegin(), end = scriptInfo.UsedComponents.cend(); it != end; ++it)
								{
									const std::string& interfaceName = it->first;
									std::string convenientIdentifier;
									if (it->second.empty())
										convenientIdentifier = fe_newlower(it->first);
									else
										convenientIdentifier = it->second;
									if (usedIdentifiers.insert(convenientIdentifier).second) // Ignore duplicate #using directives
									{
										convenientComponentProperties += interfaceName + " @ " + convenientIdentifier + ";\n";

										
										auto _where = m_AngelScriptWorld->m_ScriptInfo.find(interfaceName);
										if (_where != m_AngelScriptWorld->m_ScriptInfo.end())
										{
											std::string scriptPropertyInterface;
											scriptComponentInterfaces.push_back(scriptPropertyInterface);
										}
									}
								}

								const std::string baseCode =
									"class ScriptComponent\n"
									"{\n"
									"private ASScript@ app_obj;\n"
									"void _setAppObj(ASScript@ _app_obj) {\n"
									"@app_obj = _app_obj;\n"
									"}\n"
									"void yield() { app_obj.yield(); }\n"
									"void createCoroutine(coroutine_t @fn) { app_obj.createCoroutine(fn); }\n"
									"void createCoroutine(const string &in fn_name) { app_obj.createCoroutine(fn_name); }\n"
									"\n" +
									convenientComponentProperties +
									"}\n";

								module->AddCode("basecode", baseCode);

								for (auto it = scriptComponentInterfaces.begin(), end = scriptComponentInterfaces.end(); it != end; ++it)
								{
									const std::string usedCode =
										"class ScriptComponent\n"
										"{\n"
										"private ASScript@ app_obj;\n"
										"void _setAppObj(ASScript@ _app_obj) { @app_obj = _app_obj; }\n"
										"\n" +
										convenientComponentProperties +
										"}\n";
								}

								modulesToBuild.insert(module);
								//modulesToBuild.insert( std::make_pair(module, parentEntity->shared_from_this()) );
							}
						}
						catch (PreprocessorException &ex)
						{
							SendToConsole("Failed to pre-process " + script->m_Path + ": " + ex.what());
						}
					}

					script->m_ModuleBuilt = true;

					script->m_ReloadScript = false;
				}

				if (script->m_ScriptObject.IsValid())
				{
					// Execute any co-routines until they yield or finish
					for (auto it = script->m_ActiveCoroutines.begin(); it != script->m_ActiveCoroutines.end(); )
					{
						auto& ctx = *it;
						int r = ctx->Execute();
						if (r == asEXECUTION_SUSPENDED)
						{
							++it;
						}
						else
						{
							ctx->SetObject(NULL);
							it = script->m_ActiveCoroutines.erase(it);
						}
					}

					//ScriptUtils::Calling::Caller caller;
					boost::intrusive_ptr<asIScriptContext> ctx;
					auto _where = script->m_ScriptMethods.find("void update(float)");
					if (_where != script->m_ScriptMethods.end())
					{
						ctx = m_ScriptManager->CreateContext();
						int r = ctx->Prepare(_where->second);
						if (r >= 0)
						{
							ctx->SetObject(script->m_ScriptObject.GetScriptObject());
						}
						else
						{
							ctx->SetObject(NULL);
							ctx.reset();
						}
						//caller = ScriptUtils::Calling::Caller::CallerForMethodFuncId(script->m_ScriptObject.GetScriptObject(), _where->second);
						//m_ScriptManager->ConnectToCaller(caller);
					}
					else
					{
						//caller = script->m_ScriptObject.GetCaller("void update(float)");
						//script->m_ScriptMethods["void update(float)"] = caller.get_funcid();

						int funcId = script->m_ScriptObject.GetScriptObject()->GetObjectType()->GetMethodIdByDecl("void update(float)");

						ctx = m_ScriptManager->CreateContext();
						int r = ctx->Prepare(funcId);
						if (r >= 0)
						{
							ctx->SetObject(script->m_ScriptObject.GetScriptObject());
							script->m_ScriptMethods["void update(float)"] = funcId;
						}
						else
						{
							ctx->SetObject(NULL);
							ctx.reset();
						}
					}
					if (ctx)
					{
						ctx->SetArgFloat(0, delta);
						int r = ctx->Execute();
						if (r == asEXECUTION_SUSPENDED)
						{
							script->m_ActiveCoroutines.push_back(ctx);
						}
						//else if (r == asEXECUTION_EXCEPTION)
						//{
						//	int line, col; const char* sec;
						//	line = ctx->GetExceptionLineNumber(&col, &sec);

						//	std::stringstream str;
						//	str << "(" << line << ":" << col << ")";
						//	SendToConsole(std::string("Exception: ") + std::string(sec) + str.str() + ": " + std::string(ctx->GetExceptionString()));

						//	ctx->SetObject(NULL);
						//}
					}
				}
			}
		};

#ifdef FSN_PARALLEL_SCRIPT_EXECUTION
		tbb::parallel_for(tbb::blocked_range<size_t>(0, scripts.size()), execute_scripts);
#else
		execute_scripts(tbb::blocked_range<size_t>(0, scripts.size()));
#endif

		m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);

		for (auto it = modulesToBuild.begin(), end = modulesToBuild.end(); it != end; ++it)
		{
			auto& module = (*it);
			//auto& module = it->first;
			//auto& entity = it->second;

			if (module->Build() < 0)
			{
				// TODO: report error?
			}

			//int r = m_ScriptManager->GetEnginePtr()->RegisterObjectType("test", 0, asOBJ_REF); FSN_ASSERT(r >= 0);
			//m_ScriptManager->GetEnginePtr()->RegisterObjectBehaviour("test", asBEHAVE_FACTORY, "test@ f()", asFUNCTION(testfac), asCALL_CDECL); FSN_ASSERT(r >= 0);
			//m_ScriptManager->GetEnginePtr()->RegisterObjectBehaviour("test", asBEHAVE_ADDREF, "void f()", asFUNCTION(AddRef), asCALL_CDECL); FSN_ASSERT(r >= 0);
			//m_ScriptManager->GetEnginePtr()->RegisterObjectBehaviour("test", asBEHAVE_ADDREF, "void f()", asFUNCTION(Release), asCALL_CDECL); FSN_ASSERT(r >= 0);
		}
	}

}