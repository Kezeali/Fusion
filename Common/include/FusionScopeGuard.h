/*
*  Copyright (c) 2013 Fusion Project Team
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

#ifndef H_FusionScopeGuard
#define H_FusionScopeGuard

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <cstddef>
#include <functional>
#include <new>
#include <exception>

#include "FusionLogger.h"

namespace FusionEngine
{

	/*!
	* ScopeGuard is a general implementation of the "Initialization is
	* Resource Acquisition" idiom.  Basically, it guarantees that a function
	* is executed upon leaving the current scope unless otherwise told.
	*
	* The makeGuard() function is used to create a new ScopeGuard object.
	* It can be instantiated with a lambda function, a std::function<void()>,
	* a functor, or a void(*)() function pointer.
	*
	*
	* Usage example: Add a friend to memory iff it is also added to the db.
	*
	* void User::addFriend(User& newFriend) {
	*   // add the friend to memory
	*   friends_.push_back(&newFriend);
	*
	*   // If the db insertion that follows fails, we should
	*   // remove it from memory.
	*   // (You could also declare this as "auto guard = makeGuard(...)")
	*   auto guard = makeGuard([&] { friends_.pop_back(); });
	*
	*   // this will throw an exception upon error, which
	*   // makes the ScopeGuard execute UserCont::pop_back()
	*   // once the Guard's destructor is called.
	*   db_->addFriend(GetName(), newFriend.GetName());
	*
	*   // an exception was not thrown, so don't execute
	*   // the Guard.
	*   guard.dismiss();
	* }
	*
	* Based on (/ "stolen from"):
	*  https://github.com/facebook/folly/blob/master/folly/ScopeGuard.h
	*   Andrei's and Petru Marginean's CUJ article:
	*     http://drdobbs.com/184403758
	*   and the loki library:
	*     http://loki-lib.sourceforge.net/index.php?n=Idioms.ScopeGuardPointer
	*   and triendl.kj article:
	*     http://www.codeproject.com/KB/cpp/scope_guard.aspx
	*/
	class ScopeGuardImplBase
	{
	public:
		void dismiss() /*noexcept*/
		{
			dismissed_ = true;
		}

	protected:
		ScopeGuardImplBase()
			: dismissed_(false)
		{}

		ScopeGuardImplBase(ScopeGuardImplBase&& other)
			: dismissed_(other.dismissed_)
		{
				other.dismissed_ = true;
		}

		bool dismissed_;
	};

	template<typename FunctionType>
	class ScopeGuardImpl : public ScopeGuardImplBase
	{
	public:
		explicit ScopeGuardImpl(const FunctionType& fn)
			: function_(fn)
		{}

		explicit ScopeGuardImpl(FunctionType&& fn)
			: function_(std::move(fn))
		{}

		ScopeGuardImpl(ScopeGuardImpl&& other)
			: ScopeGuardImplBase(std::move(other)),
			function_(std::move(other.function_))
		{
		}

		~ScopeGuardImpl() /*noexcept*/
		{
			if (!dismissed_)
			{
				execute();
			}
		}

	private:
		void* operator new(size_t) = delete;

		void execute() /*noexcept*/
		{
			try
			{
				function_();
			}
			catch (const std::exception& ex)
			{
				std::stringstream str;
				str << "ScopeGuard cleanup function threw a " << typeid(ex).name() << "exception: " << ex.what();
				AddLogEntry(str.str(), LOG_CRITICAL);
			}
			catch (...)
			{
				std::stringstream str;
				str << "ScopeGuard cleanup function threw a non-exception object";
				AddLogEntry(str.str(), LOG_CRITICAL);
			}
		}

		FunctionType function_;
	};

	template<typename FunctionType>
	ScopeGuardImpl<typename std::decay<FunctionType>::type> makeGuard(FunctionType&& fn)
	{
			return ScopeGuardImpl<typename std::decay<FunctionType>::type>(std::forward<FunctionType>(fn));
	}

	/*!
	* This is largely unneeded if you just use auto for your guards.
	*/
	typedef ScopeGuardImplBase&& ScopeGuard;

}

#endif
