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

using namespace Engine;
#define PRINT_SIZEOF(T) printf("sizeof(%s) = %d\n", #T, sizeof(T))

namespace Engine
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

	typedef Ptr<class Resource> ResourcePtr;

	class Resource : public RefCounted
	{
	public:
		CLASSNAME(Resource);

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

#define gResourceCache Engine::ResourceCache::Get()

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

typedef Ptr<class ThreadTask> ThreadTaskPtr;

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


//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//




int main(void)
{
	try
	{

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