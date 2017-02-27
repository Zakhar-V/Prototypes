#pragma once

#include "Base.hpp"
#include "Thread.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// RefCounted
	//----------------------------------------------------------------------------//

#define SAFE_ADDREF(p) if (p) { p->AddRef(); }
#define SAFE_RELEASE(p) if (p) { p->Release(); }
#define SAFE_ASSIGN(lhs, rhs) SAFE_ADDREF(rhs) SAFE_RELEASE(lhs) lhs = rhs

#define CLASSNAME(T) \
	static uint ClassIdStatic(void) { static const NameHash _clsid(ClassNameStatic()); return _clsid; } \
	static const String& ClassNameStatic(void) { static const String _clsname(#T); return _clsname; } \
	uint ClassId(void) override { return ClassIdStatic(); } \
	const String& ClassName(void) override  { return ClassNameStatic(); } \
	bool IsClass(const NameHash& _id) override { return _id.hash == ClassIdStatic() || __super::IsClass(_id); } \
	template <class X> bool IsClass(void) { return IsClass(X::ClassIdStatic()); }

	class RefCounted : public NonCopyable
	{
	public:
		static uint ClassIdStatic(void) { static const NameHash _clsid(ClassNameStatic()); return _clsid; }
		static const String& ClassNameStatic(void) { static const String _clsname("RefCounted"); return _clsname; }
		virtual uint ClassId(void) { return ClassIdStatic(); }
		virtual const String& ClassName(void) { return ClassNameStatic(); }
		virtual bool IsClass(const NameHash& _id) { return _id.hash == ClassIdStatic(); }
		template <class T> bool IsClass(void) { return IsClass(T::ClassIdStatic()); }

		RefCounted(void) : m_refCount(0) { }
		virtual ~RefCounted(void) { }
		void AddRef(void) { ++m_refCount; }
		void Release(void) { if (!--m_refCount) _DeleteThis(); }
		int GetRefCount(void) { return m_refCount; }

	protected:

		virtual void _DeleteThis(void) { delete this; }

		Atomic<uint> m_refCount;
	};

	//----------------------------------------------------------------------------//
	// WeakReference
	//----------------------------------------------------------------------------//

	class WeakReference final : public NonCopyable
	{
	public:
		void AddRef(void) { ++m_refCount; }
		void Release(void) { if (!--m_refCount) delete this; }
		int GetRefs(void) { return m_refCount; }
		class BaseObject* GetPtr(void) const { return m_object; }

	private:
		friend class BaseObject;

		WeakReference(BaseObject* _object) : m_refCount(1), m_object(_object) { }
		void _Reset(void) { m_object = nullptr; Release(); }

		Atomic<int> m_refCount;
		BaseObject* m_object;
	};

	//----------------------------------------------------------------------------//
	// BaseObject
	//----------------------------------------------------------------------------//

	class BaseObject : public RefCounted
	{
	public:

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
		T& operator * (void) const { ASSERT(p != nullptr); return *const_cast<T*>(p); }
		T* operator -> (void) const { ASSERT(p != nullptr); return const_cast<T*>(p); }
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
		T& operator* (void) const { ASSERT(GetPtr(p) != nullptr); return *GetPtr(p); }
		T* operator-> (void) const { ASSERT(GetPtr(p) != nullptr); return GetPtr(p); }
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

