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

#include "PrecompiledHeaders.h"

#include "FusionAngelScriptSystem.h"

#include "FusionAngelScriptComponent.h"
#include "FusionEntity.h"
#include "FusionComponentFactory.h"
#include "FusionException.h"
#include "FusionExceptionFactory.h"
#include "FusionPaths.h"
#include "FusionPhysFS.h"
#include "FusionScriptReference.h"
#include "FusionXML.h"

// Collision handling
#include "FusionBox2DSystem.h"
#include "FusionB2ContactListenerASScript.h"

#include "FusionPlayerRegistry.h"
#include "FusionInputHandler.h"
#include "FusionScriptInputEvent.h"

#include "FusionEntityInstantiator.h"

#include "scriptany.h"

#include <boost/crc.hpp>
#include <boost/filesystem/path.hpp>

#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/scalable_allocator.h>
#include <tbb/spin_mutex.h>

#include <new>
#include <regex>

#include <BitStream.h>

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
			if (!token.empty())
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

		class TypeValidator
		{
		public:
			std::regex intTypeRegex;
			TypeValidator()
			{
				try
				{
					intTypeRegex = std::regex("u?int(?:8|16|32|64)?", std::regex_constants::ECMAScript);
					//bool|float|double|EntityWrapper
				}
				catch(std::regex_error&)
				{
					AddLogEntry(std::string("Failed to parse script properties due to regex error"));
					FSN_EXCEPT(FusionEngine::PreprocessorException, "Failed to compile script property type regex");
				}
			}

			bool operator() (const std::string& angelscript_typename)
			{
				return angelscript_typename == "bool" ||
						angelscript_typename == "float" ||
						angelscript_typename == "double" ||
						angelscript_typename == "EntityWrapper" ||
						std::regex_match(angelscript_typename, intTypeRegex);
			}
		} typeValidator;

		if (pos < script.length() && script[pos] == '{')
		{
			pos += 1;

			// Find the end of the statement block
			bool newStatement = false;
			size_t level = 1;
			while (level > 0 && pos < script.size())
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
					// token is range &script[pos], &script[pos] + tokenLength - todo: pass range (not substr) to typeValidator
					if(newStatement &&
						(t == asTC_IDENTIFIER
						|| (t == asTC_KEYWORD && typeValidator(script.substr(pos, tokenLength))))
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
						{
							pos += tokenLength;
						}
					}
					else if (t == asTC_KEYWORD && script[pos] == ';')
					{
						newStatement = true;
						++pos;
						continue;
					}
					else if (t == asTC_COMMENT)
					{
						newStatement = true;
						pos += tokenLength;
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
		{
			pos += 1;
		}
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
			if (script[pos] == '#')
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
					else if (preprocessorDirective[0] == "autoyield")
					{
						if (preprocessorDirective.size() >= 2)
						{
							if (preprocessorDirective[1] == "enable")
								scriptInfo.AutoYield = true;
							else if (preprocessorDirective[1] == "disable")
								scriptInfo.AutoYield = false;
							else
							{
								FSN_EXCEPT(PreprocessorException, "Invalid #autoyield setting: '" + preprocessorDirective[1] + "'");
							}
						}
						else
						{
							size_t directiveLength = pos - start;
							if (directiveLength > 1 && (script[pos-1] == '\r' || script[pos-1] == '\n'))
								--directiveLength;
							FSN_EXCEPT(PreprocessorException, "Invalid #autoyield directive: '" + script.substr(start, directiveLength) + "'");
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

	AngelScriptSystem::AngelScriptSystem(const std::shared_ptr<ScriptManager>& manager)
		: m_ScriptManager(manager)
	{
		ResourceManager::getSingleton().AddResourceLoader("MODULE", &LoadScriptResource, &UnloadScriptResource, NULL);
	}

	static EntityPtr InstantiationSynchroniser_Instantiate(ASScript* app_obj, const std::string& transform_component, bool synch, Vector2 pos, float angle, PlayerID owner_id, const std::string& name, EntityInstantiator* obj)
	{
		auto entity = app_obj->GetParent()->shared_from_this();

		return obj->RequestInstance(entity, synch, transform_component, name, pos, angle, owner_id);
	}

	void AngelScriptSystem::RegisterScriptInterface(asIScriptEngine* engine)
	{
		ScriptInputEvent::Register(engine);
		ScriptCollisionEvent::Register(engine);

		{
			int r = engine->RegisterGlobalFunction("bool isLocal(PlayerID)",
				asFUNCTION(PlayerRegistry::IsLocal), asCALL_CDECL);
			FSN_ASSERT(r >= 0);

			r = engine->RegisterGlobalFunction("uint getNumLocalPlayers()",
				asFUNCTION(PlayerRegistry::GetLocalPlayerCount), asCALL_CDECL);
			FSN_ASSERT(r >= 0);
		}

		engine->RegisterInterface("IEntityWrapper");

		ASScript::ScriptInterface::Register(engine);

		engine->RegisterObjectMethod("EntityInstantiator", "Entity instantiate(ASScript @, const string &in, bool, Vector, float, PlayerID owner_id = 0, const string &in name = string())",
			asFUNCTION(InstantiationSynchroniser_Instantiate), asCALL_CDECL_OBJLAST);
	}

	std::shared_ptr<ISystemWorld> AngelScriptSystem::CreateWorld()
	{
		return std::make_shared<AngelScriptWorld>(this, m_ScriptManager);
	}

	AngelScriptWorld::AngelScriptWorld(IComponentSystem* system, const std::shared_ptr<ScriptManager>& manager)
		: ISystemWorld(system),
		m_ScriptManager(manager)
	{
		m_Engine = m_ScriptManager->GetEnginePtr();
		
		BuildScripts(true);

		CreateScriptMethodMap();

		m_ASTask = new AngelScriptTask(this, m_ScriptManager);
		m_ASTaskB = new AngelScriptTaskB(this, m_ScriptManager);
	}

	AngelScriptWorld::~AngelScriptWorld()
	{
		delete m_ASTaskB;
		delete m_ASTask;
	}

	void AngelScriptWorld::OnWorldAdded(const std::shared_ptr<ISystemWorld>& other_world)
	{
		if (auto box2d = std::dynamic_pointer_cast<Box2DWorld>(other_world))
		{
			m_Box2dWorld = box2d;
		}
	}

	void AngelScriptWorld::OnWorldRemoved(const std::shared_ptr<ISystemWorld>& other_world)
	{
		if (auto box2d = std::dynamic_pointer_cast<Box2DWorld>(other_world))
		{
			m_Box2dWorld = box2d;
		}
	}

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
			if (type != "EntityWrapper@")
			{
				scriptComponentInterface +=
					"Property<" + type + ">@ get_" + identifier + "() const { "
					"Property<" + type + ">@ handle;"
					"app_obj.getProperty(" + propIndex + ").convert_into(@handle);"
					"return handle;"
					"}\n";
			}
			else
			{
				scriptComponentInterface +=
					//"Property<" + type + "> get_" + identifier + "() const { "
					type + " get_" + identifier + "() const { "
					"EntityW value; "
					"any propAny = app_obj.getPropertyRaw(" + propIndex + "); "
					"propAny.retrieve(value); "
					"return EntityWrapper(value.lock());"
					"}\n"

					"void set_" + identifier + "(EntityWrapper@ wrapper) { "
					"app_obj.setPropertyRaw(" + propIndex + ", wrapper.getRaw()); "
					"}\n";
			}
		}
		scriptComponentInterface += "}\n";
		return scriptComponentInterface;
	}

	std::string GenerateBaseCode(const AngelScriptWorld::ComponentScriptInfo& scriptInfo, const AngelScriptWorld::ScriptInfoClassMap_t& scriptComponents)
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

					convenientComponentProperties +=
						interfaceName + "@ get_" + convenientIdentifier + "() {\n"
						"return " + interfaceName + "(cast<ASScript>(getRaw().getParent().getComponent('IScript','" + it->second + "').get()));"
						"}\n";

					convenientEntityProperties +=
						interfaceName + "@ get_" + convenientIdentifier + "() {\n"
						"return " + interfaceName + "(cast<ASScript>(getLocked().getComponent('IScript','" + it->second + "').get()));"
						"}\n";

					scriptComponentInterfaces.push_back(_where->second);
				}
				else // Native components can be casted directly from the EntityComponent returned by getComponent
				{
					convenientComponentProperties +=
						interfaceName + "@ get_" + convenientIdentifier + "() {\n"
						"return cast<" + interfaceName + ">((getRaw().getParent().getComponent('" + interfaceName + "','" + it->second + "')).get());"
						"}\n";
					convenientEntityProperties +=
						interfaceName + "@ get_" + convenientIdentifier + "() {\n"
						"return cast<" + interfaceName + ">((getLocked().getComponent('" + interfaceName + "','" + it->second + "')).get());"
						"}\n";
				}
			}
		}

		std::string baseCode =
			"class Input {\n"
			"Input(const Entity &in obj) { app_obj = obj; }\n"
			"private Entity app_obj;\n"
			"bool getButton(const string &in name) { return app_obj.inputIsActive(name); }\n"
			"float getAnalog(const string &in name) { return app_obj.inputGetPosition(name); }\n"
			"}\n"

			// IEntityWrapper is an application interface which allows this type to be passed to app methods
			"class EntityWrapper : IEntityWrapper\n"
			"{\n"
			"EntityWrapper() {}\n"
			"EntityWrapper(const Entity &in obj) {\n"
			"pointer_id = initEntityPointer(owner, obj);\n"
			"_setAppObj(obj);\n"
			"}\n"
			"EntityWrapper(const EntityW &in obj, const EntityW &in owner_, uint32 id) {\n"
			"app_obj = obj;\n"
			"owner = owner_;\n"
			"pointer_id = id;\n"
			"}\n"
			"~EntityWrapper() {\n"
			"deinitEntityPointer(owner, pointer_id, app_obj.lock());\n"
			"}\n"
			"void _setAppObj(const Entity &in obj) {\n"
			"app_obj = obj;\n"
			//"@input = Input(app_obj);\n"
			"}\n"
			"private EntityW app_obj;\n"
			"private EntityW owner;\n"
			"private uint32 pointer_id;\n"
			"EntityW getRaw() const { return app_obj; }\n"
			"Entity getLocked() const { return app_obj.lock(); }\n"
			//"Input@ input;\n"
			"Input@ get_input() { return Input(app_obj.lock()); }\n"
			"void addComponent(const string &in type, const string &in id) { instantiator.addComponent(getLocked(), type, id); }"
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
			"ASScript@ getRaw() const { return app_obj; }\n"
			"EntityWrapper@ get_entity() { return EntityWrapper(app_obj.getParent()); }\n"
			"void yield() { app_obj.yield(); }\n"
			"void createCoroutine(coroutine_t @fn) { app_obj.createCoroutine(fn); }\n"
			"void createCoroutine(const string &in fn_name, float delay = 0.0f) { app_obj.createCoroutine(fn_name, delay); }\n"
			"EntityWrapper@ instantiate(const string &in type, bool synch, Vector pos, float angle, PlayerID owner_id)"
			" { return EntityWrapper(instantiator.instantiate(@app_obj, type, synch, pos, angle, owner_id)); }\n"
			"void bindMethod(const string &in fn_name, IProperty@ prop) { app_obj.bindMethod(fn_name, prop); }\n"
			"\n" +
			convenientComponentProperties +
			"}\n"

			"class CollisionEvent\n"
			"{\n"
			"CollisionEvent() {}\n"
			"CollisionEvent(ScrCollisionEvent@ obj, EntityWrapper@ other) {\n"
			"@internal = @obj;\n"
			"@other_entity = @other;\n"
			//"owner = owner_;\n"
			//"pointer_id = pointer_id_;\n"
			"}\n"
			"private ScrCollisionEvent@ internal;\n"
			//"private EntityW owner;\n"
			//"private uint32 pointer_id;\n"
			"private EntityWrapper@ other_entity;\n"
			"ScrCollisionEvent@ getRaw() const { return internal; }\n"
			"IRigidBody@ get_body() const { return internal.get_body(); }\n"
			"IFixture@ get_fixture() const { return internal.get_fixture(); }\n"
			"EntityWrapper@ get_entity() const { return other_entity; }\n"
			"}\n"

			"class EntityRapper\n"
			"{\n"
			"EntityRapper() {}\n"
			"EntityRapper(const Entity &in obj) {\n"
			"_setAppObj(obj);\n"
			"}\n"
			"void _setAppObj(const Entity &in obj) {\n"
			"app_obj = obj;\n"
			//"@input = Input(app_obj);\n"
			"}\n"
			"private EntityW app_obj;\n"
			"EntityW getRaw() const { return app_obj; }\n"
			"Entity getLocked() const { return app_obj.lock(); }\n"
			//"Input@ input;\n"
			"Input@ get_input() { return Input(app_obj.lock()); }\n"
			"void addComponent(const string &in type, const string &in id) { instantiator.addComponent(getLocked(), type, id); }"
			"\n" +
			convenientEntityProperties +
			"}\n"
			;

		for (auto it = scriptComponentInterfaces.begin(), end = scriptComponentInterfaces.end(); it != end; ++it)
		{
			baseCode += GenerateComponentInterface(*it);
		}

		return baseCode;
	}

	std::string AngelScriptWorld::GenerateBaseCodeForScript(std::string& script)
	{
		auto scriptInfo = ParseComponentScript(m_Engine, script);
		return GenerateBaseCode(scriptInfo, m_ScriptInfo);
	}

	bool AngelScriptWorld::updateChecksum(const std::string& filename, const std::string& filedata)
	{
		boost::crc_32_type crc;
		crc.process_bytes(filedata.data(), filedata.size());
		auto sum = crc.checksum();

		auto insertResult = m_ScriptChecksums.insert(std::make_pair(filename, sum));
		if (!insertResult.second)
		{
			auto _where = insertResult.first;
			if (_where->second == sum) // Existing entry is equal
				return false;
			else
			{
				_where->second = sum;
				return true;
			}
		}
		else // No existing entry
		{
			return true;
		}
	}

	std::shared_ptr<AngelScriptWorld::DependencyNode> AngelScriptWorld::getDependencyNode(const std::string &name)
	{
		std::shared_ptr<DependencyNode> dependencyNode;
		auto _where = m_DependencyData.find(name);
		if (_where == m_DependencyData.end())
		{
			dependencyNode = std::make_shared<DependencyNode>();
			dependencyNode->Name = name;
			m_DependencyData[name] = dependencyNode;
		}
		else
			dependencyNode = _where->second;
		return dependencyNode;
	};

	void AngelScriptWorld::insertScriptToBuild(std::map<std::string, std::pair<std::string, AngelScriptWorld::ComponentScriptInfo>>& scriptsToBuild, const std::string& filename, std::string& script, bool check_dependencies)
	{
		auto scriptInfo = ParseComponentScript(m_Engine, script);
		scriptInfo.Module = filename;

		scriptsToBuild[filename] = std::make_pair(script, scriptInfo);
		m_ScriptInfo[scriptInfo.ClassName] = scriptInfo;
		m_ScriptInfo["I" + scriptInfo.ClassName] = scriptInfo;

		// Add this data to the dependency tree
		auto thisNode = getDependencyNode(scriptInfo.ClassName);
		thisNode->Filename = filename;
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
		if (check_dependencies)
		{
			for (auto pit = thisNode->UsedBy.begin(), pend = thisNode->UsedBy.end(); pit != pend; ++pit)
			{
				const std::string& dependantFilename = (*pit)->Filename;
				auto buildEntry = scriptsToBuild.find(dependantFilename);
				if (buildEntry == scriptsToBuild.end())
				{
					std::string dependantScript = OpenString_PhysFS(dependantFilename);
					updateChecksum(dependantFilename, dependantScript);
					insertScriptToBuild(scriptsToBuild, dependantFilename, dependantScript, true);
				}
			}
			for (auto pit = thisNode->IncludedBy.begin(), pend = thisNode->IncludedBy.end(); pit != pend; ++pit)
			{
				const std::string& dependantFilename = (*pit)->Filename;
				auto buildEntry = scriptsToBuild.find(dependantFilename);
				if (buildEntry == scriptsToBuild.end())
				{
					std::string dependantScript = OpenString_PhysFS(dependantFilename);
					updateChecksum(dependantFilename, dependantScript);
					insertScriptToBuild(scriptsToBuild, dependantFilename, dependantScript, true);
				}
			}
		}
	}

	void AngelScriptWorld::BuildScripts(bool rebuild_all)
	{
		std::vector<std::tuple<std::string, std::string, ModulePtr>> modulesToBuild;

		std::map<std::string, std::pair<std::string, AngelScriptWorld::ComponentScriptInfo>> scriptsToBuild;

		const std::string basePath = "Scripts";
		char **files = PHYSFS_enumerateFiles(basePath.c_str());
		for (auto it = files; *it != 0; ++it)
		{
			//PHYSFS_isDirectory(*it);
			const std::string fileName = basePath + "/" + *it;
			if (fileName.rfind(".as") != std::string::npos)
			{
				std::string script = OpenString_PhysFS(fileName);
				if (updateChecksum(fileName, script) || rebuild_all)
				{
					try
					{
						insertScriptToBuild(scriptsToBuild, fileName, script, !rebuild_all);
					}
					catch (PreprocessorException &ex)
					{
						SendToConsole("Failed to pre-process " + fileName + ": " + ex.what());
					}
				}
			}
		}

		class rebuiltScript_t
		{
		public:
			boost::intrusive_ptr<ASScript> component;
			std::shared_ptr<RakNet::BitStream> occasionalData;
			std::shared_ptr<RakNet::BitStream> continiousData;
			rebuiltScript_t()
				: occasionalData(new RakNet::BitStream),
				continiousData(new RakNet::BitStream)
			{}
			rebuiltScript_t(rebuiltScript_t&& other)
				: component(std::move(other.component)),
				occasionalData(std::move(other.occasionalData)),
				continiousData(std::move(other.continiousData))
			{}
			rebuiltScript_t& operator=(rebuiltScript_t&& other)
			{
				component = std::move(other.component);
				occasionalData = std::move(other.occasionalData);
				continiousData = std::move(other.continiousData);
				return *this;
			}
		};
		std::vector<rebuiltScript_t> rebuiltScripts;

		for (auto it = scriptsToBuild.begin(), end = scriptsToBuild.end(); it != end; ++it)
		{
			auto& fileName = it->first;
			auto& script = it->second.first;
			auto& scriptInfo = it->second.second;

			ResourceDataPtr resource; // The resource pointer for any active scripts will be copied here
			for (auto sit = m_ActiveScripts.begin(), send = m_ActiveScripts.end(); sit != send; ++sit)
			{
				const auto& scriptComponent = (*sit);
				if (scriptComponent->GetScriptPath() == fileName)
				{
					rebuiltScript_t rebuiltScript;
					rebuiltScript.component = scriptComponent;

					// TODO: SerialiseEditable(rebuiltScript.serialisedData);
					scriptComponent->SerialiseOccasional(*rebuiltScript.occasionalData);
					scriptComponent->SerialiseContinuous(*rebuiltScript.continiousData);

					if (rebuiltScript.occasionalData->GetNumberOfBitsUsed() == 0)
						rebuiltScript.occasionalData.reset();
					if (rebuiltScript.continiousData->GetNumberOfBitsUsed() == 0)
						rebuiltScript.continiousData.reset();

					scriptComponent->SetScriptObject(nullptr, scriptInfo.Properties);
					//scriptComponent->m_Module.Release();
					resource = scriptComponent->m_Module.GetTarget();

					rebuiltScripts.push_back(rebuiltScript);
				}
			}

			m_ScriptManager->GetEnginePtr()->GarbageCollect(asGC_FULL_CYCLE);

			auto module = m_ScriptManager->GetModule(fileName.c_str(), asGM_ALWAYS_CREATE);

			// Hackiddy hack (switch the resource data)
			if (resource)
				resource->SetDataPtr(module->GetASModule());

			module->AddCode(fileName, script);

			module->AddCode("basecode", GenerateBaseCode(scriptInfo, m_ScriptInfo));

			auto modulePath = "ScriptCache" / boost::filesystem::path(fileName).filename();
			modulePath.replace_extension(".bytecode");
			std::string moduleFileName = modulePath.generic_string();
			//auto nameBegin = fileName.rfind('/');
			//std::string moduleFileName;
			//if (nameBegin != std::string::npos)
			//	moduleFileName = fileName.substr(nameBegin);
			//else
			//	moduleFileName = "/" + fileName;
			//moduleFileName.erase(moduleFileName.size() - 3);
			//moduleFileName = "ScriptCache" + moduleFileName + ".bytecode";
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
			}
		}

		for (auto it = rebuiltScripts.begin(), end = rebuiltScripts.end(); it != end; ++it)
		{
			const auto& rebuiltScript = *it;
			//rebuiltScript.component->m_ModuleLoadedConnection =
			//	ResourceManager::getSingleton().GetResource("MODULE", rebuiltScript.component->GetScriptPath(),
			//	std::bind(&ASScript::OnModuleLoaded, rebuiltScript.component.get(), std::placeholders::_1));

			rebuiltScript.component->m_EntityWrapperTypeId =
				rebuiltScript.component->m_Module->GetTypeIdByDecl("EntityWrapper@");
			FSN_ASSERT(rebuiltScript.component->m_EntityWrapperTypeId >= 0);

			if (rebuiltScript.component->m_EntityWrapperTypeId < 0)
			{
				SendToConsole("Failed to rebuild scripts");
				break;
			}

			if (instantiateScript(rebuiltScript.component))
			{
				if (rebuiltScript.occasionalData)
					rebuiltScript.component->DeserialiseOccasional(*rebuiltScript.occasionalData);
				if (rebuiltScript.continiousData)
					rebuiltScript.component->DeserialiseContinuous(*rebuiltScript.continiousData);
			}
		}

		ISystemWorld::PostSystemMessage(MessageType::NewTypes);
	}

	std::vector<std::string> AngelScriptWorld::GetTypes() const
	{
		std::vector<std::string> types;
		types.push_back("ASScript");
		for (auto it = m_ScriptInfo.begin(), end = m_ScriptInfo.end(); it != end; ++it)
		{
			types.push_back(it->second.ClassName);
		}
		return types;
	}

	ComponentPtr AngelScriptWorld::InstantiateComponent(const std::string& type)
	{
		return InstantiateComponent(type, Vector2::zero(), 0.f, nullptr, nullptr);
	}

	ComponentPtr AngelScriptWorld::InstantiateComponent(const std::string& type, const Vector2&, float, RakNet::BitStream* continious_data, RakNet::BitStream* occasional_data)
	{
		auto _where = m_ScriptInfo.find(type);
		if (_where != m_ScriptInfo.end())
		{
			auto com = new ASScript();//std::make_shared<ASScript>();
			auto& moduleName = _where->second.Module;
			com->SetScriptPath(moduleName);
			//com->m_Module = m_ScriptManager->GetModule(moduleName.c_str());

			return com; // ComponentPtr's constructor will init the ref count to 1
		}
		else if (type == "ASScript")
		{
			return new ASScript();
		}
		return ComponentPtr();
	}

	void AngelScriptWorld::Prepare(const ComponentPtr& component)
	{
		auto scriptComponent = boost::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			using namespace std::placeholders;
			scriptComponent->m_ModuleLoadedConnection = ResourceManager::getSingleton().GetResource("MODULE", scriptComponent->GetScriptPath(), std::bind(&ASScript::OnModuleLoaded, scriptComponent.get(), _1));

			FSN_ASSERT(!m_Updating);
			if (scriptComponent->m_ScriptObject && scriptComponent->InitialiseEntityWrappers())
				scriptComponent->MarkReady();
			else
				m_NewlyActiveScripts.push_back(scriptComponent);
		}
	}

	void AngelScriptWorld::CancelPreparation(const ComponentPtr& component)
	{
		auto scriptComponent = boost::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			// Find and remove the deactivated script
			{
				auto _where = std::find(m_NewlyActiveScripts.begin(), m_NewlyActiveScripts.end(), scriptComponent);
				if (_where != m_NewlyActiveScripts.end())
				{
					m_NewlyActiveScripts.erase(_where);
				}
			}
		}
	}

	void AngelScriptWorld::OnActivation(const ComponentPtr& component)
	{
		auto scriptComponent = boost::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			m_ActiveScripts.push_back(scriptComponent);
		}
	}

	void AngelScriptWorld::OnDeactivation(const ComponentPtr& component)
	{
		auto scriptComponent = boost::dynamic_pointer_cast<ASScript>(component);
		if (scriptComponent)
		{
			if (scriptComponent->HasContactListener())
				m_Box2dWorld->RemoveContactListener(scriptComponent->GetContactListener());
			// Find and remove the deactivated script
			{
				auto _where = std::find(m_ActiveScripts.begin(), m_ActiveScripts.end(), scriptComponent);
				if (_where != m_ActiveScripts.end())
				{
					m_ActiveScripts.erase(_where);
				}
			}

			{
				FSN_ASSERT(std::find(m_NewlyActiveScripts.begin(), m_NewlyActiveScripts.end(), scriptComponent) == m_NewlyActiveScripts.end());
				//auto _where = std::find(m_NewlyActiveScripts.begin(), m_NewlyActiveScripts.end(), scriptComponent);
				//if (_where != m_NewlyActiveScripts.end())
				//{
				//	m_NewlyActiveScripts.erase(_where);
				//}
			}
		}
	}

	ISystemTask* AngelScriptWorld::GetTask()
	{
		return m_ASTask;
	}

	std::vector<ISystemTask*> AngelScriptWorld::GetTasks()
	{
		std::vector<ISystemTask*> tasks;
		tasks.push_back(m_ASTask);
		tasks.push_back(m_ASTaskB);
		return tasks;
	}

	AngelScriptWorld::ComponentScriptInfo::ComponentScriptInfo()
		: AutoYield(true)
	{
	}

	AngelScriptWorld::ComponentScriptInfo::ComponentScriptInfo(const ComponentScriptInfo &other)
		: ClassName(other.ClassName),
		Module(other.Module),
		Properties(other.Properties),
		UsedComponents(other.UsedComponents),
		AutoYield(other.AutoYield)
	{
	}

	AngelScriptWorld::ComponentScriptInfo::ComponentScriptInfo(ComponentScriptInfo &&other)
		: ClassName(std::move(other.ClassName)),
		Module(std::move(other.Module)),
		Properties(std::move(other.Properties)),
		UsedComponents(std::move(other.UsedComponents)),
		AutoYield(other.AutoYield)
	{}

	AngelScriptWorld::ComponentScriptInfo& AngelScriptWorld::ComponentScriptInfo::operator= (const ComponentScriptInfo &other)
	{
		ClassName = other.ClassName;
		Module = other.Module;
		Properties = other.Properties;
		UsedComponents = other.UsedComponents;
		AutoYield = other.AutoYield;
		return *this;
	}

	AngelScriptWorld::ComponentScriptInfo& AngelScriptWorld::ComponentScriptInfo::operator= (ComponentScriptInfo &&other)
	{
		ClassName = std::move(other.ClassName);
		Module = std::move(other.Module);
		Properties = std::move(other.Properties);
		UsedComponents = std::move(other.UsedComponents);
		AutoYield = other.AutoYield;
		return *this;
	}

	AngelScriptTask::AngelScriptTask(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager)
		: ISystemTask(sysworld),
		m_AngelScriptWorld(sysworld),
		m_ScriptManager(script_manager)
	{
		m_PlayerInputConnection = InputManager::getSingleton().SignalInputChanged.connect([this](const InputEvent& ev)
		{
			m_PlayerInputEvents[ev.Player].push_back(ev);
		});
		m_PlayerAddedConnection = PlayerRegistry::getSingleton().SignalPlayerAdded.connect([this](const PlayerInfo& info)
		{
			tbb::spin_mutex::scoped_lock lock(m_PlayerAddedMutex);
			if (info.IsLocal())
				m_PlayerAddedEvents.push_back(std::make_pair(info.LocalIndex, info.NetID));
		});
		m_PlayerRemovedConnection = PlayerRegistry::getSingleton().SignalPlayerRemoved.connect([this](const PlayerInfo& info)
		{
			tbb::spin_mutex::scoped_lock lock(m_PlayerAddedMutex);
			if (info.IsLocal())
				m_PlayerRemovedEvents.push_back(std::make_pair(info.LocalIndex, info.NetID));
		});
	}

	AngelScriptTask::~AngelScriptTask()
	{
		m_PlayerInputConnection.disconnect();
		m_PlayerAddedConnection.disconnect();
		m_PlayerRemovedConnection.disconnect();
	}

	AngelScriptTaskB::AngelScriptTaskB(AngelScriptWorld* sysworld, std::shared_ptr<ScriptManager> script_manager)
		: ISystemTask(sysworld),
		m_AngelScriptWorld(sysworld),
		m_ScriptManager(script_manager)
	{
	}

	AngelScriptTaskB::~AngelScriptTaskB()
	{
	}

	bool AngelScriptWorld::instantiateScript(const boost::intrusive_ptr<ASScript>& script)
	{
		if (!script->m_Module.IsLoaded())
			return false;

		if (!script->m_ScriptObject)
		{
			asIObjectType* objectType = nullptr;
			for (unsigned int i = 0; i < script->m_Module->GetObjectTypeCount(); ++i)
			{
				objectType = script->m_Module->GetObjectTypeByIndex(i);
				std::string name = objectType->GetName();
				// This is a shitty way to do this
				// TODO: multimap with module name index
				auto scriptInfoForThis = m_ScriptInfo.find(name);
				if (scriptInfoForThis != m_ScriptInfo.end() && scriptInfoForThis->second.Module == script->m_Module->GetName())
					break;
			}
			if (objectType == nullptr)
			{
				SendToConsole(script->GetScriptPath() + " defines no classes");
				script->m_ModuleReloaded = false;
				return false;
			}
			if (objectType->GetBaseType() == nullptr || std::string(objectType->GetBaseType()->GetName()) != "ScriptComponent")
			{
				SendToConsole("Class defined in " + script->GetScriptPath() + " isn't derived from ScriptComponent");
				script->m_ModuleReloaded = false;
				return false;
			}
			//script->m_ScriptObject = script->m_Module->CreateObject(objectType->GetTypeId());
			auto f = ScriptUtils::Calling::Caller::FactoryCaller(objectType, "");
			if (f)
			{
				f.SetThrowOnException(true);

				try
				{
					auto obj = *static_cast<asIScriptObject**>( f() );
					if (obj)
					{
						auto scriptInfoForThis = m_ScriptInfo.find(objectType->GetName());
						if (scriptInfoForThis != m_ScriptInfo.end())
						{
							script->SetAutoYield(scriptInfoForThis->second.AutoYield);
							script->SetScriptObject(obj, scriptInfoForThis->second.Properties);
						}
						else
							return false;
					}
				}
				catch (ScriptUtils::Exception& e)
				{
					SendToConsole(e.m_Message);
				}
			}

			if (m_Box2dWorld)
			{
				const bool hasCollisionHandler = script->HasMethod(m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::SensorEnter])
					|| script->HasMethod(m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::SensorExit])
					|| script->HasMethod(m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::CollisionEnter])
					|| script->HasMethod(m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::CollisionExit]);
				if (hasCollisionHandler)
				{
					m_Box2dWorld->AddContactListener(script->GetContactListener());
				}
			}
			// Initialise the event-handler method index
			for (size_t i = 0; i < m_EventHandlerMethodDeclarations.size(); ++i)
			{
				script->AddEventHandlerMethodDecl(i, m_EventHandlerMethodDeclarations[i]);
			}
		}

		return true;
	}

	void AngelScriptTaskB::Update(const float delta)
	{
		auto& scripts_to_instantiate = m_AngelScriptWorld->m_NewlyActiveScripts;
		std::vector<boost::intrusive_ptr<ASScript>> notInstantiated;

		auto instantiate_objects = [&](const tbb::blocked_range<size_t>& range)
		{
			for (size_t ri = range.begin(), rend = range.end(); ri != rend; ++ri)
			{
				auto& script = scripts_to_instantiate[ri];

				if (script->m_Module.IsLoaded())
				{
					if (!m_AngelScriptWorld->instantiateScript(script))
						continue;
					
					if (script->InitialiseEntityWrappers())
						script->MarkReady();
				}
				
				if (!script->IsReady())
					notInstantiated.push_back(script);

				script->m_ModuleReloaded = false;
			}
		};

		m_AngelScriptWorld->m_Updating = true;

		instantiate_objects(tbb::blocked_range<size_t>(0, scripts_to_instantiate.size()));

		m_AngelScriptWorld->m_Updating = false;

		scripts_to_instantiate.clear();
		scripts_to_instantiate.swap(notInstantiated);
	}

	void AngelScriptWorld::CreateScriptMethodMap()
	{
		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::SensorEnter] = "void onSensorEnter(CollisionEvent@)";
		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::SensorExit] = "void onSensorExit(CollisionEvent@)";
		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::CollisionEnter] = "void onCollisionEnter(CollisionEvent@)";
		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::CollisionExit] = "void onCollisionExit(CollisionEvent@)";

		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::PlayerAdded] = "void onPlayerAdded(uint, PlayerID)";

		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::PlayerRemoved] = "void onPlayerRemoved(uint, PlayerID)";

		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::Input] = "void onInput(InputEvent@)";

		m_EventHandlerMethodDeclarations[EventHandlerMethodTypeIds::Update] = "void update()";

#ifdef _DEBUG
		for (auto it = m_EventHandlerMethodDeclarations.begin(); it != m_EventHandlerMethodDeclarations.end(); ++it)
		{
			// All event types must have a defined method declaraion
			FSN_ASSERT(!it->empty());
		}
#endif
	}

	boost::intrusive_ptr<asIScriptContext> AngelScriptTask::ExecuteContext(const boost::intrusive_ptr<ASScript>& script, const boost::intrusive_ptr<asIScriptContext>& ctx)
	{
		int r;
		try
		{
			r = ctx->Execute();
		}
		catch (FusionEngine::Exception& e)
		{
			SendToConsole(e.ToString());
		}

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
			script->m_ActiveCoroutines.push_back(std::make_pair(ctx, std::move(cco)));
			// Return a new context (to replace the suspended one)
			return m_ScriptManager->CreateContext();
		}
		else
		{
			ctx->Unprepare();
			// Return the existing context
			return ctx;
		}
	}

	inline bool ExecuteCollisionEvent(const boost::intrusive_ptr<asIScriptContext>& ctx, const boost::intrusive_ptr<ASScript>& script, const boost::intrusive_ptr<ScriptCollisionEvent>& ev)
	{
		auto module = script->GetScriptModule();

		if (!module)
			return false;

		int id = module->GetTypeIdByDecl("CollisionEvent");

		if (id >= 0)
		{
			auto scriptEventType = ctx->GetEngine()->GetObjectTypeById(id);
			auto ctor = ScriptUtils::Calling::Caller::FactoryCaller(scriptEventType, "ScrCollisionEvent@, EntityWrapper@");

			if (ctor)
			{
				//auto otherEntityWrapper = script->CreateEntityWrapper(ev->GetOtherEntity().lock());

				ev->addRef();
				//otherEntityWrapper->AddRef();
				auto scriptEv = *static_cast<asIScriptObject**>(ctor(ev.get(), nullptr));
				if (scriptEv)
				{
					scriptEv->AddRef();
					ctx->SetArgObject(0, scriptEv);
					//TODO: use AngelScriptTask::ExecuteContext here instead
					ctx->Execute();
				}
				else
				{
					ev->release();
					//otherEntityWrapper->Release();
				}

				return true;
			}
		}
		return false;
	}

	void AngelScriptTask::Update(const float delta)
	{
		{
		auto& scripts = m_AngelScriptWorld->m_ActiveScripts;

		std::vector<std::pair<unsigned int, PlayerID>> playerAddedEvents;
		std::vector<std::pair<unsigned int, PlayerID>> playerRemovedEvents;
		{
			tbb::spin_mutex::scoped_lock lock(m_PlayerAddedMutex);
			playerAddedEvents.swap(m_PlayerAddedEvents);
			playerRemovedEvents.swap(m_PlayerRemovedEvents);
		}

		// TODO: maybe
		//tbb::enumerable_thread_specific<boost::intrusive_ptr<asIScriptContext*>> threadContexts;

		auto execute_scripts = [&](const tbb::blocked_range<size_t>& range)
		{
			auto ctx = m_ScriptManager->CreateContext();
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
						FSN_ASSERT(it->second.new_ctx);
						script->m_ActiveCoroutines.push_back(std::make_pair(it->second.new_ctx, std::move(it->second)));
					}
					script->m_ActiveCoroutinesWithConditions.clear();

					{
						boost::intrusive_ptr<ScriptCollisionEvent> ev;
						while (script->PopCollisionEnterEvent(ev))
						{
							if (!ev->m_Sensor)
							{
								if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::CollisionEnter))
									ExecuteCollisionEvent(ctx, script, ev);
							}
							else
							{
								if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::SensorEnter))
									ExecuteCollisionEvent(ctx, script, ev);
							}
							ctx->Unprepare();
						}
						while (script->PopCollisionExitEvent(ev))
						{
							if (!ev->m_Sensor)
							{
								if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::CollisionExit))
									ExecuteCollisionEvent(ctx, script, ev);
							}
							else
							{
								if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::SensorExit))
									ExecuteCollisionEvent(ctx, script, ev);
							}
							ctx->Unprepare();
						}
					}

					// Execute property-subscription callbacks
					for (auto it = script->m_ScriptMethods.begin(); it != script->m_ScriptMethods.end(); ++it)
					{
						if (it->followedPropertyCallback)
						{
							ctx->Prepare(it->function);
							ctx->SetObject(script->GetScriptInterface()->object.get());
							it->followedPropertyCallback->ExecuteScriptMethod(ctx.get());
							ctx->Unprepare();
						}
					}

					if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::Update))
						ctx = ExecuteContext(script, ctx);

					if (script->HasMethod(AngelScriptWorld::EventHandlerMethodTypeIds::PlayerAdded))
					{
						for (auto it = playerAddedEvents.begin(); it != playerAddedEvents.end(); ++it)
						{
							if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::PlayerAdded))
							{
								int r = ctx->SetArgDWord(0, it->first); FSN_ASSERT(r >=0);
								r = ctx->SetArgByte(1, it->second); FSN_ASSERT(r >=0);

								ctx = ExecuteContext(script, ctx);
							}
							else
							{
								break;
							}
						}
					}

					if (script->HasMethod(AngelScriptWorld::EventHandlerMethodTypeIds::PlayerRemoved))
					{
						for (auto it = playerRemovedEvents.begin(); it != playerRemovedEvents.end(); ++it)
						{
							if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::PlayerRemoved))
							{
								ctx->SetArgDWord(0, it->first);
								ctx->SetArgWord(1, it->second);
								int r = ctx->Execute();

								ctx = ExecuteContext(script, ctx);
							}
							else
							{
								break;
							}
						}
					}

					if (script->HasMethod(AngelScriptWorld::EventHandlerMethodTypeIds::Input))
					{
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
									if (script->PrepareMethod(ctx, AngelScriptWorld::EventHandlerMethodTypeIds::Input))
									{
										auto inputEvent = new ScriptInputEvent(queuedEvent);
										ctx->SetArgObject(0, inputEvent);

										ctx = ExecuteContext(script, ctx);
									}
									else
									{
										break;
									}
								}
							}
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
