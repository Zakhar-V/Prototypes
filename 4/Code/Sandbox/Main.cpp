#undef _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "Sandbox.hpp"
#include <SDL2\include\SDL.h>
#include <Windows.h>
#include <io.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#	include <direct.h>
#else
#	include <dirent.h>
#endif
#include <zlib\zlib.h>
#include <zlib\unzip.h>

using namespace Rx;
#define PRINT_SIZEOF(T) printf("sizeof(%s) = %d\n", #T, sizeof(T))

namespace Rx
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


	const char* _FileExt(const char* _fname, int _length = -1)
	{
		if (_length < 0)
			_length = (int)strlen(_fname);

		const char* _end = _fname + _length;
		while (_end-- > _fname)
		{
			if (*_end == '.')
				return _end + 1;
			if (*_end == '/' || *_end == '\\')
				break;
		}
		return "";
	}


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//



	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//




	enum ResourceState : uint
	{
		RS_Unloaded,
		RS_Queued,
		RS_Loading,
		RS_Loaded,
	};

	typedef SharedPtr<class Resource> ResourcePtr;

	class Resource : public RefCounted
	{
	public:
		//CLASSNAME(Resource);

		Resource(void);
		~Resource(void);


		bool IsPending(void) { return m_state == RS_Queued || m_state == RS_Loading; }
		bool IsLoaded(void) { return m_state == RS_Loaded; }
		bool IsUnloaded(void) { return m_state == RS_Unloaded; }

		void Load(bool _wait = true);

	protected:
		friend class ResourceManager;

		virtual void _Load(void) { }

		bool _ChangeState(ResourceState _expected, ResourceState _newState)
		{
			if (_expected != _newState)
			{

			}
			
			return m_state.CompareExchange(_expected, _newState);

			return false;
		}

		void _DeleteThis(void) override;

		ResourceManager* m_manager;
		uint m_id;
		String m_name;

		Atomic<ResourceState> m_state;

	private:
		friend class ResourceCache;

		Resource* m_qprev;
		Resource* m_qnext;
	};

	//----------------------------------------------------------------------------//
	// ResourceManager
	//----------------------------------------------------------------------------//

	class ResourceManager
	{
	public:


	protected:
		friend class Resource;

		virtual ResourcePtr _Factory(void) = 0;
		void _RemoveRefs(Resource* _r)
		{
			SCOPE_LOCK(m_mutex);
			m_resources.erase(_r->m_id);
		}

		Mutex m_mutex;
		HashMap<uint, ResourcePtr> m_cache;
		HashMap<uint, Resource*> m_resources;
		String m_className;
		NameHash m_classId;
	};

	//----------------------------------------------------------------------------//
	// ResourceCache
	//----------------------------------------------------------------------------//

#define gResourceCache Rx::ResourceCache::Get()

	class ResourceCache : public Singleton<ResourceCache>
	{
	public:

	protected:
		friend class Resource;

		void _Queue(Resource* _r)
		{
			SCOPE_LOCK(m_qlock);
			_r->AddRef();
			_r->m_qnext = m_qstart;
			if (m_qstart)
				m_qstart->m_qprev = _r;
			else
				m_qend = _r;
			m_qstart = _r;
		}

		void _Unqueue(Resource* _r)
		{
			SCOPE_LOCK(m_qlock);

			if (_r->m_qnext)
				_r->m_qnext->m_qprev = _r->m_qprev;
			else
				m_qend = _r->m_qprev;

			if (_r->m_qprev)
				_r->m_qprev->m_qnext = _r->m_qnext;
			else
				m_qstart = _r->m_qnext;

			_r->m_qprev = nullptr;
			_r->m_qnext = nullptr;
			_r->Release();
		}

		void _Wait(Resource* _r)
		{
			while (_r->IsPending())
			{
				m_resourceLoaded.Wait(1);
			}
		}

		Mutex m_qlock;
		Resource* m_qstart;
		Resource* m_qend;
		Condition m_resourceLoaded;

		HashMap<uint, ResourceManager*> m_managers;
	};

	//----------------------------------------------------------------------------//
	// 	Resource
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Resource::Resource(void) :
		m_manager(nullptr),
		m_id(0),
		m_state(RS_Unloaded),
		m_qprev(nullptr),
		m_qnext(nullptr)
	{
	}
	//----------------------------------------------------------------------------//
	Resource::~Resource(void)
	{

	}
	//----------------------------------------------------------------------------//
	void Resource::Load(bool _wait)
	{
		if (!_wait) // load async
		{
			if (m_state.CompareExchange(RS_Unloaded, RS_Queued)) // queue this resource
			{
				gResourceCache->_Queue(this);
			}
		}
		else if(m_state != RS_Loaded) // resource is not loaded
		{
			ResourcePtr _addref(this);

			if (m_state.CompareExchange(RS_Queued, RS_Loading))	// unqueue it and load now
			{
				gResourceCache->_Unqueue(this);
				_Load();
				m_state = RS_Loaded;
			}
			else if (m_state.CompareExchange(RS_Unloaded, RS_Loading)) // load now 
			{
				_Load();
				m_state = RS_Loaded;
			}
			else // wait loading
			{
				ASSERT(m_state != RS_Unloaded);
				gResourceCache->_Wait(this);
			}
		}
	}
	//----------------------------------------------------------------------------//
	void Resource::_DeleteThis(void)
	{
		if (m_manager)
			m_manager->_RemoveRefs(this);

		RefCounted::_DeleteThis();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//



	extern int numUpdates;


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	/*
	
	Actor _a, _b, _c
	
	*/

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}




struct ThreadArg
{
	ThreadArg(void) { }
	ThreadArg(const ThreadArg& _arg) : v(_arg.v) { printf("copy\n"); }
	ThreadArg(ThreadArg&& _arg) : v(_arg.v) { printf("move\n"); _arg.v = -1; }
	int v = 0;
};

void ThreadFunc()
{
	printf("func()\n");
}

struct ThreadClass
{
	void ThreadFunc()
	{
		printf("method()\n");
	}
};


//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//

typedef SharedPtr<class ThreadTask> ThreadTaskPtr;

class ThreadTask : public RefCounted
{
public:
	virtual void Exec(void) = 0;
};



class ThreadPool : public Singleton<ThreadPool>
{
public:

protected:

	List<ThreadTaskPtr> m_queue;
};


//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//

template <class T> struct StringBuffer
{
	int refs;
	int size;
	int length;
	T str[1];
};

struct TestString
{
public:

	StringBuffer<char>* GetBuffer(void)
	{
		return reinterpret_cast<StringBuffer<char>*>(m_buffer - offsetof(StringBuffer<char>, str));
	}

	void SetBuffer(StringBuffer<char>* _buffer)
	{
		m_buffer = reinterpret_cast<char*>(&_buffer->str);
	}
	char* m_buffer;
};

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//

#include <atomic>
#include <thread>

template<class T> class _Atomic : public std::atomic<T>
{
public:

	typedef std::atomic<T> Base;

	_Atomic(void) = default;
	_Atomic(T _val) : Base(_val) { }

	T Get(void) const { return this->load(); }
	bool CompareExchangeStrong(T _exp, T _val) { return this->compare_exchange_weak(_exp, _val); }
	bool CompareExchangeStrong(T* _exp, T _val)
	{
		return this->compare_exchange_weak(*_exp, _val);
	}
	bool CompareExchangeWeak(T _exp, T _val)
	{
		return this->compare_exchange_weak(_exp, _val);
	}
	bool CompareExchangeWeak(T* _exp, T _val)
	{
		return this->compare_exchange_weak(_exp, _val);
	}
};

struct _SpinLock
{
	_Atomic<int> m_lock = 0;

	bool TryLock(void)
	{
		int _exp = 0;
		return m_lock.compare_exchange_weak(_exp, 1, std::memory_order_acquire);
	}

	void Lock(void)
	{
		while (!TryLock());
	}

	void Unlock(void)
	{
		m_lock.store(0, std::memory_order_release);
	}

};


class TestRC
{
public:

	TestRC()
	{
		printf("new rc\n");
	}

	~TestRC()
	{
		printf("del rc\n");
	}

	void AddRef(void)
	{
		m_refs.fetch_add(1, std::memory_order_acquire);
	}

	void Release(void)
	{
		if (!m_refs.fetch_add(-1, std::memory_order_release))
		{
			delete this;
		}
	}

	std::atomic<int8> m_refs = 0;

	static void ThreadFunc(TestRC* _obj)
	{
		s_lock.Lock();
		_obj->AddRef();
		MemoryBarrier();
		_obj->Release();
		s_lock.Unlock();
	}

	static _SpinLock s_lock;
};

_SpinLock TestRC::s_lock;

template <MemoryOrder Order = MO_Default> int8 _AtomicGet(int8& _atom) { return ((std::atomic<int8>*)&_atom)->load(static_cast<std::memory_order>(Order)); }


int main(void)
{
	try
	{
		//PRINT_SIZEOF(RefCounted);
		//PRINT_SIZEOF(RefCounter);


		/*int8 _atom;

		_AtomicGet<MO_Acquire>(_atom);
		_AtomicGet<MO_Release>(_atom);

		TestRC* _rc = new TestRC;
		_rc->AddRef();

		std::thread _t1 = std::thread(&TestRC::ThreadFunc, _rc);
		std::thread _t2 = std::thread(&TestRC::ThreadFunc, _rc);
		_rc->Release();
		_t1.join();
		_t2.join();*/

		RefCounted* _rc = new RefCounted;
		_rc->AddRef();

		String _str = "abc";
		_str += "def";
		printf("%s %d\n", _str.SubStr(1).CStr(), _str.IsEmpty());


		system("pause");
		return 0;
	}
	catch (std::exception _e)
	{
		printf("unhandled exception: %s\n", _e.what());
		system("pause");
		return -1;
	}
	catch (...)
	{
		printf("unhandled exception");
		system("pause");
		return -1;
	}
}