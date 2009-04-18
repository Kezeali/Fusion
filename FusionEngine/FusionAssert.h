/*
* Based on POW2_ASSERT by Charles Nicholson at Power of Two Games (now
* defunct, see:
*  http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/)
*/

/*
* Copyright (c) 2008, Power of Two Games LLC
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Power of Two Games LLC nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY POWER OF TWO GAMES LLC ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL POWER OF TWO GAMES LLC BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef Header_FusionEngine_Assert
#define Header_FusionEngine_Assert

namespace FusionEngine { namespace Assert
{
	enum FailBehavior
	{
		Halt,
		Continue,
	};

	typedef FailBehavior (*Handler)(const char* condition, 
									const char* msg, 
									const char* file, 
									int line);

	Handler GetHandler();
	void SetHandler(Handler newHandler);

	FailBehavior ReportFailure(const char* condition, 
							   const char* file, 
							   int line, 
							   const char* msg, ...);
}}

#define FSN_HALT() __debugbreak()
#define FSN_UNUSED(x) do { (void)sizeof(x); } while(0)

#ifdef FSN_ASSERTS_ENABLED
	#define FSN_ASSERT(cond) \
		do \
		{ \
			if (!(cond)) \
			{ \
			if (FusionEngine::Assert::ReportFailure(#cond, __FILE__, __LINE__, 0) == \
					FusionEngine::Assert::Halt) \
					FSN_HALT(); \
			} \
		} while(0)

	#define FSN_ASSERT_MSG(cond, msg, ...) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (FusionEngine::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
					FusionEngine::Assert::Halt) \
					FSN_HALT(); \
			} \
		} while(0)

	#define FSN_ASSERT_FAIL(msg, ...) \
		do \
		{ \
			if (FusionEngine::Assert::ReportFailure(0, __FILE__, __LINE__, (msg), __VA_ARGS__) == \
				FusionEngine::Assert::Halt) \
			FSN_HALT(); \
		} while(0)

	#define FSN_VERIFY(cond) FSN_ASSERT(cond)
	#define FSN_VERIFY_MSG(cond, msg, ...) FSN_ASSERT_MSG(cond, msg, ##__VA_ARGS__)
#else
	#define FSN_ASSERT(condition) \
		do { FSN_UNUSED(condition); } while(0)
	#define FSN_ASSERT_MSG(condition, msg, ...) \
		do { FSN_UNUSED(condition); FSN_UNUSED(msg); } while(0)
	#define FSN_ASSERT_FAIL(msg, ...) \
		do { FSN_UNUSED(msg); } while(0)
	#define FSN_VERIFY(cond) (void)(cond)
	#define FSN_VERIFY_MSG(cond, msg, ...) \
		do { (void)(cond); FSN_UNUSED(msg); } while(0)
#endif

#define FSN_STATIC_ASSERT(x) \
	typedef char fsnStaticAssert[(x) ? 1 : -1];

//lint -esym(751,pow2StaticAssert)
//lint -esym(751,pow2::pow2StaticAssert)

#endif
