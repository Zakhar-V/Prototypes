#include "Resource.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	SpinLock Resource::s_queueLock;
	List<ResourcePtr> Resource::s_queue;
	Condition Resource::s_queueEvent;

	//----------------------------------------------------------------------------//
	Resource::Resource(void) :
		m_rState(RS_Unloaded),
		m_rInQueue(false),
		m_id(0)
	{

	}
	//----------------------------------------------------------------------------//
	Resource::~Resource(void)
	{
	}
	//----------------------------------------------------------------------------//
	void Resource::SetSourceFile(const String& _filename)
	{
		SCOPE_LOCK(*this);
		m_sourceFile = _filename;
	}
	//----------------------------------------------------------------------------//
	void Resource::Load(bool _async)
	{
		if (_async)
		{
			if (m_rState.CompareExchange(RS_Unloaded, RS_Loading)) // queue
			{
				SCOPE_LOCK(s_queueLock);
				if (m_rInQueue.CompareExchange(false, true))
				{
					s_queue.push_front(this);
					s_queueEvent.Signal();
				}
			}
		}
		else
		{
			if (m_rState.CompareExchange(RS_Unloaded, RS_Loading) || m_rState == RS_Loading)
			{
				SCOPE_LOCK(*this); // only one thread can load this resource once
				if (m_rState != RS_Loaded)
				{
					if (m_sourceFile.IsEmpty())
						m_sourceFile = gResourceCache->SearchFile(ClassId(), m_name);

					DataStream& _src = gFileSystem->Open(m_sourceFile);
					if (_src)
					{
						m_failedLoad = !_Load(_src);
					}
					else
					{
						LOG_MSG(LL_Error, "%s '%s' is not found", *ClassName(), *m_name);
						m_failedLoad = true;
					}
					m_rState = RS_Loaded;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	void Resource::Unload(void)
	{
		SCOPE_LOCK(*this);
		if (m_rState == RS_Loaded && _Unload())
		{
			m_rState = RS_Unloaded;
		}
	}
	//----------------------------------------------------------------------------//
	ResourcePtr Resource::_Unqueue(void)
	{
		ResourcePtr _r;
		SCOPE_LOCK(s_queueLock);
		if (!s_queue.empty())
		{
			_r = s_queue.back();
			s_queue.pop_back();
			_r->m_rInQueue = false;
		}
		return _r;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ResourceType
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	ResourceType::ResourceType(const String& _name, ResourceFactoryPfn _factory) :
		m_name(_name),
		m_typeid(_name.Hashi()),
		m_factory(_factory)
	{
		ASSERT(_name.NonEmpty());
		ASSERT(_factory != nullptr);
	}
	//----------------------------------------------------------------------------//
	ResourceType::~ResourceType(void)
	{
	}
	//----------------------------------------------------------------------------//
	void ResourceType::AddPaths(const String& _paths)
	{
		StringArray _splitted = _paths.Split(";\n\r");
		for (size_t i = 0; i < _splitted.size(); ++i)
		{
			bool _found = false;
			String _path = MakeFullPath(_splitted[i].Trim(" \t"));
			for (size_t j = 0; j < m_dirs.size(); ++j)
			{
				if (_path.Equals(m_dirs[j], FS_IGNORE_CASE))
					_found = true;
			}
			if (!_found)
				m_dirs.push_back(_path);
		}
	}
	//----------------------------------------------------------------------------//
	void ResourceType::AddFileExts(const String& _exts)
	{
		StringArray _splitted = _exts.Split(" \t\n\r,;");
		for (size_t i = 0; i < _splitted.size(); ++i)
		{
			bool _found = false;
			const String& _ext = _splitted[i];
			for (size_t j = 0; j < m_exts.size(); ++j)
			{
				if (_ext.Equals(m_exts[j], FS_IGNORE_CASE))
					_found = true;
			}
			if (!_found)
				m_exts.push_back(_ext);
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ResourceCache::StartupParams
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	void ResourceCache::StartupParams::Serialize(Config& _cfg, bool _loading)
	{
		if (_loading)
		{
			Config* _val = _cfg.Search("rc_numBackgroundThreads");
			if (_val)
				numThreads = *_val;
		}
		else
		{
			_cfg("rc_numBackgroundThreads") = numThreads;
		}
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::StartupParams::SetDefaults(void)
	{
		numThreads = MAX_THREADS;
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::StartupParams::Validate(void)
	{
		if (numThreads == 0 || numThreads > MAX_THREADS)
			numThreads = MAX_THREADS;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// ResourceCache
	//----------------------------------------------------------------------------//

	int ResourceCache::s_moduleRefCount = 0;
	ResourceCache::StartupParams ResourceCache::s_startupParams;

	//----------------------------------------------------------------------------//
	ResourceCache::StartupParams& ResourceCache::GetCurrentStartupParams(void)
	{
		if (s_instance)
		{
			s_startupParams.numThreads = s_instance->m_numThreads;
		}
		return s_startupParams;
	}
	//----------------------------------------------------------------------------//
	ResourceCache::ResourceCache(void) :
		m_numThreads(0),
		m_numStartedThreads(0)
	{
		ASSERT(Thread::IsMain());

		s_startupParams.Validate();

		while (m_numThreads < s_startupParams.numThreads)
		{
			m_runThreads[m_numThreads] = true;
			m_threads[m_numThreads] = Thread(this, &ResourceCache::_LoadingThread, m_numThreads);
			++m_numThreads;
		}

		while (m_numStartedThreads < m_numThreads); // wait starting all threads
	}
	//----------------------------------------------------------------------------//
	ResourceCache::~ResourceCache(void)
	{
		ASSERT(Thread::IsMain());

		// unqueue all resources
		while (Resource::_Unqueue());
									  
		// cleanup
		{
			SCOPE_LOCK(m_cacheLock);
			m_cache.clear(); 
		}

		// stop threads
		while (m_numThreads > 0) 
		{
			--m_numThreads;
			m_runThreads[m_numThreads] = false;
			m_threads[m_numThreads].Wait();
		}

		{
			SCOPE_LOCK(Resource::s_queueLock);
			if (Resource::s_queue.size() > 0)
				LOG_MSG(LL_Warning, "Resources load queue is not empty (size = %u)", (uint)Resource::s_queue.size());
		}
		{
			SCOPE_LOCK(m_cacheLock);
			if (m_cache.size() > 0)
				LOG_MSG(LL_Warning, "Resources cache is not empty (size = %u)", (uint)m_cache.size());
		}
	}
	//----------------------------------------------------------------------------//
	bool ResourceCache::RegisterType(const String& _name, ResourceFactoryPfn _factory, const String& _paths, const String& _exts)
	{
		uint _typeid = _name.Hashi();

		if (GetType(_typeid)) // already registered
			return false;
		if (_name.IsEmpty()) // empty name
			return false;
		if (!_factory) // no factory
			return false;

		ResourceType* _type = new ResourceType(_name, _factory);
		_type->AddPaths(_paths);
		_type->AddFileExts(_exts);
		m_types[_typeid] = _type;

		LOG_MSG(LL_Event, "New resource type '%s'", *_name);

		return true;
	}
	//----------------------------------------------------------------------------//
	ResourceType* ResourceCache::GetType(const NameHash& _name)
	{
		auto _it = m_types.find(_name);
		return _it != m_types.end() ? _it->second : nullptr;
	}
	//----------------------------------------------------------------------------//
	String ResourceCache::SearchFile(const NameHash& _type, const String& _name)
	{
		ResourceType* _rt = GetType(_type);
		if (_rt)
			return gFileSystem->SearchFile(_name, _rt->m_dirs, _rt->m_exts);
		return String::Empty;
	}
	//----------------------------------------------------------------------------//
	Resource* ResourceCache::GetResource(uint _id)
	{
		SCOPE_LOCK(m_cacheLock);
		auto _it = m_cache.find(_id);
		return _it != m_cache.end() ? _it->second : nullptr;
	}
	//----------------------------------------------------------------------------//
	Resource* ResourceCache::AddResource(const NameHash& _type, const String& _name)
	{
		ResourceType* _rt = GetType(_type);
		if (!_rt) // unknown type
			return nullptr;
		if (_name.IsEmpty()) // empty name
			return nullptr;

		uint _id = Resource::CreateId(_rt->m_typeid, _name);

		ResourcePtr _r;
		{
			SCOPE_LOCK(m_cacheLock);
			_r = GetResource(_id);
			if (_r)
				return _r;

			_r = _rt->m_factory();
			_r->m_name = _name;
			_r->m_id = _id;
			m_cache[_id] = _r;
		}

		{
			SCOPE_LOCK(_rt->m_lock);
			_rt->m_items.insert(_id);
		}

		return _r;
	}
	//----------------------------------------------------------------------------//
	Resource* ResourceCache::LoadResource(const NameHash& _type, const String& _name, bool _async)
	{
		ResourcePtr _r = AddResource(_type, _name);
		if (_r)
			_r->Load(_async);
		return _r;
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::UnqueueAllResources(void)
	{
		while (Resource::_Unqueue());
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::UnqueueAllResources(const NameHash& _type)
	{
		NOT_IMPLEMENTED_YET();
	}
	//----------------------------------------------------------------------------//
	void ResourceCache::_LoadingThread(uint _index)
	{
		Thread::SetName(Thread::GetCurrentId(), String::Format("Resource load thread (%u)", _index));

		m_numStartedThreads++;
		while (m_runThreads[_index])
		{
			ResourcePtr _r = Resource::_Unqueue();

			if (_r)
			{
				if (_r->GetRefCount() > 1) // it is not lost resource
					_r->Load(false); // load synchronously
			}
			else
				Resource::s_queueEvent.Wait(20); // wait next resource
		}
		m_numStartedThreads--;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
