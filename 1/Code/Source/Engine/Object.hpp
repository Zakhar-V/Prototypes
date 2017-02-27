#pragma once

#include "String.hpp"
#include "Thread.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class Object;
	class Event;
	template <class T> class Ptr;
	template <class T> class Ref;

#define SAFE_ADDREF(p) if (p) { p->AddRef(); }
#define SAFE_RELEASE(p) if (p) { p->Release(); }
#define SAFE_ASSIGN(lhs, rhs) SAFE_ADDREF(rhs) SAFE_RELEASE(lhs) lhs = rhs
#define OBJECT(T) \
	static uint32 ClassIdStatic(void) { static const StringHash _clsid(ClassNameStatic()); return _clsid; } \
	static const String& ClassNameStatic(void) { static const String _clsname(#T); return _clsname; } \
	virtual uint32 ClassId(void) { return ClassIdStatic(); } \
	virtual const String& ClassName(void) { return ClassNameStatic(); }

	//----------------------------------------------------------------------------//
	// RefCounted
	//----------------------------------------------------------------------------//

	class ENGINE_API RefCounted : public NonCopyable
	{
	public:
		RefCounted(void) : m_refCount(0) { }
		virtual ~RefCounted(void) { }
		int AddRef(void) { return ++m_refCount; }
		int Release(void)
		{
			int _r = --m_refCount;
			if (!_r)
				_ReleaseImpl();
			return _r;
		}
		int GetRefCount(void) { return m_refCount; }

	protected:
		AtomicInt m_refCount;

		virtual void _ReleaseImpl(void) { delete this; }
	};

	//----------------------------------------------------------------------------//
	// Object
	//----------------------------------------------------------------------------//

	class ENGINE_API Object : public RefCounted
	{
	public:

		/// The weak reference
		class WeakRef : public RefCounted
		{
			friend class Object;

			WeakRef(const Object* _object) : m_object(const_cast<Object*>(_object)) { AddRef(); }
			void _Reset(void) { m_object = nullptr; Release(); }

			Object* m_object;

		public:
			Object* GetObject(void) const { return m_object; }
		};

		OBJECT(Object);

		Object(void) : m_weakRef(nullptr), m_name(0u) { }
		Object(const String& _name) : m_weakRef(nullptr), m_name(0) { SetName(_name); }
		virtual ~Object(void)
		{ 
			if (m_weakRef)
				m_weakRef->_Reset();
		}
		WeakRef* GetWeakRef(void) const
		{
			if (!m_weakRef)
				m_weakRef = new WeakRef(this);
			return m_weakRef;
		}
		void SetName(const String& _name);
		const String& GetName(void);
		uint32 GetNameHash(void) { return m_name; }

	protected:
		mutable WeakRef* m_weakRef;
		uint32 m_name;
		static Mutex s_namesMutex;
		static HashMap<uint32, String> s_names;
	};

	//----------------------------------------------------------------------------//
	// EventCallback
	//----------------------------------------------------------------------------//

	/// Interface of event callback
	class ENGINE_API EventCallback
	{
	public:	
		virtual ~EventCallback(void) { }
		virtual void Invoke(Event* _event) = 0;
		virtual Object* GetReceiver(void) = 0;
		virtual void SetReceiver(Object* _newReceiver) = 0;
	};

	//----------------------------------------------------------------------------//
	// EventCallbackImpl
	//----------------------------------------------------------------------------//

	/// Implementation of event callback
	template <class T, class E> class EventCallbackImpl : public EventCallback
	{
	public:
		typedef void(T::*Callback)(E&);

		EventCallbackImpl(T* _receiver, Callback& _callback) : receiver(_receiver), callback(_callback) { }
		void Invoke(Event* _event) override
		{
			if (receiver)
				(receiver->*callback)(*static_cast<E*>(_event));
		}
		Object* GetReceiver(void) override { return receiver; }
		void SetReceiver(Object* _newReceiver) override { receiver = static_cast<T*>(_newReceiver); }

		Ref<T> receiver;
		Callback callback;
	};

	//----------------------------------------------------------------------------//
	// SignalCallbackImpl
	//----------------------------------------------------------------------------//

	/// Implementation of event callback without arg
	template <class T> class SignalCallbackImpl : public EventCallback
	{
	public:
		typedef void(T::*Callback)(void);

		SignalCallbackImpl(T* _receiver, Callback& _callback) : receiver(_receiver), callback(_callback) { }
		void Invoke(Event* _event) override
		{
			if (receiver)
				(receiver->*callback)();
		}
		Object* GetReceiver(void) override { return receiver; }
		void SetReceiver(Object* _newReceiver) override { receiver = static_cast<T*>(_newReceiver); }

		Ref<T> receiver;
		Callback callback;
	};

	//----------------------------------------------------------------------------//
	// Event
	//----------------------------------------------------------------------------//

	/// Base class of event
	class ENGINE_API Event : public NonCopyable
	{
	public:
		virtual ~Event(void);

		
		template <class T, class E> void Subscribe(T* _receiver, void(T::*_callback)(E&))
		{
			Subscribe(_receiver, new EventCallbackImpl<T, E>(_receiver, _callback));
		}
		template <class T, class E> void Subscribe(const Ptr<T>& _receiver, void(T::*_callback)(E&))
		{
			Subscribe(_receiver, new EventCallbackImpl<T, E>(_receiver, _callback));
		}
		template <class T, class E> void Subscribe(const Ref<T>& _receiver, void(T::*_callback)(E&))
		{
			Subscribe(_receiver, new EventCallbackImpl<T, E>(_receiver, _callback));
		}

		template <class T> void Subscribe(T* _receiver, void(T::*_callback)())
		{
			Subscribe(_receiver, new SignalCallbackImpl<T>(_receiver, _callback));
		}
		template <class T> void Subscribe(const Ptr<T>& _receiver, void(T::*_callback)())
		{
			Subscribe(_receiver, new SignalCallbackImpl<T>(_receiver, _callback));
		}
		template <class T> void Subscribe(const Ref<T>& _receiver, void(T::*_callback)())
		{
			Subscribe(_receiver, new SignalCallbackImpl<T>(_receiver, _callback));
		}

		void Subscribe(Object* _receiver, EventCallback* _callback);
		void Unsubscribe(Object* _receiver);
		bool IsSubscribed(Object* _receiver);

		void Send(Object* _sender = nullptr);
		Object* GetSender(void) { return m_sender; }

	protected:
		HashMap<void*, EventCallback*> m_callbacks;
		Object* m_sender = nullptr;
		//CriticalSection m_mutex;
	};

	//----------------------------------------------------------------------------//
	// Ptr
	//----------------------------------------------------------------------------//

	template <class T> class Ptr
	{
		T* p;

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
	};

	//----------------------------------------------------------------------------//
	// Ref
	//----------------------------------------------------------------------------//

	template <class T> class Ref
	{
		Ptr<Object::WeakRef> p;
		typedef Ptr<T> Ptr;
	public:

		Ref(void) : p(nullptr) { }
		Ref(const Ref& _other) : p(_other.p) { }
		Ref(const Ptr& _object) : p(GetWeakRef(_object)) { }
		Ref(const T* _object) : p(GetWeakRef(_object)) { }
		Ref& operator = (const Ref& _rhs) { p = _rhs.p; return *this; }
		Ref& operator = (const Ptr& _rhs) { p = GetWeakRef(_rhs); return *this; }
		Ref& operator = (const T* _rhs) { p = GetWeakRef(_rhs); return *this; }
		T& operator* (void) const { assert(GetObject(p) != nullptr); mreturn *GetObject(p); }
		T* operator-> (void) const { assert(GetObject(p) != nullptr); return GetObject(p); }
		operator T* (void) const { return GetObject(p); }
		T* Get(void) const { return GetObject(p); }
		template <class X> X* Cast(void) const { return static_cast<X*>(GetObject(p)); }
		template <class X> X* DyncamicCast(void) const { return dynamic_cast<X*>(GetObject(p)); }
		static Object::WeakRef* GetWeakRef(const T* _object) { return _object ? _object->GetWeakRef() : nullptr; }
		static T* GetObject(const Object::WeakRef* _weakRef) { return _weakRef ? static_cast<T*>(_weakRef->GetObject()) : nullptr; }
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}