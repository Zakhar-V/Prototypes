#pragma once

#include "Base.hpp"
#include "File.hpp"
#include "Thread.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gResourceCache Engine::ResourceCache::Get()

	typedef Ptr<class Resource> ResourcePtr;
	typedef ResourcePtr(*ResourceFactoryPfn)(void);

	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	enum ResourceState : uint8
	{
		RS_Unloaded,
		RS_Loading,
		RS_Loaded,
	};

	class Resource : public RCBase, public CriticalSection
	{
	public:


		Resource(void);
		~Resource(void);

		void SetSourceFile(const String& _filename);
		const String& GetSourceFile(void) { return m_sourceFile; }
		ResourceState GetState(void) { return m_rState; }
		bool IsLoaded(void) { return m_rState == RS_Loaded; }
		bool IsLoadFail(void) { return m_failedLoad; }
		void Touch(bool _wait = true) { Load(!_wait); }
		void Load(bool _async = false);
		void Unload(void);

		static uint CreateId(const NameHash& _type, const String& _name) { return (_type + "@") + _name; }

	protected:
		friend class ResourceCache;

		virtual bool _Unload(void) { return false; }
		virtual bool _Load(DataStream& _src) { return true; }

		static ResourcePtr _Unqueue(void);

		Atomic<ResourceState> m_rState;
		Atomic<bool> m_rInQueue;
		bool m_failedLoad;
		String m_sourceFile;
		String m_name;
		uint m_id;

		static SpinLock s_queueLock;
		static List<ResourcePtr> s_queue;
		static Condition s_queueEvent;
	};

	//----------------------------------------------------------------------------//
	// ResourceType
	//----------------------------------------------------------------------------//

	class ResourceType : public NonCopyable
	{
	public:
		ResourceType(const String& _name, ResourceFactoryPfn _factory);
		~ResourceType(void);
		
		void AddPaths(const String& _paths);
		void AddFileExts(const String& _exts);

	protected:
		friend class ResourceCache;
		String m_name;
		uint m_typeid;
		ResourceFactoryPfn m_factory;
		StringArray m_exts;
		StringArray m_dirs;
		CriticalSection m_lock;
		HashSet<uint> m_items;
		HashMap<uint, uint> m_files;
	};

	//----------------------------------------------------------------------------//
	// ResourceCache
	//----------------------------------------------------------------------------//

	class ResourceCache : public Singleton<ResourceCache>
	{
	public:

		enum : uint
		{
			MAX_THREADS = 4,
		};

		struct StartupParams
		{
			uint numThreads = MAX_THREADS; // rc_numBackgroundThreads

			void Serialize(Config& _cfg, bool _loading);
			void SetDefaults(void);
			void Validate(void);
		};
		static StartupParams& GetStartupParams(void) { return s_startupParams; }
		static StartupParams& GetCurrentStartupParams(void);

		// []

		bool RegisterType(const String& _name, ResourceFactoryPfn _factory, const String& _paths = String::Empty, const String& _exts = String::Empty);
		ResourceType* GetType(const NameHash& _name);

		String SearchFile(const NameHash& _type, const String& _name);
		Resource* GetResource(uint _id);
		Resource* GetResource(const NameHash& _type, const String& _name) { return GetResource(Resource::CreateId(_type,_name)); }
		Resource* AddResource(const NameHash& _type, const String& _name);
		Resource* LoadResource(const NameHash& _type, const String& _name, bool _async);

		//template <class T> T* Get(const String& _name) { return static_cast<T*>(GetResource(T::ClassIdStatic(), _name)); }
		template <class T> T* Add(const String& _name) { return static_cast<T*>(AddResource(T::ClassIdStatic(), _name)); }
		template <class T> T* Load(const String& _name, bool _async) { return static_cast<T*>(LoadResource(T::ClassIdStatic(), _name, _async)); }

		//TODO: resource lifetime management

		void UnqueueAllResources(void);
		void UnqueueAllResources(const NameHash& _type);

	protected:
		friend class System;

		ResourceCache(void);
		~ResourceCache(void);
		void _LoadingThread(uint _index);

		HashMap<uint, ResourceType*> m_types;
		CriticalSection m_cacheLock;
		HashMap<uint, ResourcePtr> m_cache;
		volatile bool m_runThreads[MAX_THREADS];
		Thread m_threads[MAX_THREADS];
		uint m_numThreads;
		Atomic<uint> m_numStartedThreads;

		static int s_moduleRefCount;
		static StartupParams s_startupParams;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
