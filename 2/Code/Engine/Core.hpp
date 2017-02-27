#pragma once

#include "Lib.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Log
	//----------------------------------------------------------------------------//

#define LOG_MSG(level, msg, ...) Engine::LogMessage(level, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) Engine::LogMessage(Engine::LL_Info, msg, ##__VA_ARGS__)
#define LOG_EVENT(msg, ...)	Engine::LogMessage(Engine::LL_Event, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) Engine::LogMessage(Engine::LL_Warning, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)	Engine::LogMessage(Engine::LL_Error, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...)	Engine::LogMessage(Engine::LL_Debug, msg, ##__VA_ARGS__)

	enum LogLevel : uint
	{
		LL_Info,
		LL_Event,
		LL_Warning,
		LL_Error,
		LL_Debug,
	};

	void LogMessage(int _level, const char* _msg, ...);

	//----------------------------------------------------------------------------//
	// NonCopyable
	//----------------------------------------------------------------------------//

	class NonCopyable
	{
	public:
		NonCopyable(void) { }
		~NonCopyable(void) { }

	protected:
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator = (const NonCopyable&) = delete;
	};

	//----------------------------------------------------------------------------//
	// Singleton
	//----------------------------------------------------------------------------//

	template <class T> class Singleton : public NonCopyable
	{
	public:
		Singleton(void) { assert(s_instance == nullptr); s_instance = static_cast<T*>(this); }
		~Singleton(void) { s_instance = nullptr; }
		static T* Get(void) { return s_instance; }

	protected:
		static T* s_instance;
	};

	template <class T> T* Singleton<T>::s_instance = nullptr;
	//----------------------------------------------------------------------------//
	// RefCounted
	//----------------------------------------------------------------------------//

#define SAFE_ADDREF(p) if (p) { p->AddRef(); }
#define SAFE_RELEASE(p) if (p) { p->Release(); }
#define SAFE_ASSIGN(lhs, rhs) SAFE_ADDREF(rhs) SAFE_RELEASE(lhs) lhs = rhs
#define CLASS(T) \
	static const String& StaticClassName(void) { static const String _name = #T; return _name; } \
	static uint StaticClassID(void) { static const uint _id = NameHash(StaticClassName()); return _id; } \
	uint ClassID(void) override { return StaticClassID(); } \
	const String& ClassName(void) override { return StaticClassName(); } \
 	bool IsClass(uint _id) override { return _id == StaticClassID() || __super::IsClass(_id); }

	class RefCounted : public NonCopyable
	{
	public:
		static const String& StaticClassName(void) { static const String _name = "RefCounted"; return _name; }
		static uint StaticClassID(void) { static const uint _id = NameHash(StaticClassName()); return _id; }
		virtual uint ClassID(void) { return StaticClassID(); }
		virtual const String& ClassName(void) { return StaticClassName(); }
		virtual bool IsClass(uint _id) { return _id == StaticClassID(); }
		bool IsClass(const char* _name) { return IsClass(NameHash(_name)); }
		bool IsClass(const String& _name) { return IsClass(NameHash(_name)); }

		RefCounted(void) : m_refs(0) { }
		virtual ~RefCounted(void) { }
		void AddRef(void) { ++m_refs; }
		void Release(void) { if (!--m_refs) _DeleteThis(); }
		int GetRefs(void) { return m_refs; }

	protected:

		virtual void _DeleteThis(void) { delete this; }

		Atomic<uint> m_refs; // public links
	};

	//----------------------------------------------------------------------------//
	// WeakReference
	//----------------------------------------------------------------------------//

	class WeakReference final : public NonCopyable
	{
	public:
		void AddRef(void) { ++m_refs; }
		void Release(void) { if (!--m_refs) delete this; }
		int GetRefs(void) { return m_refs; }
		class BaseObject* GetPtr(void) const { return m_object; }

	private:
		friend class BaseObject;
		WeakReference(BaseObject* _object) : m_refs(1), m_object(_object) { }
		void _Reset(void) { m_object = nullptr; Release(); }

		Atomic<int> m_refs;
		BaseObject* m_object;
	};

	//----------------------------------------------------------------------------//
	// BaseObject
	//----------------------------------------------------------------------------//

	class BaseObject : public RefCounted
	{
	public:
		CLASS(BaseObject);

		BaseObject(void) : m_weakRef(nullptr) { }
		virtual ~BaseObject(void)
		{
			if (m_weakRef)
				m_weakRef->_Reset();
		}
		WeakReference* GetRef(void) const
		{
			if (!m_weakRef)
				m_weakRef = new WeakReference(const_cast<BaseObject*>(this));
			return m_weakRef;
		}

	private:
		mutable WeakReference* m_weakRef;
	};

	//----------------------------------------------------------------------------//
	// Ptr
	//----------------------------------------------------------------------------//

	template <class T> class Ptr final
	{
	public:
		Ptr(void) : p(nullptr) { }
		Ptr(const Ptr& _p) : p(const_cast<T*>(_p.p)) { SAFE_ADDREF(p); }
		Ptr(const T* _p) : p(const_cast<T*>(_p)) { SAFE_ADDREF(p); }
		~Ptr(void) { SAFE_RELEASE(p); }
		Ptr& operator = (const Ptr& _p) { SAFE_ASSIGN(p, const_cast<T*>(_p.p)); return *this; }
		Ptr& operator = (const T* _p) { SAFE_ASSIGN(p, const_cast<T*>(_p)); return *this; }
		T& operator * (void) const { assert(p != nullptr); return *const_cast<T*>(p); }
		T* operator -> (void) const { assert(p != nullptr); return const_cast<T*>(p); }
		operator T* (void) const { return const_cast<T*>(p); }
		T* Get(void) const { return const_cast<T*>(p); }
		template <class X> X* Cast(void) const { return static_cast<X*>(const_cast<T*>(p)); }
		template <class X> X* DynamicCast(void) const { return dynamic_cast<X*>(const_cast<T*>(p)); }

		T*& _Ptr(void) { return p; }
		Ptr& _SetNoAddRef(T* _p) { SAFE_RELEASE(p); p = _p; return *this; }

	protected:
		T* p;
	};

	//----------------------------------------------------------------------------//
	// Ref
	//----------------------------------------------------------------------------//

	template <class T> class Ref
	{
		Ptr<WeakReference> p;
		typedef Ptr<T> Ptr;
	public:

		Ref(void) : p(nullptr) { }
		Ref(const Ref& _other) : p(_other.p) { }
		Ref(const Ptr& _object) : p(GetRef(_object)) { }
		Ref(const T* _object) : p(GetRef(_object)) { }
		Ref& operator = (const Ref& _rhs) { p = _rhs.p; return *this; }
		Ref& operator = (const Ptr& _rhs) { p = GetRef(_rhs); return *this; }
		Ref& operator = (const T* _rhs) { p = GetRef(_rhs); return *this; }
		T& operator* (void) const { assert(GetPtr(p) != nullptr); return *GetPtr(p); }
		T* operator-> (void) const { assert(GetPtr(p) != nullptr); return GetPtr(p); }
		operator T* (void) const { return GetPtr(p); }
		T* Get(void) const { return GetPtr(p); }
		bool operator == (const Ref& _rhs) const { return p == _rhs.p; }
		bool operator != (const Ref& _rhs) const { return p != _rhs.p; }
		WeakReference* GetRef(void) const { return p; }
		template <class X> X* Cast(void) const { return static_cast<X*>(GetPtr(p)); }
		template <class X> X* DyncamicCast(void) const { return dynamic_cast<X*>(GetPtr(p)); }
		static WeakReference* GetRef(const T* _object) { return _object ? _object->GetRef() : nullptr; }
		static T* GetPtr(const WeakReference* _weakRef) { return _weakRef ? static_cast<T*>(_weakRef->GetPtr()) : nullptr; }
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}

namespace std
{
	template<class T> struct hash<Engine::Ptr<T>> : public std::hash<T*>
	{
	};

	template<class T> struct hash<Engine::Ref<T>> : public std::hash<Engine::WeakReference*>
	{
		size_t operator()(const Engine::Ref<T>& _Keyval) const
		{
			return std::hash<Engine::WeakReference*>::operator()(_Keyval.GetRef());
		}
	};
}

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Time
	//----------------------------------------------------------------------------//

	double TimeS(void);
	double TimeMs(void);
	double TimeUs(void);

	//----------------------------------------------------------------------------//
	// File
	//----------------------------------------------------------------------------//

	class File : public NonCopyable
	{
	public:
		File(void);
		~File(void);
		File(const String& _name, FILE* _handle, bool _readOnly = true);
		File(File&& _temp);
		File& operator = (File&& _temp);

		operator bool(void) const { return m_handle != nullptr; }
		FILE* Handle(void) { return m_handle; }
		const String& Name(void) { return m_name; }
		uint Size(void) { return m_readOnly ? m_size : _ReadSize(); }
		void Seek(int _pos, int _origin = SEEK_SET);
		uint Tell(void);
		bool Eof(void);
		uint Read(void* _dst, uint _size);
		uint Write(const void* _src, uint _size);
		void Flush(void);
		String AsString(void);

	protected:
		uint _ReadSize(void);

		String m_name;
		FILE* m_handle;
		uint m_size;
		bool m_readOnly;
	};

	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//

#define gFileSystem Engine::FileSystem::Get()

	class FileSystem : public NonCopyable
	{
	public:
		static FileSystem* Get(void) { return &s_instance; }

		bool SetRoot(const String& _path);
		bool AddPath(const String& _path);
		void SetWriteDir(const String& _path) { m_writeDir = MakeFullPath(_path, m_rootDir) + "/"; }
		const String& GetWriteDir(void) { return m_writeDir; }
		bool FileExists(const String& _path);
		String FindFile(const String& _path);
		time_t FileTime(const String& _path);
		File ReadFile(const String& _name);
		File WriteFile(const String& _name, bool _overwrite = true);
		bool CreateDir(const String& _path);

	protected:

		FileSystem(void);
		~FileSystem(void);
		bool _CreateDir(const String& _path);

		String m_appDir;
		String m_rootDir;
		String m_writeDir; 
		List<String> m_paths;

		static FileSystem s_instance;
	};

	//----------------------------------------------------------------------------//
	// Config
	//----------------------------------------------------------------------------//

	class Config
	{
	public:

		Config(void) {}
		~Config(void) {}
		Config(const Config& _other);
		Config(Config&& _temp);
		Config& operator = (Config&& _temp);
		Config& operator = (const Config& _rhs);
		Config(const String& _name, const String& _value = "");

		bool IsEmpty(void) const { return m_name.empty() && m_value.empty() && m_childs.empty(); }

		Config& SetName(const String& _name) { m_name = _name; return *this; }
		const String& GetName(void) const { return m_name; }
		Config& SetValue(const String& _value) { m_value = _value; return *this; }
		Config& SetValueF(const char* _fmt, ...);
		const String& GetValue(void) const { return m_value; }

		Config& AddChild(const String& _name, const String& _value = "");
		Array<Config>& GetChilds(void) { return m_childs; }
		const Array<Config>& GetChilds(void) const { return m_childs; }
		uint GetNumChilds(void) const { return (uint)m_childs.size(); }
		const Config& GetChild(uint _index) const { return _index < m_childs.size() ? m_childs[_index] : Empty; }
		Config* FindChild(const String& _name, Config* _prev = nullptr);
		const Config& GetChild(const String& _name) const;
		Config& RemoveAllChilds(void);

		bool Parse(const String& _text, uint* _errorLine = nullptr);
		bool Parse(const char* _text, uint* _errorLine = nullptr);
		String Print(void) const;

		static const Config Empty;

	protected:

		struct Lexer
		{
			const char* s;
			uint l;
			bool operator()(String& _token, bool& _isString);
		};

		bool _Parse(Lexer& _lex);
		static String _PrintValue(const String& _value);
		void _Print(String& _dst, int _offset) const;

		String m_name;
		String m_value;
		Array<Config> m_childs;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	inline const Config& operator >> (const Config& _lhs, float& _rhs)
	{
		if (_lhs.GetValue().empty() || !sscanf(_lhs.GetValue().c_str(), "%f", &_rhs))
			_rhs = 0;
		return _lhs;
	}
	inline const Config& operator >> (const Config& _lhs, int& _rhs)
	{
		if (_lhs.GetValue().empty() || !sscanf(_lhs.GetValue().c_str(), "%d", &_rhs))
			_rhs = 0;
		return _lhs;
	}
	inline const Config& operator >> (const Config& _lhs, uint& _rhs)
	{
		if (_lhs.GetValue().empty() || !sscanf(_lhs.GetValue().c_str(), "%u", &_rhs))
			_rhs = 0;
		return _lhs;
	}

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	inline const Config& operator << (Config& _lhs, uint _rhs) { return _lhs.SetValueF("%u", _rhs); }
	inline const Config& operator << (Config& _lhs, int _rhs) { return _lhs.SetValueF("%d", _rhs); }
	inline const Config& operator << (Config& _lhs, bool _rhs) { return _lhs.SetValueF("%d", _rhs); }
	inline const Config& operator << (Config& _lhs, float _rhs) { return _lhs.SetValueF("%f", _rhs); }
	inline const Config& operator << (Config& _lhs, double _rhs) { return _lhs.SetValueF("%f", _rhs); }
	
	//----------------------------------------------------------------------------//
	// Resource
	//----------------------------------------------------------------------------//

	typedef	Ptr<class Resource> ResourcePtr;

	enum ResourceFlags : uint
	{
		RF_Persistent, //!< for preloadable resources (resource cannot be unloaded automatically)
		RF_Manually, //!< for manual resources (manually created resource cannot be loaded or unloaded automatically)
		RF_Temp, //!< for temporary resources (lifetime of resource depends on counter of references)
	};

	enum ResourceState : uint
	{
		RS_Unloaded, //!< initial state
		RS_Queued, //!< resource will be loaded at closest time
		RS_Loading,	//!< resource is loading now
		RS_Loaded, //!< resource is loaded
	};

	enum ResourceLoadFlags : uint
	{
		RLF_Wait = 0,
		RLF_Async = 0x1,
		RLF_Reload = 0x2,
		RLF_ForceReload = RLF_Reload | 0x4,
	};

	typedef ResourcePtr(*ResourceFactory)(void);

	class Resource : public RefCounted
	{
	public:
		CLASS(Resource);

		Resource(void);
		~Resource(void);

		const String& GetName(void) { return m_name; }
		uint GetID(void) { return m_id; }
		bool IsManualResource(void) { return (m_resourceFlags & RF_Manually) != 0; }
		bool IsTempResource(void) { return (m_resourceFlags & RF_Temp) != 0; }
		bool IsPersistentResource(void) { return (m_resourceFlags & RF_Persistent) != 0; }
		void SetPersistentResource(bool _persistent = false) { _persistent ? (m_resourceFlags |= RF_Persistent) : (m_resourceFlags &= ~RF_Persistent); }
		bool IsLoaded(void) { return m_resourceState == RS_Loaded; }
		bool IsQueued(void) { return m_resourceState == RS_Queued; }

		virtual bool BeginReload(void);
		virtual void Load(uint _flags = 0);
		virtual bool Touch(bool _wait = true);

	protected:
		friend class ResourceManager;
		friend class ResourceCache;

		virtual void _InitResource(ResourceManager* _mgr, const String& _name, uint _uid, uint _flags);
		virtual void _GetFileName(void) { m_fileName = m_name; }
		virtual bool _Load(void);
		virtual bool _Load(File& _f) { return _f != false; }
		bool _ChangeState(ResourceState _exp, ResourceState _state);
		void _DeleteThis(void) override;

		String m_name; //!<\note read only
		String m_fileName;
		uint m_id; //!<\note read only
		ResourceManager* m_resourceMgr; //!<\note read only
		uint m_resourceFlags; //!<\note read only
		Atomic<ResourceState> m_resourceState;
		time_t m_fileTime;
		bool m_valid;
		bool m_trackingEnabled;

	private:
		Resource* m_qnext;
		Resource* m_qprev;
	};

	//----------------------------------------------------------------------------//
	// ResourceManager
	//----------------------------------------------------------------------------//

	class ResourceManager : public RefCounted
	{
	public:
		CLASS(ResourceManager);

		ResourceManager(const String& _class);
		~ResourceManager(void);

		const String& GetResourceClassName(void) { return m_class; }
		uint GetResourceClassID(void) { return m_classID; }
		Resource* AddResource(const String& _name, uint _flags = 0);
		virtual void RemoveResource(Resource* _r, bool _unqueue = true);
		virtual uint RemoveUnusedResources(void);
		virtual uint ReloadResources(bool _wait = true);
		virtual void AddResourceForReload(Resource* _r);

	protected:
		friend class Resource;
		friend class ResourceCache;

		void _RemoveRefs(Resource* _r);
		void _RemoveAllResources(void);
		virtual ResourcePtr	_Create(const String& _name, uint _id, uint _flags);
		virtual ResourcePtr _Factory(void) = 0;
		virtual void _BeginReloading(bool _wait) { }
		virtual void _EndReloading(bool _wait) { }

		String m_class;
		uint m_classID;
		HashMap<uint, ResourcePtr> m_cache;
		HashMap<uint, Resource*> m_resources; // all resources
		HashSet<ResourcePtr> m_resourcesToReload;
		Atomic<uint> m_reloading;
	};

	//----------------------------------------------------------------------------//
	// GenericResourceManager
	//----------------------------------------------------------------------------//

	template <class T> class GenericResourceManager : public ResourceManager
	{
	public:
		static const String& StaticClassName(void) { static const String _name = "GenericResourceManager<" + T::StaticClassName() + ">"; return _name; }
		static uint StaticClassID(void) { static const uint _id = NameHash(StaticClassName()); return _id; }
		uint ClassID(void) override { return StaticClassID(); }
		const String& ClassName(void) override { return StaticClassName(); }
		bool IsClass(uint _id) override { return _id == StaticClassID() || __super::IsClass(_id); }

		GenericResourceManager(const String& _class, ResourceFactory _factory) : ResourceManager(_class), m_factory(_factory) { }

	protected:
		ResourcePtr _Factory(void) override { return m_factory(); }

		ResourceFactory m_factory;
	};

	//----------------------------------------------------------------------------//
	// ResourceCache
	//----------------------------------------------------------------------------//

#define gResourceCache Engine::ResourceCache::Get()

	/// Resource cache. Create by Engine::System
	class ResourceCache final : public Singleton<ResourceCache>
	{
	public:

		void RegisterManager(ResourceManager* _manager);
		void UnregisterManager(uint _id);
		template <class T> void Register(void)
		{
			Ptr<GenericResourceManager<T>> _mgr = new GenericResourceManager<T>(T::StaticClassName(), [] { return ResourcePtr(new T); });
			RegisterManager(_mgr);
		}

		ResourceManager* GetManager(uint _id);
		template <class T> ResourceManager* GetManager(void) { return GetManager(T::StaticClassID()); }

		bool IsDeferredLoading(void) { return m_deferredLoading; }
		void SetDeferredLoading(bool _state);

		ResourcePtr LoadResource(uint _type, const String& _name, uint _flags = 0);
		template <class T> Ptr<T> LoadResource(const String& _name, uint _flags = 0)
		{
			return static_cast<T*>(LoadResource(T::StaticClassID(), _name, _flags).Get());
		}
		uint LoadQueuedResources(void);
		bool LoadOneResource(String* _name = nullptr);
		uint RemoveUnusedResources(void);

		void ReloadAllResources(uint _type);
		template <class T> void ReloadAllResources(void) { ReloadAllResources(T::StaticClassID()); }

		void EnableTracking(bool _enabled = true) { m_trackingEnabled = _enabled; }
		bool IsTrackingEnabled(void) { return m_trackingEnabled; }
		// todo: async mode

	protected:
		friend class System;
		friend class Resource;

		ResourceCache(void);
		~ResourceCache(void);
		void _Shutdown(void);
		void _Signal(Resource* _r);
		void _Wait(Resource* _r);
		void _Queue(Resource* _r);
		void _Unqueue(Resource* _r);

		Resource* m_qstart;
		Resource* m_qend;

		bool m_deferredLoading;
		bool m_trackingEnabled;
		HashMap<uint, Ptr<ResourceManager>> m_managers;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
