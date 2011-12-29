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
 
#ifndef BOOST_PP_IS_ITERATING

#ifndef H_FusionSerialisationHelper
#define H_FusionSerialisationHelper

#include <boost/preprocessor.hpp>
#include <boost/mpl/vector.hpp>

#include <BitStream.h>
#include <bitset>
#include <tuple>

#include <ClanLib/Display/2D/color.h>

//#include <StringCompressor.h>

namespace FusionEngine
{

	namespace SerialisationUtils
	{
		// Write templates
		template <typename T>
		inline void write(RakNet::BitStream& stream, const T& new_value)
		{
			//static_assert(std::is_fundamental<T>::value, "BitStream probably wont be able to write this type");
			stream.Write(new_value);
		}

		template <typename U>
		inline void write(RakNet::BitStream& stream, const Vector2T<U>& new_value)
		{
			stream.Write(new_value.x);
			stream.Write(new_value.y);
		}

		template <>
		inline void write(RakNet::BitStream& stream, const std::string& new_value)
		{
			stream.WriteCompressed(new_value.c_str());
		}

		template <>
		inline void write(RakNet::BitStream& stream, const CL_Colorf& new_value)
		{
			stream.Write(new_value.a);
			stream.Write(new_value.r);
			stream.Write(new_value.g);
			stream.Write(new_value.b);
		}

		// Read templates
		template <typename T>
		inline void read(RakNet::BitStream& stream, T& out_value)
		{
			//static_assert(std::is_fundamental<T>::value, "BitStream probably wont be able to read this type");
			stream.Read(out_value);
		}

		template <typename U>
		inline void read(RakNet::BitStream& stream, Vector2T<U>& out_value)
		{
			stream.Read(out_value.x);
			stream.Read(out_value.y);
		}

		template <>
		inline void read(RakNet::BitStream& stream, std::string& out_value)
		{
			RakNet::RakString temp;
			if (stream.ReadCompressed(temp))
				out_value.assign(temp.C_String(), temp.C_String() + temp.GetLength());
			else
			{
				FSN_ASSERT_FAIL("Failed to deserialise compressed string");
			}
		}

		template <>
		inline void read(RakNet::BitStream& stream, CL_Colorf& new_value)
		{
			stream.Read(new_value.a);
			stream.Read(new_value.r);
			stream.Read(new_value.g);
			stream.Read(new_value.b);
		}

		// Writes a bool and, if it changed, the value
		template <typename T>
		inline void writeChange(bool all, RakNet::BitStream& stream, bool changed, const T& new_value)
		{
			if (all || changed)
			{
				if (!all)
					stream.Write1();
				write(stream, new_value);
			}
			else if (!all)
			{
				stream.Write0();
			}
		}

		template <>
		inline void writeChange(bool, RakNet::BitStream& stream, bool changed, const bool& new_value)
		{
			stream.Write(new_value);
		}

		template <typename T>
		inline bool readChange(bool all, RakNet::BitStream& stream, T& new_value)
		{
			if (all || stream.ReadBit())
			{
				read(stream, new_value);
				return true;
			}
			else
				return false;
		}

		template <>
		inline bool readChange(bool, RakNet::BitStream& stream, bool& new_value)
		{
			stream.Read(new_value);
			return true;
		}

		template <typename T>
		inline void copyChange(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& delta_data)
		{
			T value;
			if (readChange(false, delta_data, value))
			{
				write(result, value);
				current_data.IgnoreBytes(sizeof(T));
			}
			else
			{
				read(current_data, value);
				write(result, value);
			}
		}
	};

	struct sh_none {};

}

//#define FSN_SER_REP(z, n, text) dataWritten |= force_all || m_Changed.get(n); SerialisationUtils::writeChange<BOOST_PP_CAT(T,n)>(stream, dataWritten, get<n>(new_values));
//#define FSN_DSER_REP(z, n, text) changes.set(n, SerialisationUtils::readChange(false, stream, get<n>(new_values)));
//#define FSN_DSER_REP_FORCE(z, n, text) SerialisationUtils::readChange<BOOST_PP_CAT(T,n)>(true, stream, get<n>(new_values));

#define FSN_SER_REP(z, n, text) SerialisationUtils::writeChange(force_all, stream, m_Changed[n], v ## n);
#define FSN_DSER_REP(z, n, text) changes.set(n, SerialisationUtils::readChange(force_all, stream, v ## n) );
#define FSN_DSER_CALLBACKS_REP(z, n, text) { T ## n value; if (SerialisationUtils::readChange(force_all, stream, value)) { changes.set(n); (obj->* f ## n)(value); } }

#define FSN_DSER_REP_CHANGE(z, n, text) changes.set(n, SerialisationUtils::readChange(false, stream, v ## n) );
#define FSN_DSER_REP_FORCE(z, n, text) SerialisationUtils::readChange(true, stream, v ## n);

#define FSN_CPYSER_REP(z, n, text) SerialisationUtils::copyChange< T ## n >(result, current_data, new_data);

// Allows enumeration of T0&, ... TN& type params
#define REF_PARAM(z, n, data) T ## n &

#define DATA_TYPE(z, n, data) typedef typename T##n type_##n;

#define FSN_PP_PRINT(z, n, data) data

#ifndef MAX_SerialisationHelper_PROPS
#define MAX_SerialisationHelper_PROPS 18
#endif

namespace FusionEngine
{
	template <BOOST_PP_ENUM_BINARY_PARAMS(MAX_SerialisationHelper_PROPS, typename T, = sh_none BOOST_PP_INTERCEPT)>
	struct SerialisationHelper
	{
		//typedef std::tuple<BOOST_PP_ENUM_PARAMS(8, T)> data_type;
		//typedef std::tuple<BOOST_PP_ENUM(n, REF_PARAM, ~)> reference_type;
		typedef boost::mpl::vector<BOOST_PP_ENUM_PARAMS(MAX_SerialisationHelper_PROPS, T)> types;

		//BOOST_PP_REPEAT(n, VAL_PROP, m_V)
		//data_type m_SerialisedValues;

		//BOOST_PP_REPEAT(MAX_SerialisationHelper_PROPS, DATA_TYPE, ~)

		static const size_t NumParams = MAX_SerialisationHelper_PROPS;

		std::bitset<MAX_SerialisationHelper_PROPS> m_Changed;

		SerialisationHelper()
		{}

		inline void unmark()
		{
			m_Changed.reset();
		}

		inline void markChanged(size_t i)
		{
			m_Changed.set(i);
		}

		bool writeChanges(bool force_all, RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(MAX_SerialisationHelper_PROPS, const T, &v))//const data_type& new_values)
		{
			if (!force_all && !m_Changed.any())
				return false;
			BOOST_PP_REPEAT(MAX_SerialisationHelper_PROPS, FSN_SER_REP, ~)
			m_Changed.reset();
			return true;
		}

		void readAll(RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(MAX_SerialisationHelper_PROPS, T, &v))//data_type& new_values)
		{
			BOOST_PP_REPEAT(MAX_SerialisationHelper_PROPS, FSN_DSER_REP_FORCE, ~)
		}

		void readChanges(RakNet::BitStream& stream, bool force_all, std::bitset<MAX_SerialisationHelper_PROPS>& changes, BOOST_PP_ENUM_BINARY_PARAMS(MAX_SerialisationHelper_PROPS, T, &v))//data_type& new_values)
		{
			BOOST_PP_REPEAT(MAX_SerialisationHelper_PROPS, FSN_DSER_REP, ~)
		}

		static void copyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
		{
			BOOST_PP_REPEAT(MAX_SerialisationHelper_PROPS, FSN_CPYSER_REP, ~)
		}
	};
}

#define BOOST_PP_ITERATION_LIMITS (2, MAX_SerialisationHelper_PROPS - 1)
#define BOOST_PP_FILENAME_1 "FusionSerialisationHelper.h"
#include BOOST_PP_ITERATE()

#undef FSN_SER_REP
#undef FSN_DSER_REP
#undef FSN_DSER_REP_CHANGE
#undef FSN_DSER_REP_FORCE
#undef FSN_CPYSER_REP

#undef REF_PARAM
#undef DATA_TYPE
#undef FSN_PP_PRINT

#endif // H_FusionSerialisationHelper

#else
#define n BOOST_PP_ITERATION()

namespace FusionEngine
{

	template <BOOST_PP_ENUM_PARAMS(n, typename T)>
	struct SerialisationHelper<
		BOOST_PP_ENUM_PARAMS(n,T)
		BOOST_PP_COMMA_IF(n)
		BOOST_PP_ENUM(BOOST_PP_SUB(MAX_SerialisationHelper_PROPS,n), FSN_PP_PRINT, sh_none)
	>
	{
		typedef std::tuple<BOOST_PP_ENUM_PARAMS(BOOST_PP_MIN(n, 10), T)> data_type;
		typedef boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)> types;

		static const size_t NumParams = n;

		std::bitset<n> m_Changed;

		SerialisationHelper()
		{}

		inline void unmark()
		{
			m_Changed.reset();
		}

		inline void markChanged(size_t i)
		{
			m_Changed.set(i);
		}

		bool writeChanges(bool force_all, RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(n, const T, &v))
		{
			if (!force_all && !m_Changed.any())
				return false;
			BOOST_PP_REPEAT(n, FSN_SER_REP, ~)
			m_Changed.reset();
			return true;
		}

		void readAll(RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(n, T, &v))
		{
			BOOST_PP_REPEAT(n, FSN_DSER_REP_FORCE, ~)
		}

		void readChanges(RakNet::BitStream& stream, bool force_all, std::bitset<n>& changes, BOOST_PP_ENUM_BINARY_PARAMS(n, T, &v))
		{
			BOOST_PP_REPEAT(n, FSN_DSER_REP, ~)
		}

//#if (n < 10)
//		void readChanges(RakNet::BitStream& stream, bool force_all, std::bitset<n>& changes, data_type& values)
//		{
//			BOOST_PP_REPEAT(n, FSN_DSER_REP, ~)
//		}
//#endif
//#define PARAM_CALLBACK_FN(z, n, data) void BOOST_PP_LPAREN() BOOST_PP_CAT(C::*f, n) BOOST_PP_RPAREN() BOOST_PP_LPAREN() BOOST_PP_CAT(T, n) BOOST_PP_RPAREN()
//
//		template <class C>
//		void readChanges(RakNet::BitStream& stream, bool force_all, std::bitset<n>& changes, C* obj, BOOST_PP_ENUM(n, PARAM_CALLBACK_FN, _))
//		{
//			FSN_ASSERT(obj);
//			BOOST_PP_REPEAT(n, FSN_DSER_CALLBACKS_REP, ~)
//		}
//
//#undef PARAM_CALLBACK_FN

		static void copyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
		{
			BOOST_PP_REPEAT(n, FSN_CPYSER_REP, ~)
		}
	};
	
}

#undef COMMA_COND
#undef n

#endif
