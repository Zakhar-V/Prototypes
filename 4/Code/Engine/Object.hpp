#pragma once

#include "Base.hpp"
#include "Thread.hpp"

namespace Rx
{
	//----------------------------------------------------------------------------//
	// Definitions
	//----------------------------------------------------------------------------//

	class Object;

	//----------------------------------------------------------------------------//
	// TypeInfo
	//----------------------------------------------------------------------------//
	
	class TypeInfo : public NonCopyable
	{
	public:
		TypeInfo(const String& _name, TypeInfo* _base) : m_name(_name), m_type(_name.Hashi()), m_base(_base) { }

		const String& GetName(void) const { return m_name; }
		NameHash GetType(void) const { return m_type; }
		const TypeInfo* GetBase(void) const { return m_base; }

		//void AddAttribute();
		//const Attribute* GetAttribute();
		//factory


		bool IsTypeOf(uint _type) const
		{
			for (const TypeInfo* i = this; i; i = i->m_base)
			{
				if (i->m_type == _type)
					return true;
			}
			return false;
		}

	protected:

		const String m_name;
		const NameHash m_type;
		TypeInfo* m_base;
		//static HashMap<uint, TypeInfo*> s_types;
	};

	//----------------------------------------------------------------------------//
	// Object
	//----------------------------------------------------------------------------//

#define OBJECT(_name, _base) \
	static Rx::TypeInfo* GetTypeInfoStatic(void) { static TypeInfo _type(_name, nullptr); return &_type; } \
	static Rx::uint GetTypeStatic(void) { return GetTypeInfoStatic()->GetType(); } \
	const Rx::TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); } \
	template <class T> bool IsTypeOf(void) { return IsTypeOf(T::GetTypeStatic()); }	\


	class Object : public RefCounted
	{
	public:
		Object(void) = default;
		~Object(void) = default;

		static TypeInfo* GetTypeInfoStatic(void) { static TypeInfo _type("Object", nullptr); return &_type; }
		static uint GetTypeStatic(void) { return GetTypeInfoStatic()->GetType(); }

		virtual const TypeInfo* GetTypeInfo() const { return GetTypeInfoStatic(); }
		uint GetType(void) const { return GetTypeInfo()->GetType(); };
		const String& GetTypeName(void) const { return GetTypeInfo()->GetName(); };
		const TypeInfo* GetBaseTypeInfo(void) const { return GetTypeInfo()->GetBase(); };
		uint GetBaseType(void) const { const TypeInfo* _base = GetTypeInfo()->GetBase(); return _base ? _base->GetType() : 0; };

		bool IsTypeOf(NameHash _type) const { return GetTypeInfo()->IsTypeOf(_type); }
		template <class T> bool IsTypeOf(void) { return IsTypeOf(T::GetTypeStatic()); }

	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}

