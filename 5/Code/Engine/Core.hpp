#pragma once

#include "Base.hpp"
#include "Thread.hpp"
#include "Object.hpp"

namespace Engine
{
	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// Thread 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	class Message : public RefCounted
	{
	public:

	protected:
		Message* m_prev;
		Message* m_next;
	};

	class MessagePool
	{

	};

	class MessageQueue
	{
	public:

	protected:

		Message* m_first;
		Message* m_last;
	};

	struct Command
	{
		uint size;

	};

	class Queue
	{
	public:

	protected:


	};
	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// Thread 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////



	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Variant
	{
		enum Type
		{
			T_Null,
			T_Bool,
			T_Number,
			T_Array,
			T_Object,
		};

		union
		{

		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Object : public BaseObject
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Subsystem	: public Object
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
