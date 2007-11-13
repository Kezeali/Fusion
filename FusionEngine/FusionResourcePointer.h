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
 * Otherwise boost::shared_ptr will be used. <br>
 * Using shared_ptr makes the implementation much simpler on my side :)
 */
#define FSN_RESOURCEPOINTER_USE_WEAKPTR

namespace FusionEngine
{

	//! Resource container shared pointer
	typedef boost::shared_ptr<ResourceContainer> ResourceSpt;
	//! Resource container weak pointer
	typedef boost::weak_ptr<ResourceContainer> ResourceWpt;

	//! The ResourceToken system is a simple garbage collection system.
	/*!
	 * The ResourceManager manages a collection owning pointers
	 * (Resource objects) and, via reference counting, is notified when
	 * a resource has no external references (ResourceToken) remaining.
	 * If a resource has no related external references, the memory
	 * related to it will be freed next time the ResourceManager does
	 * garbage collection
	 * (e.g. between levels, /after/ loading the next level's data
	 * (to minimise unnecessary re-loading))
	 *
	 * \todo Make this copyable (fix copy-constructor / assignment op.)
	 *
	 * \sa ResourceManager | ResourceContainer
	 */
	template<typename T>
	class ResourcePointer
	{
	protected:
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

		ResourceWpt m_ResourceBox;
		ResourceContainer::ResourcePtrTicket m_Ticket;

		mutable std::string m_Tag;

#else

		ResourceSpt m_ResourceBox;
#endif

	public:
		//! Basic Constructor
		/*!
		 * Creates an invalid resource pointer
		 */
		ResourcePointer()
			: m_Ticket(0)
		{
		}

		//! Constructor
		ResourcePointer(ResourceSpt resource)
			: m_ResourceBox(resource)
		{
#if defined (FSN_RESOURCEPOINTER_USE_WEAKPTR)

			m_Ticket = resource->AddRef();
#endif
		}

		//! Copy constructor
		//template <typename Y>
		//explicit ResourcePointer(ResourcePointer<Y> const& other)
		//	: m_ResourceBox(other.m_ResourceBox)
		//{
		//	m_Ticket = r.m_Ticket;
		//	//m_ResourceDestructionSlot = m_Resource->OnDestruction.connect(this, &ResourcePointer::onResourceDestruction);
		//}

#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR
		template <typename Y>
		ResourcePointer(ResourcePointer<Y> const& other)
			: m_ResourceBox(other.m_ResourceBox)
		{
			if (ResourceSpt r = m_ResourceBox.lock())
				m_Ticket = r->AddRef();

		}
#endif

#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

		ResourcePointer(ResourcePointer const& other)
			: m_ResourceBox(other.m_ResourceBox)
		{
			if (ResourceSpt r = m_ResourceBox.lock())
				m_Ticket = r->AddRef();
		}

#endif


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

#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			if (ResourceSpt r = m_ResourceBox.lock())
				r->DropRef(m_Ticket);
#endif
		}

	public:

#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

		//! Templated assignment operator
		template<class Y>
		ResourcePointer & operator=(ResourcePointer<Y> const & r)
		{
			m_ResourceBox = r.m_ResourceBox;

			if (ResourceSpt r = m_ResourceBox.lock())
				m_Ticket = r->AddRef();
			return *this;
		}
#endif


#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

		//! Assignment operator for weak_ptr version
		ResourcePointer & operator=(ResourcePointer const & r)
		{
			m_ResourceBox = r.m_ResourceBox;

			if (ResourceSpt r = m_ResourceBox.lock())
				m_Ticket = r->AddRef();

			return *this;
		}

#endif

		const std::string& GetTag() const
		{
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			if (ResourceSpt r = m_ResourceBox.lock())
				m_Tag = r->GetTag();
			return m_Tag;

#else
			return m_ResourceBox->GetTag();

#endif
		}

		//! Returns the resource data ptr
		T* GetDataPtr()
		{
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			if (ResourceSpt r = m_ResourceBox.lock())
				return (T*)r->GetDataPtr();
			else
				return 0;

#else

			return (T*)m_ResourceBox->GetDataPtr();

#endif
		}

		//! Returns the resource data ptr
		T const* GetDataPtr() const
		{
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			if (ResourceSpt r = m_ResourceBox.lock())
				return (const T*)r->GetDataPtr();
			else
				return 0;

#else

			return (const T*)m_ResourceBox->GetDataPtr();

#endif
		}

		//! Indirect member access operator.
		T* operator->()
		{
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			if (ResourceSpt r = m_ResourceBox.lock())
				return (T*)r->GetDataPtr();
			else
				FSN_EXCEPT(ExCode::ResourceNotLoaded, "ResourcePointer::operator->()", "Resource doesn't exist");
#else

			// using shared_ptr makes the implementation much simpler on my side :)
			return (T*)m_ResourceBox->GetDataPtr();

#endif
		}

		T const* operator->() const
		{
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			if (ResourceSpt r = m_ResourceBox.lock())
				return (const T*)r->GetDataPtr();
			else
				FSN_EXCEPT(ExCode::ResourceNotLoaded, "ResourcePointer::operator->()", "Resource doesn't exist");
#else

			// using shared_ptr makes the implementation much simpler on my side :)
			return (const T*)m_ResourceBox->GetDataPtr();

#endif
		}

		void SetTarget(ResourceSpt resource)
		{
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR
			Release();
#endif

			m_ResourceBox = resource;

#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR
			m_Ticket = m_ResourceBox->AddRef();
#endif
		}

		void Release()
		{
#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			if (ResourceSpt r = m_ResourceBox.lock())
			{
				r->DropRef(m_Ticket);
			}

#else

			m_ResourceBox.reset();

#endif
		}

		bool IsValid() const
		{
			// Check that this pointer is valid (the resource container exists), before checking that the data is valid

#ifdef FSN_RESOURCEPOINTER_USE_WEAKPTR

			ResourceSpt r = m_ResourceBox.lock();
			return r ? r->IsValid() : false;

#else

			return m_ResourceBox ? m_ResourceBox->IsValid() : false; 
#endif
		}

		void onResourceDestruction()
		{
			// nothing
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

		//static void NumConstruct(int size, ResourcePointer<T>* in)
		//{
		//	new (in) ResourcePointer<T>(size);
		//}

		static ResourcePointer<T>& Assign(const ResourcePointer<T>& rhs, ResourcePointer<T>* lhs)
		{
			*lhs = rhs;
			return *lhs;
		}

//		static T* Index(int i, ResourcePointer<T>* lhs)
//		{
//
//#ifdef AS_VECTOR_ASSERTBOUNDS
//			assert(i >= 0 && i < lhs->size() && "Array index out of bounds.");
//#endif
//
//#ifdef AS_VECTOR_CHECKBOUNDS
//			if (i < 0 || i >= (signed)lhs->size())
//			{
//				asIScriptContext* context = asGetActiveContext();
//				if( context )
//					context->SetException("Array Index Out of Bounds.");
//				return 0;
//			}
//#endif
//
//			return &(*lhs)[i];
//		}

		static std::string GetTag(ResourcePointer<T>* lhs)
		{
			return lhs->GetTag();
		}

		static T* GetData(ResourcePointer<T>* lhs)
		{
			return (T*)lhs->GetDataPtr();
		}

		//static void PushBack(const T& in, ResourcePointer<T> *lhs)
		//{
		//	lhs->push_back(in);
		//}

		//static void PopBack(ResourcePointer<T>* lhs)
		//{
		//	lhs->pop_back();
		//}

		/*	static void Erase(int i, std::vector<T>* lhs)
		{
		lhs->erase(Index(i,lhs));
		}

		static void Insert(int i, const T& e, std::vector<T>* lhs)
		{
		lhs->insert(Index(i,lhs), e);
		}
		*/
	};

	template <typename T>
	void RegisterResourcePointer(const std::string type_name, asIScriptEngine* engine)
	{
		assert(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		error_code = engine->RegisterObjectType(type_name.c_str(), sizeof(ResourcePointer<T>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		assert(error_code >= 0 && "Failed to register object type");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(), 
			asBEHAVE_CONSTRUCT, 
			"void f()", 
			asFUNCTION(resourcePointerRegisterHelper<T>::Construct), 
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register constructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_DESTRUCT,
			"void f()",
			asFUNCTION(resourcePointerRegisterHelper<T>::Destruct),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register destructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_CONSTRUCT,
			(std::string("void f(")+type_name+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::CopyConstruct),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register copy constructor");

		error_code = engine->RegisterObjectBehaviour(type_name.c_str(),
			asBEHAVE_ASSIGNMENT,
			(type_name+"& f(const "+type_name+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::Assign),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register assignment operator");

	}

	template <typename T>
	void RegisterResourcePointer(const std::string V_AS,  //The typename of the resource inside AS
		const std::string T_AS,  //Template parameter typename in AS - must already be
		asIScriptEngine* engine) //registered (or be primitive type)!!
	{
		assert(engine && "Passed NULL engine pointer to registerVector");

		int error_code = 0;

		//// Register the parameter type
		//error_code = engine->RegisterObjectType(T_AS.c_str(), sizeof(T), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		//assert( error_code >= 0 );

		//// Register the object operator overloads for the sub-type
		//error_code = engine->RegisterObjectBehaviour(T_AS.c_str(), asBEHAVE_CONSTRUCT,  "void f()", asFUNCTION(ConstructType), asCALL_CDECL_OBJLAST); 
		//assert( error_code >= 0 );
		//error_code = engine->RegisterObjectBehaviour(T_AS.c_str(), asBEHAVE_DESTRUCT,   "void f()", asFUNCTION(DestructType),  asCALL_CDECL_OBJLAST); 
		//assert( error_code >= 0 );
		//error_code = engine->RegisterObjectBehaviour(T_AS.c_str(), asBEHAVE_ASSIGNMENT, std::string(T_AS+" &f(const "+T_AS+" &in)").c_str(), asMETHODPR(T, operator =, (const T&), T&), asCALL_THISCALL); 
		//assert( error_code >= 0 );

		error_code = engine->RegisterObjectType(V_AS.c_str(), sizeof(ResourcePointer<T>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		assert(error_code >= 0 && "Failed to register object type");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(), 
			asBEHAVE_CONSTRUCT, 
			"void f()", 
			asFUNCTION(resourcePointerRegisterHelper<T>::Construct), 
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register constructor");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
			asBEHAVE_DESTRUCT,
			"void f()",
			asFUNCTION(resourcePointerRegisterHelper<T>::Destruct),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register destructor");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
			asBEHAVE_CONSTRUCT,
			(std::string("void f(")+V_AS+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::CopyConstruct),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register copy constructor");

		error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
			asBEHAVE_ASSIGNMENT,
			(V_AS+"& f(const "+V_AS+"&in)").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::Assign),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register assignment operator");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_CONSTRUCT,
		//	"void f(int)",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::NumConstruct),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register construct(size)");

		error_code = engine->RegisterObjectMethod(V_AS.c_str(),
			"string& GetTag()",
			asFUNCTION(resourcePointerRegisterHelper<T>::GetTag),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register GetTag");

		error_code = engine->RegisterObjectMethod(V_AS.c_str(),
			(T_AS+"& get()").c_str(),
			asFUNCTION(resourcePointerRegisterHelper<T>::GetData),
			asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register GetData");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_INDEX,
		//	(T_AS+"& f(int)").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Index),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register operator[]");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_INDEX,
		//	("const "+T_AS+"& f(int) const").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Index),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register operator[]");

		//error_code = engine->RegisterObjectBehaviour(V_AS.c_str(),
		//	asBEHAVE_ASSIGNMENT,
		//	(V_AS+"& f(const "+V_AS+"&in)").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Assign),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register operator=");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	"int size() const",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Size),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register size");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	"void resize(int)",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::Resize),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register resize");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	(std::string("void push_back(")+T_AS+"&in)").c_str(),
		//	asFUNCTION(resourcePointerRegisterHelper<T>::PushBack),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register push_back");

		//error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		//	"void pop_back()",
		//	asFUNCTION(resourcePointerRegisterHelper<T>::PopBack),
		//	asCALL_CDECL_OBJLAST);
		//assert(error_code >= 0 && "Failed to register pop_back");

		/*	error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		"void erase(int)",
		asFUNCTION(vectorRegisterHelper<T>::Erase),
		asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register erase");

		error_code = engine->RegisterObjectMethod(V_AS.c_str(),
		(std::string("void insert(int, const ")+T_AS+"&)").c_str(),
		asFUNCTION(vectorRegisterHelper<T>::Insert),
		asCALL_CDECL_OBJLAST);
		assert(error_code >= 0 && "Failed to register insert");
		*/
	}

}

#endif