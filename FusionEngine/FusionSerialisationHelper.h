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
		static inline bool writeChange(bool force_all, RakNet::BitStream& stream, T& old_value, const T& new_value)
		{
			if (force_all || new_value != old_value)
			{
				stream.Write1();
				stream.Write(new_value);
				old_value = new_value;
				return true;
			}
			else
			{
				stream.Write0();
				return false;
			}
		}

		template <>
		static inline bool writeChange(bool force_all, RakNet::BitStream& stream, bool& old_value, const bool& new_value)
		{
			if (force_all || new_value != old_value)
			{
				stream.Write(new_value);
				old_value = new_value;
				return true;
			}
			else
				return false;
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
				writeChange(true, result, value);
			else if (readChange(false, current_data, value))
				writeChange(true, result, value);
		}
	};

}

#define FSN_SER_REP(z, n, text) dataWritten |= SerialisationUtils::writeChange<BOOST_PP_CAT(T,n)>(force_all, stream, get<n>(m_SerialisedValues), get<n>(new_values));
//#define FSN_DSER_REP(z, n, text) changeRead.set(n, SerialisationUtils::readChange(force_all, stream, get<n>(new_values)));
#define FSN_DSER_REP(z, n, text) SerialisationUtils::readChange<BOOST_PP_CAT(T,n)>(force_all, stream, get<n>(new_values));
#define FSN_CPYSER_REP(z, n, text) SerialisationUtils::copyChange<BOOST_PP_CAT(T,n)>(result, current_data, new_data);

// Allows enumeration of T0&, ... TN& type params
#define REF_PARAM(z, n, data) T ## n &

#define VAL_PROP(z, n, data) T ## n data ## n

#define FSN_PP_PRINT(z, n, data) data

#define BOOST_PP_ITERATION_LIMITS (2, 15)
#define BOOST_PP_FILENAME_1 "FusionSerialisationHelper.h"
#include BOOST_PP_ITERATE()

#undef FSN_SER_REP
#undef FSN_DSER_REP
#undef FSN_CPYSER_REP

#undef REF_PARAM
#undef VAL_PROP
#undef FSN_PP_PRINT

#endif // H_FusionSerialisationHelper

#else
#define n BOOST_PP_ITERATION()

namespace FusionEngine
{

	template <BOOST_PP_ENUM_PARAMS(n, typename T)>
	struct SerialisationHelper
	{
		typedef std::tuple<BOOST_PP_ENUM_PARAMS(n, typename T)> data_type;
		typedef std::tuple<BOOST_PP_ENUM(n, REF_PARAM, ~)> reference_type;

		//BOOST_PP_REPEAT(n, VAL_PROP, m_V)
		data_type m_SerialisedValues;

		SerialisationHelper()
		{}

		SerialisationHelper(const data_type& initial_values)
			: m_SerialisedValues(initial_values)
		{}

		inline bool writeChanges(bool force_all, RakNet::BitStream& stream, const data_type& new_values)
		{
			bool dataWritten = false;
			BOOST_PP_REPEAT(n, FSN_SER_REP, ~)
			return dataWritten;
		}

		inline /*std::bitset<n>*/void readChanges(data_type& new_values)
		{
			//std::bitset<n> changeRead;
			BOOST_PP_REPEAT(n, FSN_DSER_REP, ~)
			//return changeRead;
		}

		void copyChanges(RakNet::BitStream& result, RakNet::BitStream& current_data, RakNet::BitStream& new_data)
		{
			BOOST_PP_REPEAT(n, FSN_CPYSER_REP, ~)
		}
	};
	
}

#undef n

#endif
