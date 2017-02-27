#pragma once

#include "Common.hpp"
#include <array>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	template <class T> using Array = std::vector < T >;
	template <class T, uint C> using FixedArray = std::array < T, C >;
	template <class T> using List = std::list < T >;
	template <class K, class V> using Map = std::map < K, V >;
	template <class K, class V> using HashMap = std::unordered_map < K, V >;
	template <class T> using HashSet = std::unordered_set < T >;

	//----------------------------------------------------------------------------//
	// Utils
	//----------------------------------------------------------------------------//

	template <class T> T* ArrayPtr(Array<T>& _array) { return _array.empty() ? nullptr : &_array[0]; }
	template <class T> const T* ArrayPtr(const Array<T>& _array) { return _array.empty() ? nullptr : &_array[0]; }

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
