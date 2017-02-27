#include "Render.hpp"
#include "Log.hpp"
#include "File.hpp"
#include "Thread.hpp"
#include "Time.hpp"
#include "ShaderCompiler.hpp"
using namespace ge;

#include "PlatformIncludes.hpp"

namespace Sandbox
{

	/*
	//1. задача может одновременно выполняться только в одном потоке
	//2. задача может перемещаться между потоками
	*/

	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class ResourceCache;
	class Resource;
	typedef Ptr<Resource> ResourcePtr;

#define gResourceCache ResourceCache::Get()
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class Job : public RefCounted
	{
	public:

		void Exec(void)
		{
			_ExecImpl();
			m_done = true;
		}

		void Wait(void)
		{
			
		}

	protected:
		friend class JobManager;

		virtual void _ExecImpl(void) = 0;

		bool m_inQueue = false;
		volatile bool m_done = false;
		Job* m_prev = nullptr;
		Job* m_next = nullptr;
	};

	/*template <class... A> class TJob : public Job
	{
	public:
		typedef Closure<void, A...> Func;

		Job(Func _func) : m_func(_func) { }
 
	protected:
		void _ExecImpl(void) override { m_func(); }

		Func m_func;
	};	*/

	class JobManager : public Singleton<JobManager>
	{
	public:

	protected:
		friend class RenderSystem;

		/// Register render threads. For ge::RenderSystem.
		void _RegisterRenderThreads(void);
		/// Unregister render threads. For ge::RenderSystem.
		void _UnregisterRenderThreads(void);

		static void _Thread(void)
		{
			Job* _job = nullptr;
			{
				SCOPE_LOCK(s_mutex);
				//if(s_jobs)
			}

		}

		static CriticalSection s_mutex;
		static Job* s_jobs;
	};

	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	enum ResourceState
	{
		//RS_Initial,
		RS_Unloaded,
		RS_Loaded,
		RS_Invalid,
	};

	/*enum ResourceLoadFlags
	{
		RSF_Async = 0x1,
		RSF_
	};*/

	class ResourceLoadTask
	{
	public:

	protected:
		friend class ResourceCache;

		ResourcePtr m_resource;
		ResourceLoadTask* m_prev = nullptr;
		ResourceLoadTask* m_next = nullptr;
	};

	class Resource : public Object
	{
	public:

		Resource(void);
		~Resource(void);

		void SetCreatedManually(void)
		{

		}
		bool IsLoaded(void) { return m_state != RS_Unloaded; }
		bool IsUnloaded(void) { return m_state == RS_Unloaded; }
		bool IsValid(void) { return m_state == RS_Loaded; }
		bool IsQueued(void) { return m_loadTask != nullptr; }

		void _Wait(void)
		{
			if (m_loadTask)
			{
				while (m_state == RS_Unloaded)
				{
					Thread::Sleep(1);
				}
			}
		}
		bool Touch(bool _wait = true);
		bool Load(DataStream& _stream);
		bool Unload(void);

		virtual bool Save(DataStream& _stream) = 0;

		Mutex& GetMutex(void) { return m_mutex; }

	protected:
		friend class ResourceCache;

		virtual bool _LoadImpl(DataStream& _stream) = 0;
		virtual bool _UnloadImpl(void) = 0;
		//virtual bool _TouchImpl(void) = 0;

		//ResourceManager* m_manager;
		String m_sourceFile;
		Mutex m_mutex;
		volatile ResourceState m_state;
		time_t m_fileTime;
		volatile uint m_touchFrame;

	private:

		ResourceLoadTask* m_loadTask;
		volatile bool m_isLoading;
	};

	typedef ResourcePtr(*ResourceFactoryPfn)(void);

	//----------------------------------------------------------------------------//
	// ResourceCache
	//----------------------------------------------------------------------------//

	class ResourceCache : public Object, public Singleton<ResourceCache>
	{
	public:

		enum : uint
		{
			NUM_THREADS = 2,
		};

		ResourceCache(void);
		~ResourceCache(void);

		///	Start background threads. Should be called after creating of RenderSystem.
		bool Startup(void);
		/// Shutdown the background threads. Should be called before destroying of RenderSystem.
		void Shutdown(void);

		void RegisterType(const String& _name, ResourceFactoryPfn _factory, const String& _dirs, const String& _exts);

		Resource* AddResource(const StringHash& _typeId, const String& _name);

		bool Load(Resource* _resource, bool _wait = true);

		uint UnloadUnusedResources(StringHash _typeId, uint _framesLeft = 0);
		uint UnloadAllUnusedResources(uint _framesLeft = 0);
		uint RemoveUnusedResources(StringHash _typeId, uint _framesLeft = 0);
		uint RemoveAllUnusedResources(uint _framesLeft = 0);

	protected:

		void _Queue(Resource* _resource, bool _highPrio);
		void _Unqueue(ResourceLoadTask* _task);
		void _Load(Resource* _resource);

		void _BackgroundThread(void);
		static void _BackgroundThreadEntry(void);

		struct Type
		{
			String name;
			ResourceFactoryPfn Factory;
			StringArray dirs;
			StringArray exts;
			HashMap<uint32, ResourcePtr> cache;
			CriticalSection cacheMutex;
		};

		HashMap<uint32, Type> m_types;
		Mutex m_queueMutex;
		ResourceLoadTask* m_queueStart;
		ResourceLoadTask* m_queueEnd;
		volatile uint m_frame;
		volatile bool m_stopThreads;
		AtomicInt m_numThreads;
		AtomicInt m_errorThreads;
		Thread m_threads[NUM_THREADS];
	};

	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Resource::Resource(void) : 
		m_state(RS_Unloaded),
		m_fileTime(0),
		m_touchFrame(0),
		m_loadTask(nullptr),
		m_isLoading(false)
	{
	}
	//----------------------------------------------------------------------------//
	Resource::~Resource(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool Resource::Touch(bool _wait)
	{
		return gResourceCache->Load(this, _wait);
	}
	//----------------------------------------------------------------------------//
	bool Resource::Load(DataStream& _stream)
	{
		SCOPE_LOCK(m_mutex);
		m_isLoading = true;
		if (_stream)
		{
			m_state = RS_Loaded;
			m_sourceFile = _stream.GetName();
			m_fileTime = _stream.GetFileTime();
			if (!_LoadImpl(_stream))
				m_state = RS_Invalid;
		}
		else
		{
			m_state = RS_Invalid;
		}
		m_isLoading = false;
		return m_state == RS_Loaded;
	}
	//----------------------------------------------------------------------------//
	bool Resource::Unload(void)
	{
		SCOPE_LOCK(m_mutex);

		if (m_state == RS_Loaded && _UnloadImpl())
			m_state = RS_Unloaded;

		return m_state == RS_Unloaded;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ResourceCache
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ResourceCache::ResourceCache(void) :
		m_queueStart(nullptr),
		m_queueEnd(nullptr),
		m_frame(0),
		m_stopThreads(true),
		m_numThreads(0),
		m_errorThreads(0)
	{
	}
	//----------------------------------------------------------------------------//
	ResourceCache::~ResourceCache(void)
	{
		ASSERT(m_stopThreads == true);
	}
	//----------------------------------------------------------------------------//
	bool ResourceCache::Startup(void)
	{
		if (m_stopThreads)
		{
			LOG_NODE("ResourceCache::Startup");
			if (!gRenderSystem)
			{
				LOG_ERROR("Couldn't start resource cache: No render system");
				return false;
			}
			// start
			m_stopThreads = false;
			for (uint i = 0; i < NUM_THREADS; ++i)
				m_threads[i] = Thread(_BackgroundThreadEntry);

			// wait
			while (m_numThreads < NUM_THREADS)
				Thread::Sleep(1);

			LOG_INFO("Background threads: %d", m_numThreads);
		}
		return m_numThreads > m_errorThreads;
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::Shutdown(void)
	{
		if (!m_stopThreads)
		{
			// stop
			m_stopThreads = true;

			// wait
			for (uint i = 0; i < NUM_THREADS; ++i)
			{
				if (m_threads[i])
					m_threads[i].Wait();
			}
		}
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::RegisterType(const String& _name, ResourceFactoryPfn _factory, const String& _dirs, const String& _exts)
	{
		if (_name.IsEmpty() || !_factory)
			return;

		Type& _type = m_types[StringHash(_name)];
		_type.name = _name;
		_type.Factory = _factory;
		_dirs.Split(";", _type.dirs);
		for (size_t i = 0; i < _type.dirs.size(); ++i)
			_type.dirs[i] = MakeFullPath(_type.dirs[i]);
		_exts.Split(" \t,;", _type.exts);
	}
	//----------------------------------------------------------------------------//
	Resource* ResourceCache::AddResource(const StringHash& _typeId, const String& _name)
	{
		if (_name.IsEmpty())
			return nullptr;

		auto _typeIt = m_types.find(_typeId);
		if (_typeIt == m_types.end())
			return nullptr;
		Type& _type = _typeIt->second;
		String _nname = MakeFullPath(_name);
		StringHash _nameHash = _nname;
		ResourcePtr _resource;
		{
			SCOPE_LOCK(_type.cacheMutex);
			auto _exists = _type.cache.find(_nameHash);
			if (_exists != _type.cache.end())
			{
				_resource = _exists->second;
			}
			else
			{
				ResourcePtr _resource = _type.Factory();
				_resource->SetName(_nname);
				_type.cache[_nameHash] = _resource;
			}
		}
		return _resource;
	}
	//----------------------------------------------------------------------------//
	bool ResourceCache::Load(Resource* _resource, bool _wait)
	{
		if (!_resource)
			return false;

		if (_resource->IsUnloaded())
		{
			if (_wait) // load now
			{
				// unqueue
				{
					SCOPE_LOCK(m_queueMutex);
					if (_resource->m_loadTask)
					{
						_Unqueue(_resource->m_loadTask);
						delete _resource->m_loadTask;
						_resource->m_loadTask = nullptr;
					}
				}
				_Load(_resource);
			}
			else // load asynchronously
			{
				_Queue(_resource, true);
			}
		}

		_resource->m_touchFrame = m_frame; // touch
		return _resource->IsValid();
	}
	//----------------------------------------------------------------------------//
	uint ResourceCache::UnloadUnusedResources(StringHash _typeId, uint _framesLeft)
	{
		uint _unloaded = 0;
		uint _frame = m_frame;
		auto _typeIt = m_types.find(_typeId);
		if (_typeIt != m_types.end())
		{
			Type& _type = _typeIt->second;
			SCOPE_LOCK(_type.cacheMutex);
			for (auto i = _type.cache.begin(), e = _type.cache.end(); i != e; ++i)
			{
				if (i->second->GetRefCount() == 1 && (_frame - i->second->m_touchFrame) < _framesLeft && i->second->Unload())
					++_unloaded;
			}
		}
		return _unloaded;
	}
	//----------------------------------------------------------------------------//
	uint ResourceCache::UnloadAllUnusedResources(uint _framesLeft)
	{
		uint _unloaded = 0, _unloadedTotal = 0;
		do
		{
			_unloaded = 0;
			for (auto i = m_types.begin(), e = m_types.end(); i != e; ++i)
				_unloaded += UnloadAllUnusedResources(_framesLeft);
			_unloadedTotal += _unloaded;

		} while (_unloaded != 0);

		return _unloadedTotal;
	}
	//----------------------------------------------------------------------------//
	uint ResourceCache::RemoveUnusedResources(StringHash _typeId, uint _framesLeft)
	{
		uint _removed = 0;
		uint _frame = m_frame;
		auto _typeIt = m_types.find(_typeId);
		if (_typeIt != m_types.end())
		{
			Type& _type = _typeIt->second;
			SCOPE_LOCK(_type.cacheMutex);
			for (auto i = _type.cache.begin(); i != _type.cache.end();)
			{
				if (i->second->GetRefCount() == 1 && (_frame - i->second->m_touchFrame) < _framesLeft)
				{
					i = _type.cache.erase(i);
					++_removed;
				}
				else
					++i;
			}
		}
		return _removed;
	}
	//----------------------------------------------------------------------------//
	uint ResourceCache::RemoveAllUnusedResources(uint _framesLeft)
	{
		uint _unloaded = 0, _unloadedTotal = 0;
		do
		{
			_unloaded = 0;
			for (auto i = m_types.begin(), e = m_types.end(); i != e; ++i)
				_unloaded += RemoveUnusedResources(_framesLeft);
			_unloadedTotal += _unloaded;

		} while (_unloaded != 0);

		return _unloadedTotal;
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Queue(Resource* _resource, bool _highPrio)
	{
		SCOPE_LOCK(m_queueMutex);

		ResourceLoadTask* _task = _resource->m_loadTask;

		if (_task) // unqueue
		{
			_Unqueue(_task);
		}
		else // new task
		{
			_task = new ResourceLoadTask;
			_task->m_resource = _resource;
			_resource->m_loadTask = _task;
		}

		// inserting in queue
		if (_highPrio)
		{
			_task->m_prev = m_queueEnd;
			if (m_queueEnd)
				m_queueEnd->m_next = _task;
			else
				m_queueStart = _task;
			m_queueEnd = _task;
		}
		else
		{
			_task->m_next = m_queueStart;
			if (m_queueStart)
				m_queueStart->m_prev = _task;
			else
				m_queueEnd = _task;
			m_queueStart = _task;
		}
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Unqueue(ResourceLoadTask* _task)
	{
		SCOPE_LOCK(m_queueMutex);

		if (_task->m_next)
			_task->m_next->m_prev = _task->m_prev;
		else
			m_queueEnd = _task->m_prev;

		if (_task->m_prev)
			_task->m_prev->m_next = _task->m_next;
		else
			m_queueStart = _task->m_next;

		_task->m_prev = nullptr;
		_task->m_next = nullptr;
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_Load(Resource* _resource)
	{
		if (_resource->m_isLoading) // wait
		{
			while (_resource->m_state == RS_Unloaded)
				Thread::Sleep(1);
		}
		else // load
		{
			SCOPE_LOCK(_resource->GetMutex());

			if (_resource->IsUnloaded())
			{
				Type& _type = m_types[_resource->ClassId()];
				String _path = gFileSystem->SearchFile(_resource->GetName(), _type.dirs, _type.exts);
				DataStream _stream = gFileSystem->Open(_path);
				_resource->Load(_stream);
			}
		}
		_resource->m_touchFrame = m_frame; // touch
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_BackgroundThread(void)
	{
		++m_numThreads;
		LOG_INFO("Begin background thread %04x", Thread::GetCurrentId());
		
		if(!gRenderSystem->RegisterThread())
		{
			++m_errorThreads;
			while (!m_stopThreads)
			{
				Thread::Sleep(1);
			}
		}

		while (!m_stopThreads)
		{
			ResourcePtr _resource;
			{
				SCOPE_LOCK(m_queueMutex);

				if (m_queueEnd)
				{
					ResourceLoadTask* _task = m_queueEnd;
					if (m_queueEnd->m_prev)
						m_queueEnd->m_prev->m_next = nullptr;
					else
						m_queueStart = nullptr;

					_resource = _task->m_resource;
					delete _task;
					_resource->m_loadTask = nullptr;
				}
			}

			if (_resource)
				_Load(_resource);
			else
				Thread::Sleep(1);
		}

		LOG_INFO("End background thread %04x", Thread::GetCurrentId());
		--m_numThreads;
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_BackgroundThreadEntry(void)
	{
		s_instance->_BackgroundThread();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class TestFunctor
	{
	public:

		int Func1(int _a)
		{
			printf("%s(%d)\n", __FUNCTION__, _a);
			return ++_a;
		}

		int Func2(int _a, int _b) const
		{
			printf("%s(%d, %d)\n", __FUNCTION__, _a, _b);
			return _a + _b;
		}

		static void Func3(void)
		{
			printf("%s()\n", __FUNCTION__);
		}
	};

	void _Closure(int _x, int& _y)
	{
		printf("%s(%d, %d)\n", __FUNCTION__, _x, _y);
		--_x;
		++_y;
	}

	void _TestClosure(void)
	{
		int _y = 11;
		auto _closure = MakeClosure(_Closure, 7, _y);
		_closure();
		_closure();
		TestFunctor _test;
		auto _closure2 = MakeClosure(&_test, &TestFunctor::Func2, 7, _y);
		auto _method = MakeFunction(&_test, &TestFunctor::Func1);
		_method(123);
		auto _cmethod = MakeFunction(&_test, &TestFunctor::Func2);
		_cmethod(777, 3);
		auto _cfunc = MakeFunction(&TestFunctor::Func3);
		_cfunc();
	}
}

double GetTime(void)
{
	LARGE_INTEGER _c, _f;
	QueryPerformanceFrequency(&_f);
	QueryPerformanceCounter(&_c);
	return ((double)(_c.QuadPart) * 1000.0) / (double)(_f.QuadPart);
}

void _TestBatches(void)
{

	RenderSystemPtr _rs = RenderSystem::Create(RST_GL, SM4, RSF_DebugOutput | RSF_GL_CoreProfile);
	if (_rs)
	{

		VertexFormatDesc _vfd; // 20 bytes
		_vfd.Attrib(VA_Position, VAF_Float3, 0, 0);
		_vfd.Attrib(VA_Color, VAF_UByte4N, 12, 0);
		_vfd.Attrib(VA_TexCoord0, VAF_Half2, 16, 0);

		VertexFormatDesc _vfd2; // 20 bytes
		_vfd2.Attrib(VA_Position, VAF_Float3, 0, 0);
		//_vfd.Attrib(VA_Color, VAF_UByte4N, 12, 0);
		_vfd2.Attrib(VA_TexCoord0, VAF_Half2, 16, 0);

		gRenderSystem->DebugPoint("init resources");
		VertexFormat* _vf = gRenderSystem->CreateVertexFormat(_vfd);
		VertexFormat* _vf2 = gRenderSystem->CreateVertexFormat(_vfd2);
		HardwareBufferPtr _buffer = gRenderSystem->CreateBuffer(HBU_Dynamic, AM_Write, 80);
		uint8* _vd = _buffer->Map(MM_WriteDiscard);
		memset(_vd, 0, _buffer->GetSize());
		_buffer->Unmap();
		VertexArrayPtr _vertexArray = gRenderSystem->CreateVertexArray(_vf);
		_vertexArray->SetBuffer(0, _buffer);
		VertexArrayPtr _vertexArray2 = gRenderSystem->CreateVertexArray(_vf2);
		_vertexArray2->SetBuffer(0, _buffer);

		gRenderSystem->DebugPoint("main loop");

		double _time = 0.0;
		uint _batches = 0;
		uint _frames = 0;
		while (true)
		{
			double _st = GetTime();
			gRenderSystem->BeginFrame();
			for (uint i = 0; i < 5000; ++i)
			{
				++_batches;
				gRenderSystem->SetGeometry(_vertexArray, nullptr);
				gRenderSystem->Draw(PT_TriangleStrip, 0, 4);
				gRenderSystem->SetGeometry(_vertexArray2, nullptr);
				gRenderSystem->Draw(PT_Triangles, 0, 3);

			}
			gRenderSystem->EndFrame();
			double _dt = GetTime() - _st;
			_time += _dt;
			++_frames;
			if (_time > 1000.0)
			{
				printf("%.2f fps (%.3f ms/frame), %u batches/s (%.3f mcs/batch) (%.3f batches/ms)\n", (_frames / _time) * 1000, _time / _frames, _batches, ((_time * 1000) / _batches), _batches / _time);
				_time = 0;
				_batches = 0;
				_frames = 0;
				gRenderSystem->DebugPoint("frame");
				Thread::Sleep(50);
			}
		}
	}
}

int main(void)
{
	//Sandbox::_TestClosure();

	/*TimerPtr _timer = Timer::Create();
	while (true)
	{
		printf("%f %f %f\n", gTimer->Seconds(), gTimer->Milliseconds(), gTimer->Microseconds());
	}*/

#define M(x) F(x)

#define F(x) printf("MF(%d)\n", x)
	M(1);
#define F(x) printf("mf(%d)\n", x)
	M(33);

	system("pause");
	return 0;



	printf("sizeof(long) = %zd\n", sizeof(long));
	printf("sizeof(size_t) = %zd\n", sizeof(size_t));
	//_TestBatches();
	RenderSystemPtr _rs = RenderSystem::Create(RST_GL, SM4, RSF_DebugOutput | RSF_GL_CoreProfile);
	WindowSystemPtr _ws = WindowSystem::Create();
	if (_rs && _ws)
	{
		WindowPtr _wnd = gWindowSystem->CreateWindow();
		if (_wnd)
		{
			_wnd->SetVisible();
			while (_wnd->IsOpened())
			{
				gWindowSystem->PollEvents();
			}
		}
	}

	system("pause");
	return 0;
}
