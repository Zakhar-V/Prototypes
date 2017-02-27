#pragma once

#include "RenderSystem.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct GLCommand
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//



	template <class T, uint SIZE> class GLCommandPool
	{
	public:

		static void* Alloc(void)
		{
			void* _p;
			s_mutex.Lock();
			if (s_top > s_pool)
			{
				_p = *s_top--;
				s_mutex.Unlock();
			}
			else
			{
				s_mutex.Unlock();
				_p = ::new uint8[sizeof(T)];
			}
			return _p;
		}

		static void Free(void* _p)
		{
			s_mutex.Lock();
			if (s_top < s_end)
			{
				*s_top++ = _p;
				s_mutex.Unlock();
			}
			else
			{
				s_mutex.Unlock();
				delete[]((uint8*)_p);
			}
		}

		void* operator new (size_t){ return Alloc(); }
		void operator delete(void* _p) { Free(_p); }

	protected:
		static CriticalSection s_mutex;
		static void* s_pool[SIZE];
		static void** s_top;
		static void** s_end;
	};

	template <class T, uint SIZE> CriticalSection GLCommandPool<T, SIZE>::s_mutex;
	template <class T, uint SIZE> void* GLCommandPool<T, SIZE>::s_pool[SIZE];
	template <class T, uint SIZE> void** GLCommandPool<T, SIZE>::s_top = &s_pool[0];
	template <class T, uint SIZE> void** GLCommandPool<T, SIZE>::s_end = (&s_pool[0]) + SIZE;



	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
