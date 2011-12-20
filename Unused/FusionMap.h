#ifndef Header_FusionEngine_FusionMap
#define Header_FusionEngine_FusionMap

#if _MSC_VER > 1000
#pragma once
#endif

#include <map>

namespace FusionEngine
{

	template<class KEY_T, class VAL_T>
	class FusionMap
	{
	public:
		std::map<KEY_T, VAL_T>* GetMap() const;
		//! Checks if the given name exists in the map.
		bool contains(const std::string &name) const;

	private:
		std::map<KEY_T, VAL_T> m_Map;
	};

}

#endif