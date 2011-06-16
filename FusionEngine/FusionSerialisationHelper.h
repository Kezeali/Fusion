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

#include <BitStream.h>
#include <bitset>
#include <tuple>

namespace FusionEngine
{

	namespace SerialisationUtils
	{
		template <typename T>
		static inline void writeChange(bool all, RakNet::BitStream& stream, bool changed, const T& new_value)
		{
			if (all || changed)
			{
				if (!all)
					stream.Write1();
				stream.Write(new_value);
			}
			else if (!all)
			{
				stream.Write0();
			}
		}

		template <>
		static inline void writeChange(bool all, RakNet::BitStream& stream, bool changed, const bool& new_value)
		{
			stream.Write(new_value);
		}

		template <typename T>
		static inline bool readChange(bool all, RakNet::BitStream& stream, T& new_value)
		{
			if (all || stream.ReadBit())
			{
				stream.Read(new_value);
				return true;
			}
			else
				return false;
		}

		template <>
		static inline bool readChange(bool all, RakNet::BitStream& stream, bool& new_value)
		{
			stream.Read(new_value);
			return true;
		}

		template <typename T>
		static void copyChange(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
		{
			T value;
			if (readChange(false, new_data, value))
			{
				result.Write(value);
				current_data.IgnoreBytes(sizeof(T));
			}
			else if (readChange(true, current_data, value))
				result.Write(value);
		}
	};

	struct sh_none {};

}

//#define FSN_SER_REP(z, n, text) dataWritten |= force_all || m_Changed.get(n); SerialisationUtils::writeChange<BOOST_PP_CAT(T,n)>(stream, dataWritten, get<n>(new_values));
//#define FSN_DSER_REP(z, n, text) changes.set(n, SerialisationUtils::readChange(false, stream, get<n>(new_values)));
//#define FSN_DSER_REP_FORCE(z, n, text) SerialisationUtils::readChange<BOOST_PP_CAT(T,n)>(true, stream, get<n>(new_values));

#define FSN_SER_REP(z, n, text) dataWritten |= force_all || m_Changed.at(n); SerialisationUtils::writeChange(force_all, stream, m_Changed.at(n), v ## n);
#define FSN_DSER_REP(z, n, text) changes.set(n, SerialisationUtils::readChange(false, stream, v ## n) );
#define FSN_DSER_REP_FORCE(z, n, text) SerialisationUtils::readChange(true, stream, v ## n);

#define FSN_CPYSER_REP(z, n, text) SerialisationUtils::copyChange< T ## n >(result, current_data, new_data);

// Allows enumeration of T0&, ... TN& type params
#define REF_PARAM(z, n, data) T ## n &

#define VAL_PROP(z, n, data) T ## n data ## n

#define FSN_PP_PRINT(z, n, data) data

namespace FusionEngine
{
	template <BOOST_PP_ENUM_BINARY_PARAMS(15, typename T, = sh_none BOOST_PP_INTERCEPT)>
	struct SerialisationHelper
	{
		//typedef std::tuple<BOOST_PP_ENUM_PARAMS(n, typename T)> data_type;
		//typedef std::tuple<BOOST_PP_ENUM(n, REF_PARAM, ~)> reference_type;

		//BOOST_PP_REPEAT(n, VAL_PROP, m_V)
		//data_type m_SerialisedValues;

		std::bitset<15> m_Changed;

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

		bool writeChanges(bool force_all, RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(15, const T, &v))//const data_type& new_values)
		{
			bool dataWritten = false;
			BOOST_PP_REPEAT(15, FSN_SER_REP, ~)
			m_Changed.reset();
			return dataWritten;
		}

		void readAll(RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(15, T, &v))//data_type& new_values)
		{
			BOOST_PP_REPEAT(15, FSN_DSER_REP_FORCE, ~)
		}

		void readChanges(RakNet::BitStream& stream, std::bitset<15>& changes, BOOST_PP_ENUM_BINARY_PARAMS(15, T, &v))//data_type& new_values)
		{
			BOOST_PP_REPEAT(15, FSN_DSER_REP, ~)
		}

		static void copyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
		{
			BOOST_PP_REPEAT(15, FSN_CPYSER_REP, ~)
		}
	};
}

#define BOOST_PP_ITERATION_LIMITS (2, 14)
#define BOOST_PP_FILENAME_1 "FusionSerialisationHelper.h"
#include BOOST_PP_ITERATE()

#undef FSN_SER_REP
#undef FSN_DSER_REP
#undef FSN_DSER_REP_FORCE
#undef FSN_CPYSER_REP

#undef REF_PARAM
#undef VAL_PROP
#undef FSN_PP_PRINT

#endif // H_FusionSerialisationHelper

#else
#define n BOOST_PP_ITERATION()

#define COMMA_COND() n

namespace FusionEngine
{

	template <BOOST_PP_ENUM_PARAMS(n, typename T)>
	struct SerialisationHelper<
		BOOST_PP_ENUM_PARAMS(n,T)
		BOOST_PP_COMMA_IF(n)
		BOOST_PP_ENUM(BOOST_PP_SUB(15,n), FSN_PP_PRINT, sh_none)
	>
	{
		//typedef std::tuple<BOOST_PP_ENUM_PARAMS(n, typename T)> data_type;
		//typedef std::tuple<BOOST_PP_ENUM(n, REF_PARAM, ~)> reference_type;

		//BOOST_PP_REPEAT(n, VAL_PROP, m_V)
		//data_type m_SerialisedValues;

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

		bool writeChanges(bool force_all, RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(n, const T, &v))//const data_type& new_values)
		{
			bool dataWritten = false;
			BOOST_PP_REPEAT(n, FSN_SER_REP, ~)
			m_Changed.reset();
			return dataWritten;
		}

		void readAll(RakNet::BitStream& stream, BOOST_PP_ENUM_BINARY_PARAMS(n, T, &v))//data_type& new_values)
		{
			BOOST_PP_REPEAT(n, FSN_DSER_REP_FORCE, ~)
		}

		void readChanges(RakNet::BitStream& stream, std::bitset<n>& changes, BOOST_PP_ENUM_BINARY_PARAMS(n, T, &v))//data_type& new_values)
		{
			BOOST_PP_REPEAT(n, FSN_DSER_REP, ~)
		}

		static void copyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
		{
			BOOST_PP_REPEAT(n, FSN_CPYSER_REP, ~)
		}
	};
	
}

#undef COMMA_COND
#undef n

#endif
