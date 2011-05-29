/*
*  Copyright (c) 2009-2011 Fusion Project Team
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

#ifndef H_FusionHashable
#define H_FusionHashable

#if _MSC_VER > 1000
#pragma once
#endif

#include <boost/functional/hash.hpp>

namespace FusionEngine
{
	//! Asoc. container key class incl. integer and string key
	/*!
	 * Structure compatible with std#hash for use as a key in
	 * unordered_map, etc.
	 */
	struct PlayerKey
	{
		int player;
		std::string key;

		PlayerKey(int player, const std::string &key)
			: player(player),
			key(key)
		{
		}
	};

	static bool operator== (PlayerKey const& l, PlayerKey const& r)
	{
		return l.player == r.player && l.key == r.key;
	}

	//! Hash function for PlayerKey
	/*std::size_t hash_value(PlayerKey const& k)
	{
		std::tr1::hash<std::string> hasher;
		return hasher(k.key + CL_StringHelp::int_to_local8(k.player).c_str());
	}*/

	//! Hashable / sortable key-class for input bindings in the input manager
	struct BindingKey
	{
		unsigned int device;
		unsigned int index;
		int code;

		BindingKey()
			: device(0),
			index(0),
			code(0)
		{
		}

		BindingKey(unsigned int device_type, unsigned int index, int code)
			: device(device_type),
			index(index),
			code(code)
		{
		}
	};

	// Needed for unordered_map
	static bool operator== (BindingKey const& l, BindingKey const& r)
	{
		return l.device == r.device && l.index == r.index && l.code == r.code;
	}

	// Lessthan sorting for std::map
	static bool operator< (BindingKey const& l, BindingKey const& r)
	{
		if (l.device < r.device)
			return true;
		if (r.device < l.device) // i.e. Only continue to compare 'index' if l.device and r.device are equal
			return false;

		if (l.index < r.index)
			return true;
		if (r.index < l.index)
			return false;

		if (l.code < r.code)
			return true;
		if (r.code < l.code)
			return false;

		// All elements are equal
		return false;
	}
}

namespace std {
	//namespace tr1 {
	template <>
	struct hash<FusionEngine::PlayerKey> : public unary_function<FusionEngine::PlayerKey, size_t>
	{
		size_t operator()(const FusionEngine::PlayerKey& key) const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, key.player);
			boost::hash_combine(seed, key.key);

			return seed;
		}
	};

	template <>
	struct hash<FusionEngine::BindingKey> : public unary_function<FusionEngine::BindingKey, size_t>
	{
		size_t operator()(const FusionEngine::BindingKey& key) const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, key.device);
			boost::hash_combine(seed, key.index);
			boost::hash_combine(seed, key.code);

			return seed;
		}
	};

	template <typename T, typename U>
	struct hash<pair<T, U>> : public unary_function<pair<T, U>, size_t>
	{
		size_t operator()(const pair<T, U>& key) const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, key.first);
			boost::hash_combine(seed, key.second);

			return seed;
		}
	};
//}
}

#endif