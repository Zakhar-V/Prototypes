#pragma once

#include "Object.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class Resource;
	typedef Ptr<Resource> ResourcePtr;

	enum ResourceState
	{
		RS_Unloaded,
		RS_Loading,
		RS_Loaded,
	};

	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	class ResourceQueueEntry
	{
		ResourceQueueEntry* prev;
		ResourceQueueEntry* next;
	};

	class Resource : public Object
	{
	public:

		Mutex mutex;

	protected:

		ResourceState m_state;

		bool m_loaded;
		bool m_valid;

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class ResourceManager : public Object
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}


