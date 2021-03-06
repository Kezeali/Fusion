/*
*  Copyright (c) 2011-2013 Fusion Project Team
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

#include "FusionEditor.h"

#include "FusionArchetypeFactory.h"
#include "FusionArchetypalEntityManager.h"
#include "FusionArchetypeResourceEditor.h"
#include "FusionCamera.h"
#include "FusionCellCache.h"
#include "FusionComponentFactory.h"
#include "FusionEditorCircleTool.h"
#include "FusionEditorPolygonTool.h"
#include "FusionEditorRectangleTool.h"
#include "FusionEditorWorld.h"
#include "FusionEntityInstantiator.h"
#include "FusionEntityManager.h"
#include "FusionVirtualFileSource_PhysFS.h"
#include "FusionClientOptions.h"
#include "FusionPhysFS.h"
#include "FusionRegionCellCache.h"
#include "FusionRegionMapLoader.h"
#include "FusionResource.h"
#include "FusionResourceDatabase.h"
#include "FusionStreamingManager.h"
#include "FusionWorldSaver.h"
#include "FusionWindowDropTarget.h"
#include "FusionScriptTypeRegistrationUtils.h"
#include "FusionBinaryStream.h"
#include "FusionCLRenderSystem.h"

#include "FusionComponentSystem.h"

#include "FusionAngelScriptSystem.h"
#include "FusionBox2DSystem.h"

#include "FusionBox2DComponent.h"
#include "FusionRender2DComponent.h"
#include "FusionScriptComponent.h"

#include "FusionElementInspectorGroup.h"

//#include "FusionTransformInspector.h"
//#include "FusionRigidBodyInspector.h"
//#include "FusionFixtureInspector.h"
//#include "FusionCircleShapeInspector.h"
//#include "FusionPolygonShapeInspector.h"
//#include "FusionSpriteInspector.h"
//#include "FusionASScriptInspector.h"

#include "FusionEntityInspector.h"

#include "Gwen/DebugRenderer.h"
#include <Gwen/Input/ClanLib.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <Gwen/Controls/Canvas.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/Menu.h>
#include <Gwen/Controls/Dialogs/Query.h>
#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Skins/Simple.h>
#include <Gwen/Skins/TexturedBase.h>
#include <oleidl.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

void intrusive_ptr_add_ref(asIScriptFunction *ptr)
{
	ptr->AddRef();
}

void intrusive_ptr_release(asIScriptFunction *ptr)
{
	ptr->Release();
}

namespace FusionEngine
{

	//class EditorSystem : public System::ISystem
	//{
	//	EditorSystem();

	//	System::SystemType GetType() const override
	//	{
	//		return System::Editor;
	//	}

	//	std::string GetName() const override
	//	{
	//		return "Editor";
	//	}

	//	void RegisterScriptInterface(asIScriptEngine* engine) override;

	//	std::shared_ptr<System::WorldBase> CreateWorld() override;
	//};

	std::vector<ResourceLoader> Editor::GetResourceLoaders()
	{
		return std::vector<ResourceLoader>();
	}

	std::shared_ptr<System::WorldBase> Editor::CreateWorld()
	{
		m_EditorWorld = std::make_shared<EditorWorld>(this);
		return m_EditorWorld;
	}

	inline void RemoveEqualElems(const boost::filesystem::path& base, boost::filesystem::path& subdir)
	{
		boost::filesystem::path path;
		auto it = subdir.begin(), end = subdir.end();
		for (auto bit = base.begin(), bend = base.end(); it != end && bit != end; ++it, ++bit)
		{
			if (*bit != *it)
				break;
		}
		for (; it != end; ++it)
			path /= *it;
		subdir.swap(path);
	}

	inline std::string RemoveBasePath(const boost::filesystem::path& base, const std::string& subdir)
	{
		boost::filesystem::path subdirPath(subdir);
		RemoveEqualElems(base, subdirPath);
		return subdirPath.string();
	}

	//! Implements b2QueryCallback, returning all entities within an AABB that satisfy the passed function
	class EntityQuery : public b2QueryCallback
	{
	public:
		EntityQuery(std::vector<EntityPtr>* output_array, std::function<bool (b2Fixture*, bool&)> test_fn)
			: m_Test(test_fn),
			m_Entities(output_array)
		{
		}

		explicit EntityQuery(std::vector<EntityPtr>* output_array)
			: m_Entities(output_array)
		{
		}

		bool ReportFixture(b2Fixture* fixture)
		{
			bool continue_query = true;
			if (!m_Test || m_Test(fixture, continue_query))
			{
				auto* component = static_cast<Box2DFixture*>(fixture->GetUserData());
				if (component != nullptr)
				{
					m_Entities->push_back(component->GetParent()->shared_from_this());
				}
			}
			// Continue if the test func. said to
			return continue_query;
		}

		std::function<bool (b2Fixture*, bool&)> m_Test;
		std::vector<EntityPtr>* m_Entities;

	private:
		EntityQuery(const EntityQuery& other)
			: m_Test(other.m_Test),
			m_Entities(other.m_Entities)
		{}
		EntityQuery& operator= (const EntityQuery& other)
		{
			m_Test = other.m_Test;
			m_Entities = other.m_Entities;
			return *this;
		}
	};

	inline bool MouseOverUI(Gwen::Controls::Canvas* context)
	{
		return false;
	}

	Editor::Editor(const std::vector<std::string>& args)
		: m_EditCamRange(-1.f),
		m_Active(false),
		m_RebuildScripts(false),
		m_CompileMap(false),
		m_SaveMap(false),
		m_LoadMap(false),
		m_ShiftSelect(false),
		m_AltSelect(false),
		m_ReceivedMouseDown(false),
		m_Dragging(false)
	{
		//auto& context = GUI::getSingleton().CreateContext("editor");
		//context->SetMouseShowPeriod(500);

		//m_GUIContext = context->m_Context;

		//MessageBoxMaker::AddFactory("error",
		//	[](Rocket::Core::Context* context, const MessageBoxMaker::ParamMap& params)->MessageBox*
		//{
		//	boost::intrusive_ptr<MessageBox> messageBox(new MessageBox(context, "Data/core/gui/message_box.rml"));

		//	messageBox->SetType("error_message");
		//	messageBox->SetTitle(MessageBoxMaker::GetParam(params, "title"));
		//	messageBox->SetElement("message_label", MessageBoxMaker::GetParam(params, "message"));

		//	MessageBox* messageBoxRawPtr = messageBox.get();
		//	messageBox->GetEventSignal("accept_clicked").connect([messageBoxRawPtr](Rocket::Core::Event& ev) {
		//		messageBoxRawPtr->release();
		//	});

		//	return messageBox.get();
		//});

		Console::getSingleton().BindCommand("editor_load", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			if (cmdargs.size() >= 2)
			{
				m_SaveName = cmdargs[1];
				Load();
				return "";
			}
			return "Save name required";
		});
		Console::getSingleton().SetCommandHelpText("editor_load", "Load the map with the given name", StringVector()/*"savename"*/);
		
		Console::getSingleton().BindCommand("editor_save", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			if (cmdargs.size() >= 2)
			{
				m_SaveName = cmdargs[1];
				Save();
				return "";
			}
			return "Save name required";
		});
		Console::getSingleton().SetCommandHelpText("editor_save", "Save the current map", StringVector()/*"savename"*/);

		Console::getSingleton().BindCommand("editor_savename", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			if (cmdargs.size() >= 2)
			{
				m_SaveName = cmdargs[1];
				return "Save name set to " + m_SaveName;
			}
			return m_SaveName;
		});
		Console::getSingleton().SetCommandHelpText("editor_savename", "Set the save name (without saving)", StringVector()/*"savename"*/);

		Console::getSingleton().BindCommand("le", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			std::string retval = "Selected entities: ";
			for (auto it = this->m_SelectedEntities.begin(), end = this->m_SelectedEntities.end(); it != end; ++it)
				retval += "\n" + (*it)->GetName();
			return retval;
		});
		Console::getSingleton().SetCommandHelpText("le", "List selected entities", StringVector());

		Console::getSingleton().BindCommand("editor_resourcedb_reset", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			if (this->m_ResourceDatabase)
			{
				this->m_ResourceDatabase->Clear();
				return "resource database cleared";
			}
			else return "resource database not initialised";
		});
		Console::getSingleton().SetCommandHelpText("editor_resourcedb_reset", "Clear stored resource types", StringVector());

		Console::getSingleton().BindCommand("editor_resourcedb_delete", [this](const std::vector<std::string>& cmdargs)->std::string
		{
			if (this->m_ResourceDatabase)
			{
				if (cmdargs.size() >= 2)
				{
					if (this->m_ResourceDatabase->RemoveResourceType(cmdargs[1]))
						return cmdargs[1] + "removed from database";
					else return cmdargs[1] + "not found";
				}
				return "";
			}
			else return "resource database not initialised";
		});
		Console::getSingleton().SetCommandHelpText("editor_resourcedb_delete", "Remove the stored type for the given resource", StringVector());

		{
			Console::getSingleton().BindCommand("resource_load", [](const std::vector<std::string>& cmdargs)->std::string
			{
				if (cmdargs.size() >= 3)
				{
					auto con = ResourceManager::getSingleton().GetResource(cmdargs[2], cmdargs[1], ResourceContainer::LoadedFn());
					con.disconnect();
					return "Getting resource";
				}
				else
					return "Enter a path and type";
			}, [](int arg_i, const std::string& arg)->std::vector<std::string>
			{
				std::vector<std::string> completions;
				if (arg_i == 2)
				{
					auto types = ResourceManager::getSingleton().GetResourceLoaderTypes();
					completions.assign(types.begin(), types.end());
					std::sort(completions.begin(), completions.end());
				}
				return completions;
			});
			{
				std::vector<std::string> cmdargs; cmdargs.push_back("path"); cmdargs.push_back("type");
				Console::getSingleton().SetCommandHelpText("resource_load", "Load the given resource. Can be used to populate the editor's resource-database", cmdargs);
			}
		}

		//m_SaveDialogListener = std::make_shared<DialogListener>([this](const std::map<std::string, std::string>& params)
		//{
		//	if (params.at("result") == "ok")
		//	{
		//		auto path = params.at("path");
		//		if (!path.empty())
		//		{
		//			auto basepath = boost::filesystem::path(PHYSFS_getWriteDir()) / "Editor";
		//			if (path.empty() || path == "ok")
		//				path = basepath.string();
		//			else
		//				path = FilesystemDataSource::PreprocessPath(path);
		//			auto saveName = boost::filesystem::path(RemoveBasePath(basepath, path)) / params.at("filename");
		//			saveName.replace_extension();
		//			m_SaveName = saveName.generic_string();
		//			m_SaveMap = true;
		//		}
		//	}
		//});
		//m_OpenDialogListener = std::make_shared<DialogListener>([this](const std::map<std::string, std::string>& params)
		//{
		//	if (params.at("result") == "ok")
		//	{
		//		auto path = params.at("path");
		//		if (!path.empty())
		//		{
		//			auto basepath = boost::filesystem::path(PHYSFS_getWriteDir()) / "Editor";
		//			if (path.empty() || path == "ok")
		//				path = basepath.string();
		//			else
		//				path = FilesystemDataSource::PreprocessPath(path);
		//			auto saveName = boost::filesystem::path(RemoveBasePath(basepath, path)) / params.at("filename");
		//			saveName.replace_extension();
		//			m_SaveName = saveName.generic_string();
		//			m_LoadMap = true;
		//		}
		//	}
		//});

		m_ResourceDatabase = std::make_shared<ResourceDatabase>();

		/*{
			auto tag = "inspector_" + ITransform::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::TransformInspector>())->RemoveReference();

			tag = "inspector_" + IRigidBody::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::RigidBodyInspector>())->RemoveReference();

			tag = "inspector_" + IFixture::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::FixtureInspector>())->RemoveReference();

			tag = "inspector_" + ICircleShape::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::CircleShapeInspector>())->RemoveReference();

			tag = "inspector_" + IPolygonShape::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::PolygonShapeInspector>())->RemoveReference();

			tag = "inspector_" + ISprite::GetTypeName();
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::SpriteInspector>())->RemoveReference();

			Rocket::Core::Factory::RegisterElementInstancer("inspector_asscript",
				new Rocket::Core::ElementInstancerGeneric<Inspectors::ASScriptInspector>())->RemoveReference();
		}

		{
			std::string tag = "entity_selector";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<EntitySelector>())->RemoveReference();

			tag = "inspector_entity";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::ElementEntityInspector>())->RemoveReference();

			tag = "inspector_group";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<Inspectors::ElementGroup>())->RemoveReference();

			tag = "inspector_section";
			Rocket::Core::Factory::RegisterElementInstancer(Rocket::Core::String(tag.data(), tag.data() + tag.length()),
				new Rocket::Core::ElementInstancerGeneric<InspectorSection>())->RemoveReference();
		}*/

		//m_RightClickMenu = boost::intrusive_ptr<ContextMenu>(new ContextMenu(m_GUIContext, true), false);
		//m_PropertiesMenu = boost::intrusive_ptr<MenuItem>(new MenuItem("Properties", "properties"), false);
		//m_RightClickMenu->AddChild(m_PropertiesMenu.get());
		//m_EntitySelectionMenu = boost::intrusive_ptr<MenuItem>(new MenuItem("Select", "select"), false);
		//m_RightClickMenu->AddChild(m_EntitySelectionMenu.get());

		//m_PropertiesMenu->SignalClicked.connect([this](const MenuItemEvent& e) { CreatePropertiesWindowForSelected(); });
	}

	Editor::~Editor()
	{
		Console::getSingleton().UnbindCommand("le");
		Console::getSingleton().UnbindCommand("editor_resourcedb_reset");
		Console::getSingleton().UnbindCommand("editor_resourcedb_delete");
		Console::getSingleton().UnbindCommand("resource_load");
	}

	void Editor::CleanUp()
	{
		Deactivate();

		//m_ResourceBrowser.reset();
		//m_DockedWindows.reset();
		//m_EntitySelectionMenu.reset();
		//m_PropertiesMenu.reset();
		//m_RightClickMenu.reset();
		// These need to be cleaned up before the GUI context is destroyed
		//m_SaveDialogListener.reset();
		//m_OpenDialogListener.reset();
	}

	class Win32DropTargetImpl;

	class Win32DropTarget : public WindowDropTarget
	{
	public:
		Win32DropTarget(const clan::DisplayWindow& window);

		virtual ~Win32DropTarget();

		virtual boost::signals2::signal<bool (const Vector2i& drop_location)>& GetSigDragEnter() const;

		virtual boost::signals2::signal<void (const std::string& filename, const Vector2i& drop_location)>& GetSigDrop() const;

		virtual bool TryPopDropEvent(DropEvent& out);

	private:
		clan::DisplayWindow m_DisplayWindow;
		boost::intrusive_ptr<Win32DropTargetImpl> m_Impl;
	};

	class Win32DropTargetImpl : public RefCounted, public IDropTarget, public noncopyable
	{
	public:
		Win32DropTargetImpl()
			: m_AllowDrop(false)
		{
		}

		virtual HRESULT STDMETHODCALLTYPE DragEnter(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) 
		{
			m_AllowDrop = IsSupportedData(pDataObj);

			if (m_AllowDrop)
			{
				*pdwEffect = ChooseDragEffect(grfKeyState, *pdwEffect);

				m_AllowDrop = m_SigDragEnter(Vector2i(pt.x, pt.y));
				if (!m_AllowDrop)
					*pdwEffect = DROPEFFECT_NONE;
			}
			else
				*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) 
		{
			if (m_AllowDrop)
				*pdwEffect = ChooseDragEffect(grfKeyState, *pdwEffect);
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE DragLeave(void) 
		{
			return S_OK;
		}

		virtual HRESULT STDMETHODCALLTYPE Drop(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) 
		{
			if (IsSupportedData(pDataObj))
			{
				*pdwEffect = ChooseDragEffect(grfKeyState, *pdwEffect);

				m_DroppedStuff.push(std::move(DroppedStuff(pDataObj, pt)));
			}
			else
				*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		class DroppedStuff
		{
		public:
			DroppedStuff()
				: dataObject(nullptr)
			{
			}

			DroppedStuff(IDataObject* dataObject_, POINTL pt)
				: dataObject(dataObject_),
				position(pt.x, pt.y)
			{
				dataObject->AddRef();
			}

			DroppedStuff(const DroppedStuff& other)
				: dataObject(other.dataObject),
				position(other.position)
			{
				dataObject->AddRef();
			}

			DroppedStuff(DroppedStuff&& other)
				: dataObject(other.dataObject),
				position(other.position)
			{
				other.dataObject = nullptr;
			}

			~DroppedStuff()
			{
				if (dataObject)
					dataObject->Release();
			}

			DroppedStuff& operator= (const DroppedStuff& other)
			{
				dataObject = other.dataObject;
				if (dataObject)
					dataObject->AddRef();
				position = other.position;
				return *this;
			}

			DroppedStuff& operator= (DroppedStuff&& other)
			{
				dataObject = other.dataObject;
				other.dataObject = nullptr;
				position = other.position;
				return *this;
			}

			std::vector<std::string> GetFilePaths() const
			{
				if (dataObject)
				{
					if (HasHDrop(dataObject))
						return GetFileListFromData(dataObject);
					else if (HasText(dataObject))
						return fe_splitstring(GetTextFromData(dataObject), ";");
					else
						FSN_EXCEPT(InvalidArgumentException, "Failed to get useful data from the dropped object.");
				}
				else
					FSN_EXCEPT(InvalidArgumentException, "Tried to access empty data object.");
			}

			Vector2i GetPosition() const
			{
				return position;
			}

		private:
			IDataObject* dataObject;

			Vector2i position;
		};

		tbb::concurrent_queue<DroppedStuff> m_DroppedStuff;

		static bool HasText(IDataObject *pDataObject)
		{
			FORMATETC fmtetc = { CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			return SUCCEEDED(pDataObject->QueryGetData(&fmtetc));
		}

		static bool HasHDrop(IDataObject *pDataObject)
		{
			FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			return SUCCEEDED(pDataObject->QueryGetData(&fmtetc));
		}

		static bool IsSupportedData(IDataObject *dataObject)
		{
			return HasText(dataObject) || HasHDrop(dataObject);
		}

		static DWORD ChooseDragEffect(DWORD keyState, DWORD allowedEffects)
		{
			return allowedEffects & DROPEFFECT_COPY;
		}

		static std::string GetTextFromData(IDataObject *pDataObject)
		{
			std::string dataText;

			FORMATETC fmtetc = { CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stgmed;

			if(SUCCEEDED(pDataObject->QueryGetData(&fmtetc)))
			{
				if(SUCCEEDED(pDataObject->GetData(&fmtetc, &stgmed)))
				{
					PVOID data = GlobalLock(stgmed.hGlobal);

					dataText = reinterpret_cast<char*>(data);

					GlobalUnlock(stgmed.hGlobal);

					// release the data using the COM API
					ReleaseStgMedium(&stgmed);
				}
			}

			return dataText;
		}

		static std::vector<std::string> GetFileListFromData(IDataObject *pDataObject)
		{
			std::vector<std::string> fileList;

			FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stgmed;

			if(SUCCEEDED(pDataObject->QueryGetData(&fmtetc)))
			{
				if(SUCCEEDED(pDataObject->GetData(&fmtetc, &stgmed)))
				{
					PVOID data = GlobalLock(stgmed.hGlobal);

					auto hdrop = reinterpret_cast<HDROP>(data);
					UINT numFiles = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
					for (UINT i = 0; i < numFiles; ++i)
					{
						TCHAR fileName[MAX_PATH];
						UINT cch = DragQueryFile(hdrop, i, fileName, MAX_PATH);
						if (cch > 0 && cch < MAX_PATH)
						{
							fileList.push_back(clan::StringHelp::ucs2_to_local8(std::wstring(&fileName[0], size_t(cch))));
						}
					}

					GlobalUnlock(stgmed.hGlobal);

					// release the data using the COM API
					ReleaseStgMedium(&stgmed);
				}
			}

			return fileList;
		}

		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) 
		{
			if(riid == IID_IDropTarget || riid == IID_IUnknown)
			{
				AddRef();
				*ppvObject = this;
				return S_OK;
			}
			else
			{
				*ppvObject = 0;
				return E_NOINTERFACE;
			}
		}

		virtual ULONG STDMETHODCALLTYPE AddRef(void) 
		{
			addRef();
			return this->m_RefCount;
		}

		virtual ULONG STDMETHODCALLTYPE Release(void) 
		{
			release();
			return this->m_RefCount;
		}

		boost::signals2::signal<bool (const Vector2i& drop_location)> m_SigDragEnter;
		boost::signals2::signal<void (const std::string& filename, const Vector2i& drop_location)> m_SigDrop;

	private:
		bool m_AllowDrop;
	};

	Win32DropTarget::Win32DropTarget(const clan::DisplayWindow& window)
		: m_DisplayWindow(window),
		m_Impl(new Win32DropTargetImpl(), false)
	{
		OleInitialize(NULL);

		//CoLockObjectExternal(m_Impl.get(), TRUE, FALSE);

		RegisterDragDrop(window.get_hwnd(), m_Impl.get());
	}

	Win32DropTarget::~Win32DropTarget()
	{
		if (!m_DisplayWindow.is_null())
			RevokeDragDrop(m_DisplayWindow.get_hwnd());

		//CoLockObjectExternal(m_Impl.get(), FALSE, TRUE);

		OleUninitialize();
	}

	boost::signals2::signal<bool (const Vector2i& drop_location)>& Win32DropTarget::GetSigDragEnter() const
	{
		return m_Impl->m_SigDragEnter;
	}

	boost::signals2::signal<void (const std::string& filename, const Vector2i& drop_location)>& Win32DropTarget::GetSigDrop() const
	{
		return m_Impl->m_SigDrop;
	}

	bool Win32DropTarget::TryPopDropEvent(DropEvent& out)
	{
		Win32DropTargetImpl::DroppedStuff dropObject;
		if (m_Impl->m_DroppedStuff.try_pop(dropObject))
		{
			try
			{
				out.filesList = dropObject.GetFilePaths();
				out.dropPosition = dropObject.GetPosition();
				return true;
			}
			catch (InvalidArgumentException&)
			{
			}
		}
		return false;
	}

	void Editor::Activate()
	{
		m_Active = true;

		m_OriginalSavePath = m_MapLoader->GetSavePath();
		m_MapLoader->SetSavePath("Editor/");

		BuildCreateEntityScript();

		auto camera = std::make_shared<Camera>();
		camera->SetPosition(0.f, 0.f);
		m_Viewport = std::make_shared<Viewport>(clan::Rectf(0.f, 0.f, 1.f, 1.f), camera);
		m_RenderWorld->AddViewport(m_Viewport);
		m_StreamingManager->AddCamera(camera, m_EditCamRange);

		m_EditCam = camera;

		//m_EditorOverlay->m_EditCam = m_EditCam;
		//if (m_EditCamRange <= 0.f)
		//	m_EditorOverlay->m_CamRange = m_StreamingManager->GetRange();
		//else
		//	m_EditorOverlay->m_CamRange = m_EditCamRange;

		//m_DockedWindows = std::make_shared<DockedWindowManager>(this);

		m_DropTarget = std::make_shared<Win32DropTarget>(m_DisplayWindow);
		m_DropTarget->GetSigDragEnter().connect([this](const Vector2i& drop_location)->bool
		{
			SendToConsole("Attempting to bring window to front");
			m_DisplayWindow.bring_to_front();
			return true;
		});

		//m_DropTarget->GetSigDrop().connect([this](const std::string& filename, const Vector2i& drop_location)
		//{
		//	DragDrop(filename);
		//});
	}

	void Editor::Deactivate()
	{
		m_Active = false;

		m_DropTarget.reset();

		//m_Background.reset();

		//m_EditorOverlay->m_EditCam.reset();

		m_RenderWorld->RemoveViewport(m_Viewport);

		m_EditCam.reset();
		m_Viewport.reset();

		m_MapLoader->SetSavePath(m_OriginalSavePath);
	}

	void Editor::SetOptions(const ClientOptions& options)
	{
		try
		{
			m_EditCamRange = boost::lexical_cast<float>(options.GetOption_str("editor_cam_range"));
			//m_EditorOverlay->m_CamRange = m_EditCamRange;
		}
		catch (boost::bad_lexical_cast&)
		{}
	}

	void Editor::SetDisplay(const clan::DisplayWindow& display)
	{
		m_DisplayWindow = display;
		m_KeyDownSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_down().connect(this, &Editor::OnKeyDown);
		m_KeyUpSlot = m_DisplayWindow.get_ic().get_keyboard().sig_key_up().connect(this, &Editor::OnKeyUp);
		m_MouseDownSlot = m_DisplayWindow.get_ic().get_mouse().sig_key_down().connect(this, &Editor::OnMouseDown);
		m_MouseUpSlot = m_DisplayWindow.get_ic().get_mouse().sig_key_up().connect(this, &Editor::OnMouseUp);
		m_MouseMoveSlot = m_DisplayWindow.get_ic().get_mouse().sig_pointer_move().connect(this, &Editor::OnMouseMove);
		m_WindowResizeSlot = m_DisplayWindow.sig_resize().connect(this, &Editor::OnWindowResize);
	}

	void Editor::SetCanvas(const clan::Canvas& canvas)
	{
		m_GuiRenderer = std::make_shared<Gwen::Renderer::DebugRenderer>(canvas, clan::FileSystem(new VirtualFileSource_PhysFS()));
		auto skin = std::make_shared<Gwen::Skin::TexturedBase>(m_GuiRenderer.get());
		skin->Init("Data/core/gui/DefaultSkin.png");
		m_GuiSkin = skin;

		Gwen::Controls::Canvas* pCanvas = new Gwen::Controls::Canvas( skin.get() );
		pCanvas->SetSize(canvas.get_width(), canvas.get_height());
		pCanvas->SetDrawBackground( true );
		pCanvas->SetBackgroundColor( Gwen::Color( 150, 170, 170, 255 ) );
		m_GUIContext = pCanvas;

		auto ic = m_DisplayWindow.get_ic();

		m_GuiInput = std::make_shared<Gwen::Input::ClanLib>();
		m_GuiInput->Initialize(pCanvas);

		// Mouse Events
		m_InputSlots.push_back(ic.get_mouse().sig_key_down().connect(m_GuiInput.get(), &Gwen::Input::ClanLib::onMouseDown));
		m_InputSlots.push_back(ic.get_mouse().sig_key_up().connect(m_GuiInput.get(), &Gwen::Input::ClanLib::onMouseUp));
		m_InputSlots.push_back(ic.get_mouse().sig_pointer_move().connect(m_GuiInput.get(), &Gwen::Input::ClanLib::onMouseMove));
		// KBD events
		m_InputSlots.push_back(ic.get_mouse().sig_key_down().connect(m_GuiInput.get(), &Gwen::Input::ClanLib::onKeyDown));
		m_InputSlots.push_back(ic.get_mouse().sig_key_up().connect(m_GuiInput.get(), &Gwen::Input::ClanLib::onKeyUp));
	}

	void Editor::SetMapLoader(const std::shared_ptr<RegionCellArchivist>& map_loader)
	{
		m_MapLoader = map_loader;	
	}

	void Editor::SetAngelScriptWorld(const std::shared_ptr<AngelScriptWorld>& asw)
	{
		m_AngelScriptWorld = asw;

		ScriptManager::getSingleton().RegisterGlobalObject("Editor editor", this);
		//ScriptManager::getSingleton().RegisterGlobalObject("FilesystemDataSource filesystem_datasource", Rocket::Controls::DataSource::GetDataSource("filesystem"));
		if (m_Active)
			BuildCreateEntityScript();

		ScriptManager::getSingleton().AddFile("/Data/core/gui/gui_popup.as", "gui_popup.as");
	}

	void Editor::OnWorldCreated(const std::shared_ptr<System::WorldBase>& world)
	{
		if (auto asw = std::dynamic_pointer_cast<AngelScriptWorld>(world))
		{
			SetAngelScriptWorld(asw);
		}
		else if (auto b2World = std::dynamic_pointer_cast<Box2DWorld>(world))
		{
			m_Box2DWorld = b2World;

			if (m_RenderWorld)
				m_RenderWorld->SetPhysWorld(m_Box2DWorld->Getb2World());
		}
		else if (auto renderWorld = std::dynamic_pointer_cast<CLRenderWorld>(world))
		{
			m_RenderWorld = renderWorld;

			if (m_Box2DWorld)
				m_RenderWorld->SetPhysWorld(m_Box2DWorld->Getb2World());

			if (m_Active)
			{
				auto camera = std::make_shared<Camera>();
				camera->SetPosition(0.f, 0.f);
				m_Viewport = std::make_shared<Viewport>(clan::Rectf(0.f, 0.f, 1.f, 1.f), camera);
				m_RenderWorld->AddViewport(m_Viewport);
				m_StreamingManager->AddCamera(camera);

				m_EditCam = camera;
			}
		}
	}

	class PhysVFS : public VirtualFilesystem
	{
	public:
		virtual std::shared_ptr<std::istream> OpenFileForReading(const std::string& path) const
		{
			return std::make_shared<IO::PhysFSStream>(path, IO::Read);
		}

		virtual std::shared_ptr<std::ostream> OpenFileForWriting( const std::string& path ) const
		{
			const std::string folder = boost::filesystem::path(path).remove_filename().string();
			CreateFolder(folder);
			return std::make_shared<IO::PhysFSStream>(path, IO::Write);
		}

		virtual void CreateFolder(const std::string& path) const
		{
			if (!PHYSFS_exists(path.c_str()))
			{
				if (PHYSFS_mkdir(path.c_str()) == 0)
				{
					FSN_EXCEPT(FileSystemException, "Failed to create folder " + path + ": " + PHYSFS_getLastError());
				}
			}
		}

		virtual void Delete(const std::string& path) const
		{
			if (PHYSFS_isDirectory(path.c_str()))
				PhysFSHelp::clear_folder(path);

			if (PHYSFS_delete(path.c_str()) == 0)
			{
				FSN_EXCEPT(FileSystemException, "Failed to delete item at " + path + ": " + PHYSFS_getLastError());
			}
		}
	};

	void Editor::Update(float time, float dt)
	{
		
		// Bodies have to be forced to create since the simulation isn't running
		m_Box2DWorld->InitialiseActiveComponents();

		if (m_EditCam && !MouseOverUI(m_GUIContext))
		{
			if (!m_DisplayWindow.get_ic().get_keyboard().get_keycode(clan::keycode_control))
			{
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(clan::keycode_up))
					m_CamVelocity.y = -400;
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(clan::keycode_down))
					m_CamVelocity.y = 400;
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(clan::keycode_left))
					m_CamVelocity.x = -400;
				if (m_DisplayWindow.get_ic().get_keyboard().get_keycode(clan::keycode_right))
					m_CamVelocity.x = 400;
			}
			if (!v2Equal(m_CamVelocity, Vector2(0.f, 0.f)))
			{
				auto camPos = m_EditCam->GetPosition();
				auto camTrans = m_CamVelocity * dt;
				m_EditCam->SetPosition(camPos.x + camTrans.x, camPos.y + camTrans.y);

				const auto kb = m_DisplayWindow.get_ic().get_keyboard();
				if (!(kb.get_keycode(clan::keycode_up) || kb.get_keycode(clan::keycode_down)))
				{
					if (m_CamVelocity.length() > 1.f)
						m_CamVelocity.y *= 0.75f * dt;
					else
						m_CamVelocity.y = 0.f;
				}
				if (!(kb.get_keycode(clan::keycode_left) || kb.get_keycode(clan::keycode_right)))
				{
					if (m_CamVelocity.length() > 1.f)
						m_CamVelocity.x *= 0.5f * dt;
					else
						m_CamVelocity.x = 0.f;
				}
			}
		}

		for (auto it = m_ToDelete.begin(), end = m_ToDelete.end(); it != end; ++it)
			DeleteEntity(*it);
		m_ToDelete.clear();

		if (m_RebuildScripts && m_AngelScriptWorld)
		{
			m_RebuildScripts = false;
			m_AngelScriptWorld->BuildScripts();

			BuildCreateEntityScript();
		}

		const auto timeSinceLastAction = time - m_LastActionTime;
		if (timeSinceLastAction > 0.5f)
		{
			m_LastActionTime = time;
			if (timeSinceLastAction > dt) // make sure there is a break between actions
				ExecuteAction();
		}

		WindowDropTarget::DropEvent dropEvent;
		if (m_DropTarget->TryPopDropEvent(dropEvent))
		{
			for (const auto& file : dropEvent.filesList)
				DragDrop(file);
		}

		if (m_CompileMap)
		{
			m_CompileMap = false;

			Compile(m_SaveName);
		}

		if (m_SaveMap)
		{
			m_SaveMap = false;
			Save();
		}

		if (m_LoadMap)
		{
			m_LoadMap = false;
			Load();
		}
	}

	std::vector<std::shared_ptr<RendererExtension>> Editor::MakeRendererExtensions() const
	{
		return std::vector<std::shared_ptr<RendererExtension>>();
	}

	bool Editor::IsResourceEditable(const std::string& filename) const
	{
		return m_ResourceEditors.find(m_ResourceDatabase->GetResourceType(filename)) != m_ResourceEditors.end();
	}
	
	std::shared_ptr<ResourceEditor> Editor::StartResourceEditor(const std::string& filename, const Vector2& offset)
	{
		auto type = m_ResourceDatabase->GetResourceType(filename);
		if (!type.empty())
		{
			auto editorEntry = m_ResourceEditors.find(type);
			if (editorEntry != m_ResourceEditors.end())
			{
				auto path = boost::filesystem::path(filename).parent_path();
				if (path.has_parent_path())
					PHYSFS_mkdir(path.string().c_str());

				auto editor = editorEntry->second;
				ResourceManager::getSingleton().GetResource(type, filename, [editor, offset](ResourceDataPtr res) { editor->SetResource(res, offset); });

				return editor;
			}
			else
				SendToConsole("There is no resource editor for " + type + ".");
		}
		else
			SendToConsole(filename + " is not present in the resource-db. Load it to assign a type before attempting to edit.");

		return std::shared_ptr<ResourceEditor>();
	}

	std::shared_ptr<ResourceEditor> Editor::StartResourceEditor(const std::string& filename)
	{
		// Default to cam center
		Vector2 offset(m_EditCam->GetPosition().x, m_EditCam->GetPosition().y);
		// Set offset to center of selection
		if (GetNumSelected() > 0)
		{
			auto c = GetBBOfSelected().get_center();
			offset = Vector2(ToRenderUnits(c.x), ToRenderUnits(c.y));
		}
		return StartResourceEditor(filename, offset);
	}

	std::string Editor::GetResourceType(const std::string& path)
	{
		return m_ResourceDatabase->GetResourceType(path);
	}

	void Editor::GoToEntity(const EntityPtr& entity)
	{
		const auto entityPosition = entity->GetPosition();
		m_EditCam->SetSimPosition(entityPosition);
	}

	void Editor::ShowSaveMapDialog()
	{
		//auto document = m_GUIContext->LoadDocument("Data/core/gui/file_dialog.rml");

		//Rocket::Core::String title("Save Map");
		//document->SetTitle(title);
		//if (auto okButton = document->GetElementById("button_ok"))
		//	okButton->SetInnerRML("Save");
		//if (auto fileList = document->GetElementById("file_list"))
		//	fileList->SetAttribute("source", "filesystem.#write_dir/Editor");

		//m_SaveDialogListener->Attach(document);
		//document->Show(Rocket::Core::ElementDocument::MODAL | Rocket::Core::ElementDocument::FOCUS);
		//document->RemoveReference();
	}

	void Editor::ShowLoadMapDialog()
	{
		//auto document = m_GUIContext->LoadDocument("Data/core/gui/file_dialog.rml");

		//Rocket::Core::String title("Open Map");
		//document->SetTitle(title);
		//if (auto okButton = document->GetElementById("button_ok"))
		//	okButton->SetInnerRML("Open");
		//if (auto fileList = document->GetElementById("file_list"))
		//	fileList->SetAttribute("source", "filesystem.#write_dir/Editor");

		//m_OpenDialogListener->Attach(document);
		//document->Show(Rocket::Core::ElementDocument::MODAL | Rocket::Core::ElementDocument::FOCUS);
		//document->RemoveReference();
	}

	void Editor::ShowSaveDialog(const std::string& title, const std::string& initial_path)
	{
		if (m_SaveDialogOverride)
		{
			m_SaveDialogOverride(title, initial_path, [this](const std::string& path) { Save(path); });
		}
		else
		{
			//auto document = m_GUIContext->LoadDocument("Data/core/gui/file_dialog.rml");

			//Rocket::Core::String title("Save Map");
			//document->SetTitle(title);
			//if (auto okButton = document->GetElementById("button_ok"))
			//	okButton->SetInnerRML("Save");
			//if (auto fileList = document->GetElementById("file_list"))
			//	fileList->SetAttribute("source", "filesystem.#write_dir/Editor");

			//m_SaveDialogListener->Attach(document);
			//document->Show(Rocket::Core::ElementDocument::MODAL | Rocket::Core::ElementDocument::FOCUS);
			//document->RemoveReference();
		}
	}

	void Editor::ShowLoadDialog(const std::string& title, const std::string& initial_path)
	{
		if (m_OpenDialogOverride)
		{
			m_OpenDialogOverride(title, initial_path, [this](const std::string& path) { Load(path); });
		}
		else
		{
			//auto document = m_GUIContext->LoadDocument("Data/core/gui/file_dialog.rml");

			//Rocket::Core::String title("Open Map");
			//document->SetTitle(title);
			//if (auto okButton = document->GetElementById("button_ok"))
			//	okButton->SetInnerRML("Open");
			//if (auto fileList = document->GetElementById("file_list"))
			//	fileList->SetAttribute("source", "filesystem.#write_dir/Editor");

			//m_OpenDialogListener->Attach(document);
			//document->Show(Rocket::Core::ElementDocument::MODAL | Rocket::Core::ElementDocument::FOCUS);
			//document->RemoveReference();
		}
	}

	void Editor::Save()
	{
		if (!m_SaveName.empty())
		{
			Save(m_SaveName);
		}
	}

	void Editor::Save(const std::string& name)
	{
		try
		{
			m_Saver->Save(name, false);
		}
		catch (std::exception& e)
		{
			auto messageTitle = std::string("Failed");
			auto messageText = std::string(e.what());
		}
	}

	void Editor::Load()
	{
		if (!m_SaveName.empty())
		{
			Load(m_SaveName);
		}
	}

	void Editor::Load(const std::string& name)
	{
		try
		{
			m_Saver->Load(name);
				
			m_NonStreamedEntities = m_EntityManager->GetLastLoadedNonStreamedEntities();
			m_StreamingManager->AddCamera(m_EditCam, m_EditCamRange);
		}
		catch (std::exception& e)
		{
			auto messageTitle = std::string("Failed");
			auto messageText = std::string(e.what());
		}
	}

	void Editor::Compile(const std::string& mapName)
	{
		try
			{
				if (!mapName.empty() && (PHYSFS_isDirectory(mapName.c_str()) || PHYSFS_mkdir(mapName.c_str())))
				{
					m_StreamingManager->StoreAllCells(false);

					m_MapLoader->Save(mapName);
					m_MapLoader->Stop();

					PhysVFS vfs;
					GameMap::CompileMap(vfs, mapName, m_StreamingManager->GetCellSize(), m_MapLoader.get(), m_NonStreamedEntities, m_EntityInstantiator.get());

					auto messageTitle = std::string("Success");
					auto messageText = std::string("Compiled ") + mapName;
				}
				else
				{
					SendToConsole("Failed to compile map: failed to create map folder");
					auto messageTitle = std::string("Compilation Failed");
					auto messageText = std::string("Failed to create map folder");
				}
			}
			catch (FileSystemException& e)
			{
				SendToConsole("Failed to compile map: " + e.GetDescription());
				auto messageTitle = std::string("Compilation Failed");
				auto messageText = e.GetDescription();
			}
			catch (Exception& e)
			{
				SendToConsole("Failed to compile map: " + e.GetDescription());
				auto messageTitle = std::string("Compilation Failed");
				auto messageText = e.GetDescription();
			}
			m_MapLoader->Start();
			m_StreamingManager->Update(true);
	}

	bool Editor::DragEnter(const std::string &path)
	{
		m_DragData = path;
		return true;
	}

	bool Editor::DragDrop(const std::string &path)
	{
		SendToConsole("Dropped: " + path);

		m_DragData = path;
		CreateArchetypeInstance(m_DragData, GetMousePositionInWorld(), 0.0f, m_SelectionRectangle, 100.f);
		return true;
	}

	void Editor::ToggleResourceBrowser()
	{
		if (IsResourceBrowserVisible())
			HideResourceBrowser();
		else
			ShowResourceBrowser();
	}

	bool Editor::IsResourceBrowserVisible() const
	{
		//return m_ResourceBrowser && m_ResourceBrowser->IsVisible();
		return false;
	}

	void Editor::ShowResourceBrowser()
	{
		//if (!m_ResourceBrowser)
		//{
		//	m_ResourceBrowser = m_GUIContext->LoadDocument("Data/core/gui/resource_browser.rml");
		//	m_ResourceBrowser->RemoveReference();

		//	m_DockedWindows->AddWindow(m_ResourceBrowser.get(), DockedWindowManager::Left);
		//}
		//if (m_ResourceBrowser)
		//{
		//	m_ResourceBrowser->Show();

		//	auto area = m_Viewport->GetArea();
		//	area.left = m_ResourceBrowser->GetOffsetWidth() / m_DisplayWindow.get_gc().get_width();
		//	m_Viewport->SetArea(area);
		//}
	}

	void Editor::HideResourceBrowser()
	{
		//if (m_ResourceBrowser)
		//{
		//	m_ResourceBrowser->Hide();
		//	m_Viewport->SetArea(0.f, 0.f, 1.f, 1.f);
		//}
	}

	EntityPtr createEntity(bool add_to_scene, unsigned int i, Vector2 position, EntityInstantiator* instantiator, ComponentFactory* factory, EntityManager* entityManager)
	{
		position.x = ToSimUnits(position.x); position.y = ToSimUnits(position.y);

		ComponentPtr transformCom;
		if (i == 1 || i == 2)
		{
			transformCom = factory->InstantiateComponent("StaticTransform");
		}
		else if (i == 4)
		{
			transformCom = factory->InstantiateComponent("b2Kinematic");
		}
		else
		{
			transformCom = factory->InstantiateComponent("b2RigidBody");
		}

		auto entity = std::make_shared<Entity>(entityManager, transformCom);

		if (i == 2 || i == 3)
		{
			ObjectID id = 0;
			id = instantiator->GetFreeGlobalID();
			entity->SetID(id);

			std::stringstream str;
			str << i << "_" << id;
			entity->SetName("edit" + str.str());

			if (i == 2)
				entity->SetDomain(SYSTEM_DOMAIN);
		}
		//else
		//{
		//	std::stringstream str;
		//	str << reinterpret_cast<uintptr_t>(entity.get());
		//	entity->SetName("edit" + str.str());
		//}

		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Position.Set(position);
			transform->Angle.Set(0.f);
		}

		if (add_to_scene)
			entityManager->AddEntity(entity);

		if (i == 1)
		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Depth.Set(-1);
		}

		ComponentPtr b2CircleFixture;
		if (i == 3 || i == 4)
		{
			b2CircleFixture = factory->InstantiateComponent("b2Circle");
			entity->AddComponent(b2CircleFixture);
			{
				auto fixture = entity->GetComponent<FusionEngine::IFixture>();
				fixture->Density.Set(0.8f);
				fixture->Sensor.Set(i > 80);
				auto shape = entity->GetComponent<ICircleShape>();
				shape->Radius.Set(ToSimUnits(50.f / 2.f));
			}
			entity->SynchroniseParallelEdits();
		}

		auto clSprite = factory->InstantiateComponent("CLSprite");
		entity->AddComponent(clSprite);

		ComponentPtr asScript, asScript2;
		if (i == 4)
		{
			asScript = factory->InstantiateComponent("ASScript");
			entity->AddComponent(asScript, "script_a");
		}

		if (i == 2)
		{
			asScript2 = factory->InstantiateComponent("ASScript");
			entity->AddComponent(asScript2, "spawn_script");
		}

		if (i == 4)
		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Depth.Set(1);
		}

		{
			auto sprite = entity->GetComponent<ISprite>();
			if (i == 1)
			{
				sprite->ImagePath.Set("Entities/Dirt.png");
			}
			else if (i == 3)
			{
				sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving.png");
				sprite->AnimationPath.Set("Entities/Test/test_anim.yaml");
			}
			else
				sprite->ImagePath.Set("Entities/Test/Gfx/spaceshoot_body_moving1.png");
		}

		if (i == 5)
		{
			auto script = entity->GetComponent<IScript>("script_a");
			if (script)
				script->ScriptPath.Set("Scripts/test_script.as");
		}
		if (i == 2)
		{
			auto script = entity->GetComponent<IScript>("spawn_script");
			if (script)
				script->ScriptPath.Set("Scripts/SpawnPoint.as");
		}
		entity->SynchroniseParallelEdits();

		{
			auto body = entity->GetComponent<IRigidBody>();
			if (body)
			{
				//body->ApplyTorque(10.f);
				//body->ApplyForce(Vector2(2000, 0), body->GetCenterOfMass() + Vector2(2, -1));
				//body->AngularVelocity.Set(CL_Angle(180, cl_degrees).to_radians());
				body->LinearDamping.Set(0.1f);
				body->AngularDamping.Set(0.9f);
			}
		}

		return entity;
	}

	void Editor::NudgeSelectedEntities(const Vector2& delta_pixels)
	{
		Vector2 delta = delta_pixels;
		delta *= 1.f / m_EditCam->GetZoom();
		delta.x = ToSimUnits(delta.x); delta.y = ToSimUnits(delta.y);

		ForEachSelected([delta](const EntityPtr& entity) { entity->SetPosition(entity->GetPosition() + delta); });

		//m_EditorOverlay->SetOffset(Vector2());
	}

	void Editor::QueueAction(const std::function<void (void)>& action)
	{
		m_ActionQueue.push(action);
	}

	void Editor::ExecuteAction()
	{
		if (!m_ActionQueue.empty())
		{
			try
			{
				auto action = m_ActionQueue.front();
				action();
			}
			catch (std::exception& ex)
			{
				SendToConsole(ex.what());
			}
			m_ActionQueue.pop();
		}
	}

	void Editor::OnKeyDown(const clan::InputEvent& ev)
	{
		if (!m_Active)
			return;

		if (ev.shift)
			m_ShiftSelect = true;
		if (ev.alt)
			m_AltSelect = true;

		if (MouseOverUI(m_GUIContext))
			return;

		if (m_Tool != Tool::None)
		{
			auto tool = m_ShapeTools[m_Tool];
			if (tool)
				tool->KeyChange(ev.shift, ev.ctrl, ev.alt);
		}

		// Ctrl + Keys
		if (ev.ctrl && ev.repeat_count == 0)
		{
			switch (ev.id)
			{
			case clan::keycode_s:
				if (ev.shift || m_SaveName.empty())
					ShowSaveMapDialog();
				else
					m_SaveMap = true;
				break;
			case clan::keycode_o:
				ShowLoadMapDialog();
				break;

			case clan::keycode_c:
				CopySelectedEntities();
				break;
			case clan::keycode_v:
				{
					Vector2 offset = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
					TranslateScreenToWorld(&offset.x, &offset.y);
					PasteEntities(offset);
				}
				break;

			case clan::keycode_r:
				{
					ToggleResourceBrowser();
				}
				break;

			case clan::keycode_0:
				m_EditCam->SetZoom(1.0);
				break;

			case clan::keycode_p:
				{
					CreatePropertiesWindowForSelected();
				}
				break;

			case clan::keycode_left:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(-1.f, 0.f));
				break;
			case clan::keycode_right:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(1.f, 0.f));
				break;
			case clan::keycode_up:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(0.f, -1.f));
				break;
			case clan::keycode_down:
				if (!m_Dragging)
					NudgeSelectedEntities(Vector2(0.f, 1.f));
				break;
			}
		}
		else
		{
			switch (ev.id)
			{
			//case keycode_l:
			//	{
			//		std::vector<Vector2> verts;
			//		m_PolygonTool->Start(verts, [](const std::vector<Vector2>& v) {}, EditorPolygonTool::Line);
			//		m_Tool = Tool::Polygon;
			//	}
			//	break;
			//case keycode_k:
			//	{
			//		Vector2 center;
			//		m_CircleTool->Start(center, 0.0f, [](const Vector2& c, float r) {
			//			SendToConsole("Circle: " + boost::lexical_cast<std::string>(c.x) + "," + boost::lexical_cast<std::string>(c.x) + ", " + boost::lexical_cast<std::string>(r));
			//		});
			//		m_Tool = Tool::Elipse;
			//	}
			//	break;
			//case keycode_m:
			//	{
			//		Vector2 hsize(1.f, 1.f);
			//		Vector2 center;
			//		m_RectangleTool->Start(hsize, center, 0.0f, [](const Vector2& hs, const Vector2& c, float r) {
			//			SendToConsole("Rect: " + boost::lexical_cast<std::string>(c.x) + "," + boost::lexical_cast<std::string>(c.x) +
			//				" size:" + boost::lexical_cast<std::string>(hs.x) + "," + boost::lexical_cast<std::string>(hs.x) +
			//				" r:" + boost::lexical_cast<std::string>(r));
			//		});
			//		m_Tool = Tool::Rectangle;
			//	}
			//	break;
			case clan::keycode_return:
				if (m_Tool != Tool::None)
				{
					auto tool = m_ShapeTools[m_Tool];
					if (tool)
						tool->Finish();
				}
				m_Tool = Tool::None;
				break;
			case clan::keycode_escape:
				if (m_Tool != Tool::None)
				{
					auto tool = m_ShapeTools[m_Tool];
					if (tool)
						tool->Cancel();
				}
				m_Tool = Tool::None;
				break;
			};
			//if (m_Tool == Tool::Polygon || m_Tool == Tool::Line)
			//{
			//	if (ev.id == keycode_return)
			//	{
			//		m_Tool = Tool::None;
			//		m_PolygonTool->Finish();
			//	}
			//	else if (ev.id == keycode_escape)
			//	{
			//		m_Tool = Tool::None;
			//		m_PolygonTool->Cancel();
			//	}
			//}
		}
	}

	void Editor::OnKeyUp(const clan::InputEvent& ev)
	{
		if (!m_Active)
			return;
		
		m_ShiftSelect = false;
		m_AltSelect = false;

		if (MouseOverUI(m_GUIContext))
			return;

		if (m_Tool != Tool::None)
		{
			auto tool = m_ShapeTools[m_Tool];
			if (tool)
				tool->KeyChange(ev.shift, ev.ctrl, ev.alt);
		}

		// Ctrl + Keys
		if (ev.ctrl)
		{
			switch (ev.id)
			{
				case clan::keycode_t:
				{
						EntityPtr entity;
						//if (!m_EditorOverlay->m_Selected.empty())
						//{
						//	entity = *m_EditorOverlay->m_Selected.begin();
						//}
						//else
						{
							entity = createEntity(false, 3, Vector2::zero(), m_EntityInstantiator.get(), m_ComponentFactory.get(), m_EntityManager.get());
							//entity->SetName(m_EntityManager->GenerateName(entity));
						}

						std::string archetypeName = "/Data/" + entity->GetName() + ".archetype";

						std::shared_ptr<ArchetypeFactory> archetypeFactory = std::make_shared<ArchetypeFactory>();
						archetypeFactory->DefineArchetypeFromEntity(m_ComponentFactory.get(), archetypeName, entity);

						IO::PhysFSStream archetypeFile(archetypeName, IO::OpenMode::Write);
						archetypeFactory->Save(archetypeFile);
				}
				break;
				case clan::keycode_y:
				{
						EntityPtr entity;
						//if (!m_EditorOverlay->m_Selected.empty())
						//{
						//	entity = *m_EditorOverlay->m_Selected.begin();
						//}
						//else
						{
							entity = createEntity(false, 3, Vector2::zero(), m_EntityInstantiator.get(), m_ComponentFactory.get(), m_EntityManager.get());
							//entity->SetName(m_EntityManager->GenerateName(entity));
						}

						//CreatePrefab("/Data/" + entity->GetName() + ".prefab", m_EditorOverlay->m_Selected);
				}
				break;
			}
		}
		// Keys
		else
		{
			switch (ev.id)
			{
			case clan::keycode_f3:
				m_RenderWorld->ToggleDebugDraw();
				break;
			case clan::keycode_f5:
				m_CompileMap = true;
				break;
			case clan::keycode_f7:
				m_RebuildScripts = true;
				break;
			case clan::keycode_f9:
				Console::getSingleton().Interpret("prof_savetimes on");
				break;
			case clan::keycode_f10:
				Console::getSingleton().Interpret("prof_savetimes off");
				break;
			case clan::keycode_delete:
				if (ev.shift)
					ForEachSelected([this](const EntityPtr& entity) { this->AddEntityToDelete(entity); });
				else
				{
					//ShowDeleteDialog();
				}
				break;
			case clan::keycode_prior:
				{
				float interval = m_StreamingManager->GetPollArchiveInterval();
				m_StreamingManager->SetPollArchiveInterval(interval += 0.05f);
				}
				break;
			case clan::keycode_next:
				{
				float interval = m_StreamingManager->GetPollArchiveInterval();
				m_StreamingManager->SetPollArchiveInterval(interval -= 0.05f);
				}
				break;
			}
		}
		// Numbers
		if (ev.id >= clan::keycode_0 && ev.id <= clan::keycode_9)
		{
			unsigned int num = (unsigned int)(ev.id - clan::keycode_0);

			auto vps = m_RenderWorld->GetViewports();
			if (vps.empty())
				return;
			auto vp = vps.front();

			srand(clan::System::get_time());

			bool randomAngle = ev.shift;

			// TODO: if Caller gets destroyed without being called an exception is fired: investigate
			auto caller = ScriptUtils::Calling::Caller::CallerForGlobalFuncId(ScriptManager::getSingleton().GetEnginePtr(), m_CreateEntityFn->GetId());
			if (caller)
			{
				Vector2 pos((float)ev.mouse_pos.x, (float)ev.mouse_pos.y);
				TranslateScreenToWorld(&pos.x, &pos.y);

				// Fix the selection rectangle
				if (m_SelectionRectangle.bottom < m_SelectionRectangle.top)
					std::swap(m_SelectionRectangle.top, m_SelectionRectangle.bottom);
				if (m_SelectionRectangle.right < m_SelectionRectangle.left)
					std::swap(m_SelectionRectangle.left, m_SelectionRectangle.right);

				const int spawnAreaInterval = 100;

				if (m_SelectionRectangle.contains(clan::Vec2f(pos.x, pos.y)) && m_SelectionRectangle.get_width() > spawnAreaInterval && m_SelectionRectangle.get_height() > spawnAreaInterval)
				{
					for (pos.y = m_SelectionRectangle.top; pos.y < m_SelectionRectangle.bottom; pos.y += spawnAreaInterval)
						for (pos.x = m_SelectionRectangle.left; pos.x < m_SelectionRectangle.right; pos.x += spawnAreaInterval)
						{
							Vector2 simPos(ToSimUnits(pos.x), ToSimUnits(pos.y));

							float angle = 0.f;
							if (randomAngle)
								angle = 2.f * s_pi * (rand() / (float)RAND_MAX);

							caller(num, &simPos, angle);
						}
				}
				else
				{
					pos.x = ToSimUnits(pos.x);
					pos.y = ToSimUnits(pos.y);

					float angle = 0.f;

					caller(num, &pos, angle);
				}
			}
		}
	}

	inline ShapeTool::MouseInput GetMouseInputType(const int id)
	{
		switch (id)   
		{
		case clan::mouse_left: return ShapeTool::MouseInput::LeftButton;
		case clan::mouse_wheel_up: return ShapeTool::MouseInput::ScrollUp;
		case clan::mouse_wheel_down: return ShapeTool::MouseInput::ScrollDown;
		default: return ShapeTool::MouseInput::None;
		}
	}

	void Editor::OnMouseDown(const clan::InputEvent& ev)
	{
		if (m_Active)
		{
			//[process global inputs here]

			if (MouseOverUI(m_GUIContext))
				return;

			m_ReceivedMouseDown = true;

			if (m_Tool == Tool::None)
			{
				if (ev.ctrl)
				{
					m_Dragging = true;
					//GUI::getSingleton().GetContext()->GetRootElement()->SetAttribute("style", "cursor: Move;");
					//GUI::getSingleton().GetContext()->SetMouseCursor("Move");
				}

				OnMouseDown_Selection(ev);
			}
			else
			{
				auto tool = m_ShapeTools[m_Tool];
				if (tool)
				{
					Vector2 mpos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
					const auto mouseInputType = GetMouseInputType(ev.id);
					if (mouseInputType != ShapeTool::MouseInput::None)
						tool->MousePress(ReturnScreenToWorld(mpos.x, mpos.y), mouseInputType, ev.shift, ev.ctrl, ev.alt);
				}
			}
		}
	}

	void Editor::OnMouseUp(const clan::InputEvent& ev)
	{
		if (m_Active)
		{
			if (m_ReceivedMouseDown)
			{
				bool inputBlockedByTool = false;
				if (m_Tool == Tool::None)
				{
					if (m_Dragging/*m_Tool == Tool::Move*/)
					{
						Vector2 mouseInWorld = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
						TranslateScreenToWorld(&mouseInWorld.x, &mouseInWorld.y);

						auto delta = mouseInWorld - m_DragFrom;
						delta.x = ToSimUnits(delta.x); delta.y = ToSimUnits(delta.y);

						ForEachSelected([delta](const EntityPtr& entity) { entity->SetPosition(entity->GetPosition() + delta); });

						//m_EditorOverlay->SetOffset(Vector2());
					}
					else
					{
						switch (ev.id)
						{
						case clan::mouse_left:
							OnMouseUp_Selection(ev);
							break;
						};
					}
				}
				else
				{
					auto tool = m_ShapeTools[m_Tool];
					if (tool)
					{
						Vector2 mpos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
						const auto mouseInputType = GetMouseInputType(ev.id);
						if (mouseInputType != ShapeTool::MouseInput::None)
							inputBlockedByTool = tool->MouseRelease(ReturnScreenToWorld(mpos.x, mpos.y), mouseInputType, ev.shift, ev.ctrl, ev.alt);
					}
				}
				if (!inputBlockedByTool)
				{
					switch (ev.id)
					{
					case clan::mouse_right:
						//if (m_EditorOverlay->m_Selected.empty())
						//	OnMouseUp_Selection(ev);
						//ShowContextMenu(Vector2i(ev.mouse_pos.x, ev.mouse_pos.y), m_EditorOverlay->m_Selected);
						break;
					case clan::mouse_wheel_up:
						if (ev.ctrl)
						{
							if (ev.alt)
								ForEachSelected([](const EntityPtr& entity) { entity->SetAngle(entity->GetAngle() + s_pi * 0.01f); });
							else
								ForEachSelected([](const EntityPtr& entity) { entity->SetAngle(entity->GetAngle() + s_pi * 0.1f);  });
						}
						else
							m_EditCam->SetZoom(m_EditCam->GetZoom() + 0.05f);
						break;
					case clan::mouse_wheel_down:
						if (ev.ctrl)
						{
							if (ev.alt)
								ForEachSelected([](const EntityPtr& entity) { entity->SetAngle(entity->GetAngle() - s_pi * 0.01f);  });
							else
								ForEachSelected([](const EntityPtr& entity) { entity->SetAngle(entity->GetAngle() - s_pi * 0.1f);  });
						}
						else
							m_EditCam->SetZoom(m_EditCam->GetZoom() - 0.05f);
						break;
					};
				}
			}

			//GUI::getSingleton().GetContext()->GetRootElement()->SetAttribute("style", "cursor: Arrow;");
			//GUI::getSingleton().GetContext()->SetMouseCursor("Arrow");
			m_Dragging = false;
			m_ReceivedMouseDown = false;
		}
	}

	void Editor::OnMouseMove(const clan::InputEvent& ev)
	{
		if (m_Active)
		{
			if (m_Tool != Tool::None)
			{
				auto tool = m_ShapeTools[m_Tool];
				if (tool)
				{
					Vector2 mpos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
					tool->MouseMove(ReturnScreenToWorld(mpos.x, mpos.y), ev.shift, ev.ctrl, ev.alt);
				}
			}
			else if (m_ReceivedMouseDown)
			{
				if (m_Dragging)
				{
					OnMouseMove_Move(ev);
				}
				else
					UpdateSelectionRectangle(Vector2i(ev.mouse_pos.x, ev.mouse_pos.y), true);
			}
		}
	}

	void Editor::OnMouseDown_Selection(const clan::InputEvent& ev)
	{
		m_DragFrom = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
		TranslateScreenToWorld(&m_DragFrom.x, &m_DragFrom.y);
		m_SelectionRectangle.right = m_SelectionRectangle.left = m_DragFrom.x;
		m_SelectionRectangle.bottom = m_SelectionRectangle.top = m_DragFrom.y;
	}

	void Editor::OnMouseUp_Selection(const clan::InputEvent& ev)
	{
		Vector2 mousePos = Vector2i(ev.mouse_pos.x, ev.mouse_pos.y);
		TranslateScreenToWorld(&mousePos.x, &mousePos.y);

		// Detect click (no mouse movement between press and release)
		if (Vector2::distance(m_DragFrom, mousePos) <= Vector2(1.f, 1.f).length())
		{
			clan::Rectf nearRect(mousePos.x - 50.f, mousePos.y - 50.f, mousePos.x + 50.f, mousePos.y + 50.f);

			std::set<EntityPtr> entitiesUnselected;
			if (!m_ShiftSelect)
				entitiesUnselected = m_SelectedEntities;

			// Select entities found within the updated selection rectangle
			std::vector<EntityPtr> entitiesUnderMouse;
			GetEntitiesOverlapping(entitiesUnderMouse, nearRect, QueryType::General);
			for (auto it = entitiesUnderMouse.begin(), end = entitiesUnderMouse.end(); it != end; ++it)
			{
				const EntityPtr& map_entity = *it;
				if (!m_ShiftSelect)
					entitiesUnselected.erase(map_entity);
				if (!m_AltSelect)
					SelectEntity(map_entity);
				else
					DeselectEntity(map_entity);
			}
			// Deselect all the entities not found in the updated rectangle
			for (auto it = entitiesUnselected.begin(), end = entitiesUnselected.end(); it != end; ++it)
			{
				const EntityPtr& map_entity = *it;
				if (!m_AltSelect)
					DeselectEntity(map_entity);
				else
					SelectEntity(map_entity);
			}
		}
		UpdateSelectionRectangle(mousePos, false);
	}

	void Editor::OnMouseMove_Move(const clan::InputEvent& ev)
	{
		Vector2i mousePos(ev.mouse_pos.x, ev.mouse_pos.y);

		Vector2 mouseInWorld = mousePos;
		TranslateScreenToWorld(&mouseInWorld.x, &mouseInWorld.y);

		//m_EditorOverlay->SetOffset(mouseInWorld - m_DragFrom);
	}

	void Editor::OnWindowResize(int x, int y)
	{
		if (m_Active)
		{
			//m_Background->SetProperty("width", Rocket::Core::Property(m_DisplayWindow.get_gc().get_width(), Rocket::Core::Property::PX));
			//m_Background->SetProperty("height", Rocket::Core::Property(m_DisplayWindow.get_gc().get_height(), Rocket::Core::Property::PX));
		}
	}

	//void ClearCtxMenu(MenuItem *menu)
	//{
	//	menu->RemoveAllChildren();
	//}

	//boost::intrusive_ptr<MenuItem> AddMenuItem(MenuItem* parent, const std::string& title, std::function<void (const MenuItemEvent&)> on_clicked)
	//{
	//	return AddMenuItem(parent, title, "", on_clicked);
	//}

	class FunctorEventHandler : public Gwen::Event::Handler
	{
	public:
		FunctorEventHandler(std::function<void (Gwen::Controls::Base* con)> fn)
		{
			m_Functor = fn;
		}

		void Call(Gwen::Controls::Base* con)
		{
			m_Functor(con);
		}

	private:
		std::function<void (Gwen::Controls::Base* con)> m_Functor;
	};

	class DynamicMenu
	{
	public:
		DynamicMenu(Gwen::Controls::Base* parent)
			: m_Menu(std::make_shared<Gwen::Controls::Menu>(parent))
		{
		}

		virtual ~DynamicMenu()
		{
			ClearItems();
		}

		Gwen::Controls::MenuItem* AddItem(const std::string& title, std::function<void (Gwen::Controls::Base*)> on_clicked)
		{
			auto clickHandler = std::make_shared<FunctorEventHandler>(on_clicked);
			m_ActionHandlers.push_back(clickHandler);
			auto item = m_Menu->AddItem(title)->SetAction(clickHandler.get(), &FunctorEventHandler::Call);
			return item;
		}

		void ClearItems()
		{
			m_ActionHandlers.clear();
			m_Menu->ClearItems();
		}

		void Show(const Vector2i& position)
		{
			m_Menu->SetPos(position.x, position.y);
			m_Menu->Show();
		}

	private:
		std::shared_ptr<Gwen::Controls::Menu> m_Menu;
		std::vector<std::shared_ptr<Gwen::Event::Handler>> m_ActionHandlers;
	};

	void Editor::ShowContextMenu(const Vector2i& position, const std::set<EntityPtr>& entities)
	{
		m_ContextMenu = std::make_shared<DynamicMenu>(m_GUIContext);
		m_ContextMenu->AddItem("fart", [](Gwen::Controls::Base* pControl) {SendToConsole("Fart");});
		m_ContextMenu->Show(position);
		
		//ClearCtxMenu(m_PropertiesMenu.get());
		//ClearCtxMenu(m_EntitySelectionMenu.get());

		//AddMenuItem(m_EntitySelectionMenu.get(),
		//	"Deselect All",
		//	std::bind(&Editor::DeselectAll, this));

		//for (auto it = entities.begin(), end = entities.end(); it != end; ++it)
		//{
		//	const auto &entity = *it;

		//	const std::string title = (entity->GetName().empty() ? std::string("Unnamed") : entity->GetName()) +
		//		(entity->IsSyncedEntity() ? " (" +  boost::lexical_cast<std::string>(entity->GetID()) + ")" : "");

		//	// Add an item for this entity to the Properties sub-menu
		//	auto item = AddMenuItem(m_PropertiesMenu.get(),
		//		title, entity->GetName(),
		//		[this, entity](const MenuItemEvent& e) { CreatePropertiesWindow(entity); }
		//	);
		//	item->SetBGColour(m_EditorOverlay->GetColour(entity));

		//	// Add an item for this entity to the Select sub-menu
		//	item = AddMenuItem(m_EntitySelectionMenu.get(),
		//		title, entity->GetName(),
		//		[this, entity](const MenuItemEvent& e) { if (!m_ShiftSelect) DeselectAll(); SelectEntity(entity); }
		//	);
		//	item->SetBGColour(m_EditorOverlay->GetColour(entity));
		//}

		//m_RightClickMenu->Show(position.x, position.y);
	}

	bool Editor::TranslateScreenToWorld(ViewportPtr viewport, float* x, float* y) const
	{
		clan::Rectf worldArea, screenArea;
		viewport->CalculateScreenArea(m_DisplayWindow.get_gc(), worldArea, true);
		viewport->CalculateScreenArea(m_DisplayWindow.get_gc(), screenArea, false);

		const bool withinViewport = *x >= screenArea.left && *y >= screenArea.top;
		*x -= screenArea.left, *y -= screenArea.top;

		*x *= (1 / m_EditCam->GetZoom());
		*y *= (1 / m_EditCam->GetZoom());
		*x += worldArea.left, *y += worldArea.top;

		return withinViewport;
	}

	bool Editor::TranslateScreenToWorld(float* x, float* y) const
	{
		return TranslateScreenToWorld(m_Viewport, x, y);
	}

	Vector2 Editor::ReturnScreenToWorld(float x, float y) const
	{
		TranslateScreenToWorld(&x, &y);
		return Vector2(x, y);
	}

	Vector2 Editor::GetMousePositionInWindow() const
	{
		auto pos = m_DisplayWindow.get_ic().get_mouse().get_position();
		return Vector2((float)pos.x, (float)pos.y);
	}

	std::pair<Vector2, ViewportPtr> Editor::GetPositionInWorldAndViewport(Vector2 pos_in_window) const
	{
		std::vector<ViewportPtr> viewports;
		viewports.push_back(m_Viewport);
		for (ViewportPtr viewport : viewports)
		{
			auto translatedPos = pos_in_window;
			if (TranslateScreenToWorld(viewport, &translatedPos.x, &translatedPos.y))
				return std::make_pair(translatedPos, viewport);
		}
		return std::make_pair(Vector2::zero(), ViewportPtr());
	}

	std::pair<Vector2, ViewportPtr> Editor::GetMousePositionInWorldAndViewport() const
	{
		return GetPositionInWorldAndViewport(GetMousePositionInWindow());
	}

	ViewportPtr Editor::GetViewportUnderMouse() const
	{
		return GetMousePositionInWorldAndViewport().second;
	}

	Vector2 Editor::GetMousePositionInWorld() const
	{
		return GetMousePositionInWorldAndViewport().first;
	}

	void Editor::UpdateSelectionRectangle(const Vector2& pointer_position, bool translate_position)
	{
		float oldWidth = std::abs(m_SelectionRectangle.get_width());
		float oldHeight = std::abs(m_SelectionRectangle.get_height());

		m_SelectionRectangle.right = pointer_position.x;
		m_SelectionRectangle.bottom = pointer_position.y;
		if (translate_position)
			TranslateScreenToWorld(&m_SelectionRectangle.right, &m_SelectionRectangle.bottom);

		std::set<EntityPtr> entitiesUnselected;
		if (!m_ShiftSelect && (std::abs(m_SelectionRectangle.get_width()) < oldWidth || std::abs(m_SelectionRectangle.get_height()) < oldHeight))
			entitiesUnselected = m_SelectedEntities;

		// Select entities found within the updated selection rectangle
		std::vector<EntityPtr> entitiesUnderMouse;
		GetEntitiesOverlapping(entitiesUnderMouse, m_SelectionRectangle, QueryType::General);
		for (auto it = entitiesUnderMouse.begin(), end = entitiesUnderMouse.end(); it != end; ++it)
		{
			const EntityPtr& map_entity = *it;
			if (!m_ShiftSelect)
				entitiesUnselected.erase(map_entity);
			if (!m_AltSelect)
				SelectEntity(map_entity);
			else
				DeselectEntity(map_entity);
		}
		// Deselect all the entities not found in the updated rectangle
		for (auto it = entitiesUnselected.begin(), end = entitiesUnselected.end(); it != end; ++it)
		{
			const EntityPtr& map_entity = *it;
			if (!m_AltSelect)
				DeselectEntity(map_entity);
			else
				SelectEntity(map_entity);
		}

		m_EditorWorld->SetSelectionBox(m_SelectionRectangle);
	}

	void Editor::SetFilebrowserOpenOverride(const Editor::FileBrowserOverrideFn_t& fn)
	{
		m_OpenDialogOverride = fn;
	}

	void Editor::SetFilebrowserSaveOverride(const Editor::FileBrowserOverrideFn_t& fn)
	{
		m_SaveDialogOverride = fn;
	}

	void Editor::SelectEntity(const EntityPtr& entity)
	{
		m_SelectedEntities.insert(entity);
		m_EditorWorld->SelectEntity(entity);
	}

	void Editor::DeselectEntity(const EntityPtr& entity)
	{
		m_SelectedEntities.erase(entity);
		m_EditorWorld->DeselectEntity(entity);
	}

	void Editor::DeselectAll()
	{
		auto selectedEntities = m_SelectedEntities;
		for (auto it = selectedEntities.begin(), end = selectedEntities.end(); it != end; ++it)
			DeselectEntity(*it);
	}

	void Editor::SelectEntityWithID(const ObjectID id)
	{
		auto entity = m_EntityManager->GetEntity(id, true);
		if (entity)
			SelectEntity(entity);
	}

	void Editor::GoToEntityWithID(const ObjectID id)
	{
		auto entity = m_EntityManager->GetEntity(id, true);
		if (entity)
			GoToEntity(entity);
	}

	size_t Editor::GetNumSelected() const
	{
		//return m_Selected.size();
		return -1;
	}

	void Editor::ForEachSelected(std::function<void (const EntityPtr&)> fn)
	{
	//	for (auto it = m_EditorOverlay->m_Selected.begin(), end = m_EditorOverlay->m_Selected.end(); it != end; ++it)
	//	{
	//		fn(*it);
	//	}
	}

	void Editor::ForEachSelectedWithColours(std::function<void (const EntityPtr&, const clan::Colorf&)> fn)
	{
	//	for (auto it = m_EditorOverlay->m_Selected.begin(), end = m_EditorOverlay->m_Selected.end(); it != end; ++it)
	//	{
	//		fn(*it, m_EditorOverlay->GetColour(it));
	//	}
	}

	void Editor::DoWithArchetypeFactory(const std::string& archetype_name, std::function<void (const ResourcePointer<ArchetypeFactory>&)> fn)
	{
		if (m_ArchetypeFactoryLoadConnections.empty())
			m_NextFactoryId = 0;

		m_ArchetypeFactoryLoadConnections[m_NextFactoryId++] = std::make_shared<boost::signals2::scoped_connection>(
			ResourceManager::getSingleton().GetResource(
			"ArchetypeFactory", archetype_name, [fn](ResourceDataPtr& resource)
		{
			fn(ResourcePointer<ArchetypeFactory>(resource));
		}));
	}

	namespace {
		template <typename SelectorFnT>
		clan::Rectf GetBBOf(SelectorFnT selector_fn)
		{
			clan::Rectf bb(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
			selector_fn([&bb](const EntityPtr& entity)
			{
				auto transform = entity->GetComponent<ITransform>(); FSN_ASSERT(transform);
				auto pos = transform->GetPosition();
				if (pos.x < bb.left)
					bb.left = pos.x;
				if (pos.y < bb.top)
					bb.top = pos.y;
				if (pos.x > bb.right)
					bb.right = pos.x;
				if (pos.y > bb.bottom)
					bb.bottom = pos.y;
			});
			return bb;
		}
	}

	clan::Rectf Editor::GetBBOfSelected()
	{
		return GetBBOf(std::bind(&Editor::ForEachSelected, this, std::placeholders::_1));
	}

	void Editor::GetEntitiesOverlapping(std::vector<EntityPtr> &out, const clan::Rectf &rectangle, const Editor::QueryType query_type)
	{
		// Figure out where the rectangle is in the world
		Vector2 top_left(ToSimUnits(std::min(rectangle.left, rectangle.right)), ToSimUnits(std::min(rectangle.top, rectangle.bottom)));
		Vector2 bottom_right(ToSimUnits(std::max(rectangle.left, rectangle.right)), ToSimUnits(std::max(rectangle.top, rectangle.bottom)));

		switch (query_type)
		{
		case QueryType::General:
			{
				m_EntityManager->QueryRect([&out](const EntityPtr& ent)->bool { out.push_back(ent); return true; }, top_left, bottom_right);
				clan::Rectf simUnitsRectangle(top_left.x, top_left.y, bottom_right.x, bottom_right.y);
				for (auto it = m_NonStreamedEntities.begin(), end = m_NonStreamedEntities.end(); it != end; ++it)
				{
					auto pos = (*it)->GetPosition();
					if (simUnitsRectangle.contains(clan::Vec2f(pos.x, pos.y)))
						out.push_back(*it);
				}
			}
			break;

		case QueryType::Physical:
			{
				// Make an AABB for the given rect
				b2AABB aabb;
				aabb.lowerBound.Set(top_left.x, top_left.y);
				aabb.upperBound.Set(bottom_right.x, bottom_right.y);

				// Query the world for overlapping shapes
				EntityQuery callback(&out, [](b2Fixture*, bool&) {return true;});
				m_Box2DWorld->Getb2World()->QueryAABB(&callback, aabb);
			}
			break;
		};
	}

	void Editor::BuildCreateEntityScript()
	{
		auto& scriptManager = ScriptManager::getSingleton();

		m_CreateEntityFn.reset();

		auto module = scriptManager.GetModule("core_create_entity");
		// Load the script file
		auto script = OpenString_PhysFS("Data/core/create_entity.as");
		// Generate and add the basecode section (also preprocess the script)
		int r = module->AddCode("basecode", m_AngelScriptWorld->GenerateBaseCodeForScript(script));
		FSN_ASSERT(r >= 0);
		// Add the pre-processed script
		r = module->AddCode("/Data/core/create_entity.as", script);
		FSN_ASSERT(r >= 0);
		// Attempt to build
		r = module->Build();
		if (r < 0)
			SendToConsole("Failed to build /Data/core/create_entity.as");

		m_CreateEntityFn = module->GetASModule()->GetFunctionByName("createEntity");
	}

	EntityPtr Editor::CreateEntity(const std::string& transform_type, const Vector2& position, float angle, bool synced, bool streaming)
	{
		ComponentPtr transformCom = m_ComponentFactory->InstantiateComponent(transform_type);

		auto entity = std::make_shared<Entity>(m_EntityManager.get(), transformCom);

		ObjectID id = 0;
		if (synced)
			id = m_EntityInstantiator->GetFreeGlobalID();
		entity->SetID(id);

		if (!streaming)
		{
			entity->SetDomain(SYSTEM_DOMAIN);
			// TODO: limit max non-streamed-entities
			m_NonStreamedEntities.push_back(entity);
		}

		{
			auto transform = entity->GetComponent<ITransform>();
			transform->Position.Set(position);
			transform->Angle.Set(angle);
		}
		entity->SynchroniseParallelEdits();

		m_EntityManager->AddEntity(entity);

		return entity;
	}

	void Editor::DeleteEntity(const EntityPtr& entity)
	{
		DeselectEntity(entity);

		//ClearCtxMenu(m_PropertiesMenu.get());
		//ClearCtxMenu(m_EntitySelectionMenu.get());

		auto it = std::remove(m_NonStreamedEntities.begin(), m_NonStreamedEntities.end(), entity);
		if (it != m_NonStreamedEntities.end())
			m_NonStreamedEntities.erase(it, m_NonStreamedEntities.end());
		
		m_EntityManager->RemoveEntity(entity);
	}

	void Editor::AddEntityToDelete(const EntityPtr& entity)
	{
		m_ToDelete.push_back(entity);
	}

	void Editor::CreateArchetypeInstance(const std::string& archetype_name, const Vector2& position, float angle, clan::Rectf selected_area, float grid_size)
	{
		DoWithArchetypeFactory(archetype_name, [this, position, angle, selected_area, grid_size](const ResourcePointer<ArchetypeFactory>& archetype_factory)
		{
			Vector2 p(position);
			const float subrectSizef = grid_size * 20.0f;
			const size_t subrectSize = size_t(subrectSizef / grid_size) + 1;
			if (selected_area.contains(clan::Vec2f(p.x, p.y)))
			{
				const size_t numRows = size_t(selected_area.get_height() / grid_size) + 1;
				const size_t numCols = size_t(selected_area.get_width() / grid_size) + 1;
				for (size_t subrectTop = 0; subrectTop < numRows; subrectTop += subrectSize)
				{
					for (size_t subrectLeft = 0; subrectLeft < numCols; subrectLeft += subrectSize)
					{
						clan::Rect subrect(subrectLeft, subrectTop, subrectLeft + subrectSize, subrectTop + subrectSize);
						QueueAction([this, archetype_factory, selected_area, subrect, angle, grid_size]()
						{
							SendToConsole("Spawning subrect [" + boost::lexical_cast<std::string>(subrect.top) + ", " + boost::lexical_cast<std::string>(subrect.left) + "]");
							tbb::parallel_for(tbb::blocked_range2d<size_t>(subrect.top, subrect.bottom, subrect.left, subrect.right),
							[this, archetype_factory, selected_area, angle, grid_size](const tbb::blocked_range2d<size_t>& range)
							{
								for (size_t row = range.rows().begin(); row < range.rows().end(); ++row)
								{
									for (size_t col = range.cols().begin(); col < range.cols().end(); ++col)
									{
										float y = selected_area.top + row * grid_size, x = selected_area.left + col * grid_size;
										if (x <= selected_area.right && y <= selected_area.bottom)
										{
											Vector2 simPos(ToSimUnits(x), ToSimUnits(y));
											auto entity = archetype_factory->MakeInstance(m_ComponentFactory.get(), simPos, angle);
											m_EntityManager->AddEntity(entity);

											//SendToConsole("Instantiate [" + boost::lexical_cast<std::string>(simPos.x) + ", " + boost::lexical_cast<std::string>(simPos.y) + "]");
										}
									}
								}
							});
						});
					}
				}
			}
			else
			{
				Vector2 simPos(ToSimUnits(p.x), ToSimUnits(p.y));
				auto entity = archetype_factory->MakeInstance(m_ComponentFactory.get(), simPos, angle);
				m_EntityManager->AddEntity(entity);
			}
		});
	}

	void Editor::SaveEntities(OCellStream& file, const std::set<EntityPtr>& entities)
	{
		auto bb = GetBBOf([&entities](std::function<void (EntityPtr)> fn) { for (const auto& entity : entities) fn(entity); });
		auto c = bb.get_center();
		Vector2 center(c.x, c.y);
		if (file)
		{
			IO::Streams::CellStreamWriter writer(&file);
			writer.Write(GetNumSelected());
			for (const auto& entity : entities)
			{
				{
					auto transform = entity->GetComponent<ITransform>();
					auto pos = transform->GetPosition();
					pos -= center;
					writer.Write(pos.x);
					writer.Write(pos.y);
					writer.Write(transform->Angle.Get());
				}
				EntitySerialisationUtils::SaveEntity(file, entity, true, EntitySerialisationUtils::EditableBinary);
			}
		}
		else
			FSN_EXCEPT(FileSystemException, "Failed to create entity clipboard file");
	}

	std::vector<EntityPtr> Editor::LoadEntities(std::shared_ptr<ICellStream> file, const Vector2& offset, float base_angle)
	{
		std::vector<EntityPtr> loadedEntities;

		Vector2 simOffset(ToSimUnits(offset.x), ToSimUnits(offset.y));

		IO::Streams::CellStreamReader reader(file.get());
		size_t numEntities = reader.ReadValue<size_t>();
		for (size_t i = 0; i < numEntities; ++i)
		{
			Vector2 entityPos;
			float entityAngle = 0.f;
			{
				reader.Read(entityPos.x);
				reader.Read(entityPos.y);
				reader.Read(entityAngle);
			}

			EntityPtr entity;
			std::tie(entity, file) = EntitySerialisationUtils::LoadEntityImmeadiate(std::move(file), true, 0, EntitySerialisationUtils::EditableBinary, m_ComponentFactory.get(), m_EntityManager.get(), m_EntityInstantiator.get());

			if (entity->GetID())
				entity->SetID(m_EntityInstantiator->GetFreeGlobalID());
			entity->SetName("");

			{
				auto transform = entity->GetComponent<ITransform>();
				transform->Position.Set(simOffset + entityPos);
				transform->Angle.Set(base_angle + entityAngle);
			}

			m_EntityManager->AddEntity(entity);
			if (entity->GetDomain() == SYSTEM_DOMAIN)
				m_NonStreamedEntities.push_back(entity);

			loadedEntities.push_back(entity);
		}

		return loadedEntities;
	}

	void Editor::CreatePrefab(const std::string& name, const std::set<EntityPtr>& entities)
	{
		IO::PhysFSStream file(name, IO::Write);
		if (file)
			SaveEntities(file, entities);
		else
			FSN_EXCEPT(FileSystemException, "Failed to create prefab file");
	}

	std::vector<EntityPtr> Editor::InstantiatePrefab(const std::string& filename, const Vector2& offset, float base_angle)
	{
		auto file = std::make_shared<IO::PhysFSStream>(filename, IO::Read);
		if (file)
			return LoadEntities(file, offset, base_angle);
		else
			FSN_EXCEPT(FileSystemException, "Failed to open file");
	}

	void Editor::CopySelectedEntities()
	{
		//if (auto file = m_DataArchiver->CreateDataFile("editor.entity_clipboard"))
		//	SaveEntities(*file, m_EditorOverlay->m_Selected);
		//else
		//	FSN_EXCEPT(FileSystemException, "Failed to create entity clipboard file");
	}

	void Editor::PasteEntities(const Vector2& offset, float base_angle)
	{
		if (auto selfishFile = m_DataArchiver->LoadDataFile("editor.entity_clipboard"))
			LoadEntities(std::move(selfishFile), offset, base_angle);
		else
			FSN_EXCEPT(FileSystemException, "Failed to open data file");
	}

	void Editor::CreatePropertiesWindow(const EntityPtr& entity, const std::function<void (void)>& close_callback)
	{
	}

	void Editor::CreatePropertiesWindow(const std::vector<EntityPtr>& entities, const std::function<void (void)>& close_callback)
	{
	}

	void Editor::CreatePropertiesWindowForSelected()
	{
	}

	void Editor_StartResourceEditor(const std::string& path, Editor* obj)
	{
		obj->StartResourceEditor(path);
	}

	ViewportPtr Editor_GetViewport(const std::string& name, Editor* obj)
	{
		return obj->GetViewport();
	}

	void Editor::RegisterScriptInterface(asIScriptEngine* engine)
	{
		int r;
		RegisterSingletonType<Editor>("Editor", engine);
		r = engine->RegisterObjectMethod("Editor", "Entity CreateEntity(const string &in, const Vector &in, float, bool, bool)", asMETHOD(Editor, CreateEntity), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "void createArchetypeInstance(const string &in, const Vector &in, float)", asMETHOD(Editor, CreateArchetypeInstance), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "bool isResourceEditable(const string &in) const", asMETHOD(Editor, IsResourceEditable), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "void startResourceEditor(const string &in)", asFUNCTION(Editor_StartResourceEditor), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "bool goToEntity(const Entity &in)", asMETHOD(Editor, GoToEntity), asCALL_THISCALL); FSN_ASSERT(r >= 0);
		r = engine->RegisterObjectMethod("Editor", "Viewport getViewport(const string &in)", asFUNCTION(Editor_GetViewport), asCALL_CDECL_OBJLAST); FSN_ASSERT(r >= 0);
	}

}
