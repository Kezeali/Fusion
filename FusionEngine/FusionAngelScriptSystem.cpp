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
#include "FusionEntityFactory.h"
#include "FusionException.h"
#include "FusionExceptionFactory.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionScriptReference.h"
#include "FusionXML.h"

#include "FusionPlayerRegistry.h"
#include "FusionInputHandler.h"
#include "FusionScriptInputEvent.h"

#include "scriptany.h"

#include <tbb/parallel_for.h>
#include <tbb/scalable_allocator.h>
#include <tbb/spin_mutex.h>

#include <new>
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

		if (tokenClass == asTC_VALUE && tokenLength > 2 && script[pos] == '"')
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

						const bool primative = t == asTC_KEYWORD;

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
							if (!primative && script[pos] == '@') // Only allow handles for non-primative types
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

						// A type (possibly of a property) has been parsed: copy the text
						std::string type(&script[start], &script[pos]);

						if (t == asTC_WHITESPACE)
						{
							pos += tokenLength;
							t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
						}

						if (t == asTC_IDENTIFIER)
						{
							// Found (probably) a property identifier: copy it
							std::string identifier = script.substr(pos, tokenLength);
							pos += tokenLength;

							t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
							if (t == asTC_WHITESPACE)
							{
								pos += tokenLength;
								t = engine->ParseToken(&script[pos], script.size() - pos, &tokenLength);
							}

							// At this point a type, whitespace, and an identifier have been found: If the next
							//  token is a statement-ending token ([;,]), a public property has been found
							if (script[pos] == ';' || script[pos] == ',') 
							{
								scriptInfo.Properties.push_back(std::make_pair(type, identifier));
								// Commented out code gets the full declaration in a single string. This was hard to work
								//  with for the purposes it is used for at the time writing, but may be useful for something
								//  else later (hence it is retained here)
								//scriptInfo.Properties.push_back(script.substr(start, pos - start));
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
			if (/*token.first == asTC_UNKNOWN && */script[pos] == '#')
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

	AngelScriptSystem::AngelScriptSystem(const std::shared_ptr<ScriptManager>& manager, EntityFactory* factory)
		: m_ScriptManager(manager),
		m_EntityFactory(factory)
	{
	}

	std::shared_ptr<ISystemWorld> AngelScriptSystem::CreateWorld()
	{
		return std::make_shared<AngelScriptWorld>(this, m_ScriptManager, m_EntityFactory);
	}

	AngelScriptWorld::AngelScriptWorld(IComponentSystem* system, const std::shared_ptr<ScriptManager>& manager, EntityFactory* factory)
		: ISystemWorld(system),
		m_ScriptManager(manager),
		m_EntityFactory(factory)
	{
		m_Engine = m_ScriptManager->GetEnginePtr();

		m_ASTask = new AngelScriptTask(this, m_ScriptManager);
	}

	AngelScriptWorld::~AngelScriptWorld()
	{
		delete m_ASTask;
	}

	struct DependencyNode
	{
		std::string Name;
		std::set<std::shared_ptr<DependencyNode>> IncludedBy;
		std::set<std::shared_ptr<DependencyNode>> UsedBy;
	};

	static std::string GenerateComponentInterface(const AngelScriptWorld::ComponentScriptInfo& scriptInfo)
	{
		std::string scriptComponentInterface;
		scriptComponentInterface =
			"class I" + scriptInfo.ClassName + "\n{\n"
			"private ASScript@ app_obj;\n"
			"I" + scriptInfo.ClassName + "() {}\n"
			"I" + scriptInfo.ClassName + "(ASScript@ obj) { _setAppObj(obj); }\n"
			"void _setAppObj(ASScript@ obj) { @app_obj = obj; }\n"
			"\n";
		for (size_t i = 0; i < scriptInfo.Properties.size(); ++i)
		{
			const std::string& type = scriptInfo.Properties[i].first, identifier = scriptInfo.Properties[i].second;
			std::stringstream str;
			str << i;
			std::string propIndex = str.str();
			const bool isHandleType = type.find('@') != std::string::npos;
			scriptComponentInterface +=
				type + " get_" + identifier + "() const { "
				+ type + " value; "
				"any propAny = app_obj.getProperty(" + propIndex + "); "
				"propAny.retrieve(" + std::string(isHandleType ? "@" : "") + "value); "
				"return value; }\n"

				"void set_" + identifier + "(" + type + " value) { "
				"app_obj.setProperty(" + propIndex + ", value);"
				"}\n";
		}
		scriptComponentInterface += "}\n";
		return scriptComponentInterface;
	}

	static std::string GenerateBaseCode(const AngelScriptWorld::ComponentScriptInfo& scriptInfo, std::map<std::string, AngelScriptWorld::ComponentScriptInfo>& scriptComponents)
	{
		std::set<std::string> usedIdentifiers;
		std::vector<AngelScriptWorld::ComponentScriptInfo> scriptComponentInterfaces;
		std::string convenientComponentProperties, convenientEntityProperties;
		for (auto it = scriptInfo.UsedComponents.cbegin(), end = scriptInfo.UsedComponents.cend(); it != end; ++it)
		{
			std::string interfaceName = it->first;
			std::string convenientIdentifier;
			if (it->second.empty())
				convenientIdentifier = fe_newlower(it->first);
			else
				convenientIdentifier = it->second;
			if (usedIdentifiers.insert(convenientIdentifier).second) // Ignore duplicate #using directives
			{
				auto _where = scriptComponents.find(interfaceName);
				if (_where != scriptComponents.end())
				{
					interfaceName = "I" + interfaceName;
					convenientComponentProperties += interfaceName + "@ " + convenientIdentifier + ";\n";

					convenientEntityProperties +=
						interfaceName + "@ get_" + convenientIdentifier + "() {\n"
						"return " + interfaceName + "(cast<ASScript>(app_obj.getComponent('IScript','" + it->second + "').get()));"
						"}\n";

					scriptComponentInterfaces.push_back(_where->second);
				}
				else // Native components can be casted directly from the IComponent returned by getComponent
				{
					convenientComponentProperties +=
						interfaceName + "@ get_" + convenientIdentifier + "() {\n"
						"return cast<" + interfaceName + ">((app_obj.getParent().getComponent('" + interfaceName + "','" + it->second + "')).get());"
						"}\n";
					convenientEntityProperties +=
						interfaceName + "@ get_" + convenientIdentifier + "() {\n"
						"return cast<" + interfaceName + ">((app_obj.getComponent('" + interfaceName + "','" + it->second + "')).get());"
						"}\n";
				}
			}
		}

		std::string baseCode =
			"class Input {\n"
			"Input(Entity &in obj) { app_obj = obj; }\n"
			"private Entity app_obj;\n"
			"bool getButton(const string &in name) { return app_obj.inputIsActive(name); }\n"
			"float getAnalog(const string &in name) { return app_obj.inputGetPosition(name); }\n"
			"}\n"

			"class EntityWrapper\n"
			"{\n"
			"EntityWrapper() {}\n"
			"EntityWrapper(Entity &in obj) {\n"
			"owner = initEntityPointer(obj);\n"
			"_setAppObj(obj);\n"
			"}\n"
			"~EntityWrapper() {\n"
			"deinitEntityPointer(owner, app_obj);\n"
			"}\n"
			"void _setAppObj(Entity &in obj) {\n"
			"app_obj = obj;\n"
			//"@input = Input(app_obj);\n"
			"}\n"
			"private Entity app_obj;\n"
			"private Entity owner;\n"
			//"Input@ input;\n"
			"Input@ get_input() { return Input(app_obj); }\n"
			"\n" +
			convenientEntityProperties +
			"}\n"
			"class ScriptComponent\n"
			"{\n"
			"private ASScript@ app_obj;\n"
			//"private EntityWrapper@ wrapped_entity;\n"
			"void _setAppObj(ASScript@ obj) {\n"
			"@app_obj = obj;\n"
			//"@wrapped_entity = @EntityWrapper(app_obj.getParent());\n"
			"}\n"
			"EntityWrapper@ get_entity() { return EntityWrapper(app_obj.getParent()); }\n"
			"void yield() { app_obj.yield(); }\n"
			"void createCoroutine(coroutine_t @fn) { app_obj.createCoroutine(fn); }\n"
			"void createCoroutine(const string &in fn_name, float delay = 0.0f) { app_obj.createCoroutine(fn_name, delay); }\n"
			"Entity instantiate(const string &in type, bool synch, Vector pos, float angle, PlayerID owner_id) { return ontology.instantiate(@app_obj, type, synch, pos, angle, owner_id); }\n"
			"\n" +
			convenientComponentProperties +
			"}\n";

		for (auto it = scriptComponentInterfaces.begin(), end = scriptComponentInterfaces.end(); it != end; ++it)
		{
			baseCode += GenerateComponentInterface(*it);
		}

		return baseCode;
	}

	void AngelScriptWorld::BuildScripts(bool rebuild_all)
	{
		std::vector<std::tuple<std::string, std::string, ModulePtr>> modulesToBuild;

		std::map<std::string, std::pair<std::string, AngelScriptWorld::ComponentScriptInfo>> scriptsToBuild;

		std::map<std::string, std::shared_ptr<DependencyNode>> dependencyData;
		auto getDependencyNode = [&](const std::string &name)->std::shared_ptr<DependencyNode>
		{
			std::shared_ptr<DependencyNode> dependencyNode;
			auto _where = dependencyData.find(name);
			if (_where == dependencyData.end())
			{
				dependencyNode = std::make_shared<DependencyNode>();
				dependencyNode->Name = name;
				dependencyData[name] = dependencyNode;
			}
			else
				dependencyNode = _where->second;
			return dependencyNode;
		};

		const std::string basePath = "Scripts";
		char **files = PHYSFS_enumerateFiles(basePath.c_str());
		for (auto it = files; *it != 0; ++it)
		{
			//PHYSFS_isDirectory(*it);
			const std::string fileName = basePath + "/" + *it;
			if (fileName.rfind(".as") != std::string::npos)
			{
				std::string script = OpenString_PhysFS(fileName);
				try
				{
					auto scriptInfo = ParseComponentScript(m_Engine, script);
					scriptInfo.Module = fileName;

					scriptsToBuild[fileName] = std::make_pair(script, scriptInfo);
					m_ScriptInfo[scriptInfo.ClassName] = scriptInfo;
					m_ScriptInfo["I" + scriptInfo.ClassName] = scriptInfo;

					// Add this data to the dependency tree
					auto thisNode = getDependencyNode(scriptInfo.ClassName);
					for (auto pit = scriptInfo.UsedComponents.begin(), pend = scriptInfo.UsedComponents.end(); pit != pend; ++pit)
					{
						auto usedNode = getDependencyNode(pit->first);
						usedNode->UsedBy.insert(thisNode);
					}
					for (auto pit = scriptInfo.IncludedScripts.begin(), pend = scriptInfo.IncludedScripts.end(); pit != pend; ++pit)
					{
						auto includedNode = getDependencyNode(*pit);
						includedNode->IncludedBy.insert(thisNode);
					}
				}
				catch (PreprocessorException &ex)
				{
					SendToConsole("Failed to pre-process " + fileName + ": " + ex.what());
				}
			}
		}

		for (auto it = scriptsToBuild.begin(), end = scriptsToBuild.end(); it != end; ++it)
		{
			auto& fileName = it->first;
			auto& script = it->second.first;
			auto& scriptInfo = it->second.second;

			auto module = m_ScriptManager->GetModule(fileName.c_str(), asGM_ALWAYS_CREATE);

			module->AddCode(fileName, script);

			module->AddCode("basecode", GenerateBaseCode(scriptInfo, m_ScriptInfo));

			std::string moduleFileName = fileName.substr(fileName.rfind('/'));
			moduleFileName.erase(moduleFileName.size() - 3);
			moduleFileName = "ScriptCache" + moduleFileName + ".bytecode";
			modulesToBuild.push_back(std::make_tuple(moduleFileName, scriptInfo.ClassName, module));
		}

		for (auto it = modulesToBuild.begin(), end = modulesToBuild.end(); it != end; ++it)
		{
			const std::string& bytecodeFilename = std::get<0>(*it);
			const std::string& typeName = std::get<1>(*it);
			auto& module = std::get<2>(*it);
			if (module->Build() >= 0)
			{
				std::unique_ptr<CLBinaryStream> outFile(new CLBinaryStream(bytecodeFilename, CLBinaryStream::open_write));
				module->GetASModule()->SaveByteCode(outFile.get());

				m_EntityFactory->AddInstancer(typeName, this->shared_from_this());
			}
		}
	}

	std::vector<std::string> AngelScriptWorld::GetTypes() const
	{
		static const std::string types[] = { "ASScript" };
		return std::vector<std::string>(types, types + 1);
	}

	std::shared_ptr<IComponent> AngelScriptWorld::InstantiateComponent(const std::string& type)
	{
		return InstantiateComponent(type, Vector2::zero(), 0.f, nullptr, nullptr);
	}

	std::shared_ptr<IComponent> AngelScriptWorld::InstantiateComponent(const std::string& type, const Vector2&, float, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data)
	{
		auto _where = m_ScriptInfo.find(type);
		if (_where != m_ScriptInfo.end())
		{
			auto com = std::make_shared<ASScript>();
			auto& moduleName = _where->second.Module;
			com->SetScriptPath(moduleName);
			//com->m_Module = m_ScriptManager->GetModule(moduleName.c_str());
			return com;
		}
		else if (type == "ASScript")
		{
			auto com = std::make_shared<ASScript>();
			return com;
		}
		return std::shared_ptr<IComponent>();
	}

	void AngelScriptWorld::Prepare(const std::shared_ptr<IComponent>& component)
	{
		auto scriptComponent = std::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			using namespace std::placeholders;
			scriptComponent->m_ModuleLoadedConnection = ResourceManager::getSingleton().GetResource("MODULE", scriptComponent->GetScriptPath(), std::bind(&ASScript::OnModuleLoaded, scriptComponent.get(), _1));

			FSN_ASSERT(!m_Updating);
			if (!scriptComponent->m_ScriptObject)
				m_NewlyActiveScripts.push_back(scriptComponent);
			else
				scriptComponent->MarkReady();
		}
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
			{
				auto _where = std::find(m_ActiveScripts.begin(), m_ActiveScripts.end(), scriptComponent);
				if (_where != m_ActiveScripts.end())
				{
					_where->swap(m_ActiveScripts.back());
					m_ActiveScripts.pop_back();
				}
			}

			{
				auto _where = std::find(m_NewlyActiveScripts.begin(), m_NewlyActiveScripts.end(), scriptComponent);
				if (_where != m_NewlyActiveScripts.end())
				{
					_where->swap(m_NewlyActiveScripts.back());
					m_NewlyActiveScripts.pop_back();
				}
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
		Module(other.Module),
		Properties(other.Properties),
		UsedComponents(other.UsedComponents)
	{
	}

	AngelScriptWorld::ComponentScriptInfo::ComponentScriptInfo(ComponentScriptInfo &&other)
		: ClassName(std::move(other.ClassName)),
		Module(std::move(other.Module)),
		Properties(std::move(other.Properties)),
		UsedComponents(std::move(other.UsedComponents))
	{}

	AngelScriptWorld::ComponentScriptInfo& AngelScriptWorld::ComponentScriptInfo::operator= (const ComponentScriptInfo &other)
	{
		ClassName = other.ClassName;
		Module = other.Module;
		Properties = other.Properties;
		UsedComponents = other.UsedComponents;
		return *this;
	}

	AngelScriptWorld::ComponentScriptInfo& AngelScriptWorld::ComponentScriptInfo::operator= (ComponentScriptInfo &&other)
	{
		ClassName = std::move(other.ClassName);
		Module = std::move(other.Module);
		Properties = std::move(other.Properties);
		UsedComponents = std::move(other.UsedComponents);
		return *this;
	}

	AngelScriptTask::AngelScriptTask(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager)
		: ISystemTask(sysworld),
		m_AngelScriptWorld(sysworld),
		m_ScriptManager(script_manager)
	{
		InputManager::getSingleton().SignalInputChanged.connect([this](const InputEvent& ev)
		{
			m_PlayerInputEvents[ev.Player].push_back(ev);
		});
	}

	AngelScriptTask::~AngelScriptTask()
	{
	}

	void AngelScriptTask::Update(const float delta)
	{
		{
		auto& scripts_to_instantiate = m_AngelScriptWorld->m_NewlyActiveScripts;
		std::vector<std::shared_ptr<ASScript>> notInstantiated;

		//tbb::spin_mutex mutex;

		auto instantiate_objects = [&](const tbb::blocked_range<size_t>& range)
		{
			for (size_t ri = range.begin(), rend = range.end(); ri != rend; ++ri)
			{
				auto& script = scripts_to_instantiate[ri];

				//if (script->m_ReloadScript)
				//{
				//	script->m_ScriptObject.Release();

				//	auto moduleName = script->GetScriptPath();

				//	tbb::spin_mutex::scoped_lock lock(mutex);
				//	script->m_Module = m_ScriptManager->GetModule(moduleName.c_str());

				//	script->m_ModuleBuilt = true;

				//	script->m_ReloadScript = false;
				//}

				//if (script->m_ModuleReloaded)
				{
					if (script->m_Module.IsLoaded())
					{
						auto objectType = script->m_Module->GetObjectTypeByIndex(0);
						if (objectType->GetBaseType() == nullptr || std::string(objectType->GetBaseType()->GetName()) != "ScriptComponent")
						{
							SendToConsole("First class defined in " + script->GetScriptPath() + " isn't derived from ScriptComponent");
							script->m_ModuleReloaded = false;
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
									auto scriptInfoForThis = m_AngelScriptWorld->m_ScriptInfo.find(objectType->GetName());
									script->SetScriptObject(obj, scriptInfoForThis->second.Properties);
									{
									auto setAppObj = ScriptUtils::Calling::Caller(obj, "void _setAppObj(ASScript @)");
									if (setAppObj)
									{
										setAppObj.SetThrowOnException(true);
										setAppObj(script.get());
									}
									}
#if 0
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
									for (int p = 0, count = objType->GetPropertyCount(); p < count; ++p)
									{
										const char* name = 0; int typeId = -1; bool isPrivate = false; int offset = 0;// bool isReference = false;
										objType->GetProperty(p, &name, &typeId, &isPrivate, &offset/*, &isReference*/);
										if (!isPrivate)
										{	
											auto _where = convenientComponents.find(name);
											if (_where != convenientComponents.end())
											{
												//FSN_ASSERT(isReference);

												auto propertyObjectType = m_AngelScriptWorld->GetScriptEngine()->GetObjectTypeById(typeId);
												FSN_ASSERT(propertyObjectType != nullptr);
												auto scriptInfoEntry = m_AngelScriptWorld->m_ScriptInfo.find(propertyObjectType->GetName());
												if (scriptInfoEntry != m_AngelScriptWorld->m_ScriptInfo.end())
												{
													auto componentAsScript = dynamic_cast<ASScript*>( _where->second.get() );

													// Create a script-interface object for this property
													auto fac = ScriptUtils::Calling::Caller::FactoryCaller(propertyObjectType, "");
													if (!fac)
													{
														SendToConsole("Failed to construct a script-interface object for " + _where->first + ": basecode is broken");
														continue;
													}
													fac.SetThrowOnException(true);
													try
													{
														auto scriptInterfaceObj = *static_cast<asIScriptObject**>( fac() );

														auto setAppObj = ScriptUtils::Calling::Caller(scriptInterfaceObj, "void _setAppObj(ASScript @)");
														if (setAppObj)
														{
															setAppObj.SetThrowOnException(true);
															componentAsScript->addRef();
															try
															{
																setAppObj(componentAsScript);
															}
															catch (ScriptUtils::Exception&)
															{
																componentAsScript->release();
																continue;
															}

															// Set the property to point to the interface
															auto propAddress = reinterpret_cast<void*>(reinterpret_cast<asBYTE*>(obj) + offset);
															asIScriptObject** propObjPtr = static_cast<asIScriptObject**>(propAddress);
															scriptInterfaceObj->AddRef();
															*propObjPtr = scriptInterfaceObj;
														}
													}
													catch (ScriptUtils::Exception& ex)
													{
														SendToConsole("Failed to construct a script-interface object for " + _where->first + ": " + ex.m_Message);
														continue;
													}
												}
												else // Not a script component
												{
													//auto propAddress = reinterpret_cast<void*>(reinterpret_cast<asBYTE*>(obj) + offset);
													//void** component = static_cast<void**>(propAddress);
													//_where->second->addRef();
													//*component = (_where->second.get());
												}
											}
											else
											{
												std::string propDecl = objType->GetPropertyDeclaration(p);
												SendToConsole(script->GetScriptPath() + " expected a sibling to fit '" + propDecl + "' but " + parentEntity->GetName() + " doesn't have any such component");
											}
										}
									}
#endif
								}
							}
							catch (ScriptUtils::Exception& e)
							{
								script->release();
								SendToConsole(e.m_Message);
							}
						}

						script->MarkReady();
					}
					else
						notInstantiated.push_back(script);

					script->m_ModuleReloaded = false;
				}
			}
		};

		m_AngelScriptWorld->m_Updating = true;
//#ifdef FSN_PARALLEL_SCRIPT_EXECUTION
//		tbb::parallel_for(tbb::blocked_range<size_t>(0, scripts_to_instantiate.size()), instantiate_objects);
//#else
		instantiate_objects(tbb::blocked_range<size_t>(0, scripts_to_instantiate.size()));
//#endif
		m_AngelScriptWorld->m_Updating = false;

		scripts_to_instantiate.clear();
		scripts_to_instantiate.swap(notInstantiated);
		} // scope for instantiate_objects lambda

		{
		auto& scripts = m_AngelScriptWorld->m_ActiveScripts;

		auto execute_scripts = [&](const tbb::blocked_range<size_t>& range)
		{
			//auto ctx = m_ScriptManager->CreateContext();
			for (size_t ri = range.begin(), rend = range.end(); ri != rend; ++ri)
			{
				auto& script = scripts[ri];

				if (script->m_ScriptObject)
				{
					// Execute any co-routines until they yield or finish
					for (auto it = script->m_ActiveCoroutines.begin(); it != script->m_ActiveCoroutines.end(); )
					{
						auto& ctx = it->first;
						if (it->second.IsReady())
						{
							int r = ctx->Execute();
							if (r == asEXECUTION_SUSPENDED)
							{
								// Check whether a condition was registered when this yield occured
								auto conditionEntry = script->m_ActiveCoroutinesWithConditions.find(ctx.get());
								if (conditionEntry != script->m_ActiveCoroutinesWithConditions.end())
								{
									it->second = std::move(conditionEntry->second);
									script->m_ActiveCoroutinesWithConditions.erase(conditionEntry);
								}
								++it;
							}
							else
							{
								ctx->SetObject(NULL);
								it = script->m_ActiveCoroutines.erase(it);
							}
						}
						else
							++it;
					}
					// Conditional coroutines (not generated by yields, as they were handled and earased from this container already)
					for (auto it = script->m_ActiveCoroutinesWithConditions.begin(), end = script->m_ActiveCoroutinesWithConditions.end(); it != end; ++it)
					{
						script->m_ActiveCoroutines.push_back(std::make_pair(it->second.new_ctx, it->second));
					}
					script->m_ActiveCoroutinesWithConditions.clear();

					if (script->GetParent()->GetOwnerID() != 0 && PlayerRegistry::IsLocal(script->GetParent()->GetOwnerID()))
					{
						int localPlayer = (int)PlayerRegistry::GetPlayer(script->GetParent()->GetOwnerID()).LocalIndex;
						auto playerInputEventsEntry = m_PlayerInputEvents.find(localPlayer);
						if (playerInputEventsEntry != m_PlayerInputEvents.end())
						{
							auto& eventQueue = playerInputEventsEntry->second;
							for (auto it = eventQueue.begin(), end = eventQueue.end(); it != end; ++it)
							{
								auto &queuedEvent = *it;//eventQueue.front();
								if (auto ctx = script->PrepareMethod(m_ScriptManager.get(), "void onInput(InputEvent@)"))
								{
									auto inputEvent = new ScriptInputEvent(queuedEvent);
									ctx->SetArgObject(0, inputEvent);
									int r = ctx->Execute();
									if (r == asEXECUTION_SUSPENDED)
									{
										// TODO: write yeild handling function
										//script->m_ActiveCoroutines.push_back(ctx);
									}
								}
								else
								{
									break;
								}
							}
						}
					}
					if (auto ctx = script->PrepareMethod(m_ScriptManager.get(), "void update()"))
					{
						int r = ctx->Execute();
						if (r == asEXECUTION_SUSPENDED)
						{
							ConditionalCoroutine cco;
							// Check whether a condition was registered when this yield occured
							auto conditionEntry = script->m_ActiveCoroutinesWithConditions.find(ctx.get());
							if (conditionEntry != script->m_ActiveCoroutinesWithConditions.end())
							{
								cco = std::move(conditionEntry->second);
								script->m_ActiveCoroutinesWithConditions.erase(conditionEntry);
							}
							script->m_ActiveCoroutines.push_back(std::make_pair(ctx, cco));
						}
					}
					
					script->CheckChangedPropertiesIn();
				}
			}
		};

#ifdef FSN_PARALLEL_SCRIPT_EXECUTION
		tbb::parallel_for(tbb::blocked_range<size_t>(0, scripts.size()), execute_scripts);
#else
		execute_scripts(tbb::blocked_range<size_t>(0, scripts.size()));
#endif

		m_PlayerInputEvents.clear();

		} // Scope for execute_scripts lambda

		m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_ONE_STEP);
	}

}