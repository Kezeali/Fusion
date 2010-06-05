/*
  Copyright (c) 2006-2007 Fusion Project Team

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

		
	File Author(s):

		Elliot Hayward
*/

#ifndef Header_FusionEngine_ResourcePointer
#define Header_FusionEngine_ResourcePointer

#if _MSC_VER > 1000
#	pragma once
#endif

#include "FusionCommon.h"

#include "FusionResource.h"

#ifdef _DEBUG
#include "FusionConsole.h"
#endif

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

/*!
 * Define FSN_RESOURCEPOINTER_USE_WEAKPTR to use built-in reference counting for garbage collection.
 * Otherwise std::shared_ptr will be used. <br>
 * Using shared_ptr makes the implementation much simpler on my side :)
 */
//#define FSN_RESOURCEPOINTER_USE_WEAKPTR

namespace FusionEngine
{

	//! Resource container weak pointer
	typedef boost::weak_ptr<ResourceContainer> ResourceWpt;

	//! Makes accessing resource data easier.
	/*!
	 * The basic resource class accesses loaded data via a void*.
	 * To access data by it's actual type a Resource can be
	 * held by an instance of the ResourcePointer class.
	 *
	 * \sa ResourceManager | ResourceContainer
	 */
	template<typename T>
	class ResourcePointer
	{
	protected:
		ResourceDataPtr m_Resource;

	public:
		//! Basic Constructor
		/*!
		 * Creates a null resource pointer
		 */
		ResourcePointer()
		{
		}

		//! Constructor
		ResourcePointer(ResourceDataPtr &resource)
			: m_Resource(resource)
		{
		}

		//! Copy constructor
		ResourcePointer(const ResourcePointer<T>& other)
			: m_Resource(other.m_Resource)
		{
		}


		//! Destructor
		~ResourcePointer()
		{
//#if defined (_DEBUG) && defined(FSN_RESOURCEPOINTER_USE_WEAKPTR)
//			SendToConsole("ResourcePointer<" + std::string(typeid(T).name()) +"> to " + 
//				(IsValid() ? "\'" + m_ResourceBox.lock()->GetPath() + "\'" : "invalid resource") + " deleted");
//#elif defined (_DEBUG)
//			SendToConsole("ResourcePointer<" + std::string(typeid(T).name()) +"> to " + 
//				(IsValid() ? "\'" + m_ResourceBox->GetPath() + "\'" : "invalid resource") + " deleted");
//#endif
		}

	public:

		//! Assignment operator
		ResourcePointer<T>& operator=(const ResourcePointer<T>& r)
		{
			m_Resource = r.m_Resource;

			return *this;
		}

		//! Comparison
		bool operator==(const ResourcePointer<T>& other)
		{
			return m_Resource == other.m_Resource;
		}

		//! Returns the resource data ptr
		T* Get()
		{
			return static_cast<T*>( m_Resource->GetDataPtr() );
		}

		//! Returns the resource data ptr
		T const* Get() const
		{
			return static_cast<const T*>( m_Resource->GetDataPtr() );
		}

		//! Indirect member access operator.
		T* operator->()
		{
			return static_cast<T*>( m_Resource->GetDataPtr() );
		}

		T const* operator->() const
		{
			return static_cast<const T*>( m_Resource->GetDataPtr() );
		}

		void SetTarget(ResourceDataPtr &resource)
		{
			m_Resource = resource;
		}

		void Release()
		{
			m_Resource.reset();
		}

		bool IsLoaded() const
		{
			// Check that this pointer is valid (the resource container exists), before checking that the data is valid
			return m_Resource ? m_Resource->IsLoaded() : false; 
		}

		bool IsNull() const
		{
			return !(m_Resource);
		}

		boost::signals2::signal<void ()> &SigDelete() const
		{
			return m_Resource->SigDelete;
		}

		boost::signals2::signal<void ()> &SigLoad() const
		{
			return m_Resource->SigLoad;
		}

		boost::signals2::signal<void ()> &SigUnload() const
		{
			return m_Resource->SigUnload;
		}
	};



	template <typename T>
	class resourcePointerRegisterHelper
	{
	public:
		static void Construct(ResourcePointer<T>* in)
		{
			new (in) ResourcePointer<T>();
		}

		static void Destruct(ResourcePointer<T>* in)
		{
			in->~ResourcePointer<T>();
		}

		static void CopyConstruct(const ResourcePointer<T>& rhs, ResourcePointer<T>* in)
		{
			new (in) ResourcePointer<T>(rhs);
		}

		static ResourcePointer<T>& Assign(const ResourcePointer<T>& rhs, ResourcePointer<T>* lhs)
		{
			*lhs = rhs;
			return *lhs;
		}
	};

	template <typename T>
	void RegisterResourcePointer(const std::string type_name, asIScriptEngine* engine)
	{
		FSN_ASSERT(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(ResourcePointer<T>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		FSN_ASSERT(error_code >= 0 && "Failed to register object type");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(), 
			asBEHAVE_CONSTRUCT, 
			"void f()", 
			asFUNCTION(resourcePointerRegisterHelper<T>::Construct), 
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register constructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_DESTRUCT,
			"void f()",
			asFUNCTION(resourcePointerRegisterHelper<T>::Destruct),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register destructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_CONSTRUCT,
			(std::string("void f(")+type_name+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::CopyConstruct),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register copy constructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_ASSIGNMENT,
			(type_name+"& f(const "+type_name+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::Assign),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register assignment operator");

	}

	template <typename T>
	void RegisterResourcePointer(const std::string V_AS,  //The typename of the resource inside AS
		const std::string T_AS,  //Template parameter typename in AS - must already be
		asIScriptEngine* engine) //registered (or be primitive type)!!
	{
		FSN_ASSERT(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		//// Register the parameter type
		//error_code = engine->RegisterObjectType(T_AS.c_str(), sizeof(T), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		//FSN_ASSERT( error_code >= 0 );

		//// Register the object operator overloads for the sub-type
		//error_code = engine->RegisterObjectBehaviour(T_AS.c_str(), asBEHAVE_CONSTRUCT,  "void f()", asFUNCTION(ConstructType), asCALL_CDECL_OBJLAST); 
		//FSN_ASSERT( error_code >= 0 );
		//error_code = engine->RegisterObjectBehaviour(T_AS.c_str(), asBEHAVE_DESTRUCT,   "void f()", asFUNCTION(DestructType),  asCALL_CDECL_OBJLAST); 
		//FSN_ASSERT( error_code >= 0 );
		//error_code = engine->RegisterObjectBehaviour(T_AS.c_str(), asBEHAVE_ASSIGNMENT, std::string(T_AS+" &f(const "+T_AS+" &in)").c_str(), asMETHODPR(T, operator =, (const T&), T&), asCALL_THISCALL); 
		//FSN_ASSERT( error_code >= 0 );

		error_code = engine->RegisterObjectType(V_AS.c_str(), sizeof(ResourcePointer<T>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		FSN_ASSERT(error_code >= 0 && "Failed to register object type");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(), 
			asBEHAVE_CONSTRUCT, 
			"void f()", 
			asFUNCTION(resourcePointerRegisterHelper<T>::Construct), 
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register constructor");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
			asBEHAVE_DESTRUCT,
			"void f()",
			asFUNCTION(resourcePointerRegisterHelper<T>::Destruct),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register destructor");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
			asBEHAVE_CONSTRUCT,
			(std::string("void f(")+V_AS+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::CopyConstruct),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register copy constructor");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
			asBEHAVE_ASSIGNMENT,
			(V_AS+"& f(const "+V_AS+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::Assign),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register assignment operator");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_CONSTRUCT,
		//	"void f(int)",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::NumConstruct),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register construct(size)");

		error_code = engine->RegisterObjectMethod(V_AS.c_str(),
			"string& GetTag()",
			asFUNCTION(resourcePointerRegisterHelper<T>::GetTag),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register GetTag");

		error_code = engine->RegisterObjectMethod(V_AS.c_str(),
			(T_AS+"& get()").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::GetData),
			asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register GetData");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_INDEX,
		//	(T_AS+"& f(int)").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Index),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register operator[]");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_INDEX,
		//	("const "+T_AS+"& f(int) const").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Index),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register operator[]");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_ASSIGNMENT,
		//	(V_AS+"& f(const "+V_AS+"&in)").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Assign),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register operator=");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	"int size() const",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Size),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register size");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	"void resize(int)",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Resize),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register resize");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	(std::string("void push_back(")+T_AS+"&in)").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::PushBack),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register push_back");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	"void pop_back()",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::PopBack),
		//	asCALL_CDECL_OBJLAST);
		//FSN_ASSERT(error_code >= 0 && "Failed to register pop_back");

		/*	error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		"void erase(int)",
		asFUNCTION(vectorRegisterHelper<T>::Erase),
		asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register erase");

		error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		(std::string("void insert(int, const ")+T_AS+"&)").c_str(),
		asFUNCTION(vectorRegisterHelper<T>::Insert),
		asCALL_CDECL_OBJLAST);
		FSN_ASSERT(error_code >= 0 && "Failed to register insert");
		*/
	}

}

#endif