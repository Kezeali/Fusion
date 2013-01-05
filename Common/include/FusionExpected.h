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

#ifndef H_FusionExpected
#define H_FusionExpected

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include <functional>
#include <new>
#include <exception>

#include "FusionLogger.h"

namespace FusionEngine
{

	/*!
	* Expected is useful when processing data where you don't want to obfusticate your
	* code with error handling, and/or performance of exception handling is not good
	* enough.
	*
	* Based on:
	*  Andrei Alexandrescu's "Systematic Error Handling in C++" talk
	*  http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
	*  https://skydrive.live.com/?cid=f1b8ff18a2aec5c5&id=F1B8FF18A2AEC5C5%211158
	*/
	template <typename T>
	class Expected
	{
	private:
		union
		{
			T actualValue_;
			std::exception_ptr exception_;
		};

		bool hasValue_;

	public:
		Expected(const T& value)
			: actualValue_(value),
			hasValue_(true)
		{
		}

		Expected(T&& value)
			: actualValue_(std::move(value)),
			hasValue_(true)
		{
		}

		Expected(const std::exception_ptr& exception)
			: exception_(exception),
			hasValue_(false)
		{
		}

		Expected(std::exception_ptr&& exception)
			: exception_(std::move(exception)),
			hasValue_(false)
		{
		}

		Expected(const std::exception& value)
			: Expected(std::make_exception_ptr(value))
		{
		}

		Expected(const Expected& other)
		{
			if (other.hasValue_) // Call the copy constructor of T and place it in the value field
				new(&actualValue_) T(other.actualValue_);
			else
				new(&exception_) std::exception_ptr(other.exception_);
		}

		Expected(Expected&& other)
		{
			if (other.hasValue_) // Call the copy constructor of T and place it in the value field
				new(&actualValue_) T(std::move(other.actualValue_));
			else
				new(&exception_) std::exception_ptr(std::move(other.exception_));
		}

		void swap(Expected& other)
		{
			using namespace std;
			if (hasValue_)
			{
				if (other.hasValue_) // Both have value
				{
					using std::swap;
					swap(actualValue_, other.actualValue_);
				}
				else // This has value, other doesn't
				{
					std::exception_ptr t; // Create a null exception pointer
					t.swap(other.exception_);
					other.exception_.~exception_ptr(); // Destruct the nullptr in other's exception_, just for correctness

					new(&other.actualValue_) T(std::move(actualValue_)); // Move / copy construct this.actualValue_ into other

					new(&exception_) std::exception_ptr(t); // Copy construct the previously swapped exception ptr from other into this.exception_

					std::swap(hasValue_, other.hasValue_);
				}
			}
			else
			{
				if (other.hasValue_) // This doesn't have value, other does
				{
					other.swap(*this); // Recurse to use the previously defined behaviour in the else clause above
				}
				else // Neither have value
				{
					exception_.swap(other.exception_);
				}
			}
		}

	};

}

#endif
