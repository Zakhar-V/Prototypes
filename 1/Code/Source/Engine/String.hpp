#pragma once

#include "Container.hpp" // for Array<T>
#include <string>

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class String;
	struct StringHash;
	typedef Array<String> StringArray;

	//----------------------------------------------------------------------------//
	// WString
	//----------------------------------------------------------------------------//

	class ENGINE_API WString
	{

	};

	//----------------------------------------------------------------------------//
	// String
	//----------------------------------------------------------------------------//

	class ENGINE_API String : public std::string
	{
	public:
		typedef std::string Base;

		String(void) { }
		~String(void) { }
		String(const String& _other) : Base(_other) { }
		String(String&& _temp) : Base(Move(_temp)) { }
		String(const Base& _other) : Base(_other) { }
		String(Base&& _temp) : Base(Move(_temp)) { }
		String(int _ch) : Base((const char*)&_ch, 1) { }
		String(char _ch) : Base(&_ch, 1) { }
		String(const char* _str) : Base(_str ? _str : "") { }
		String(uint _count, char _ch) : Base(_count, _ch) { }
		String(const char* _str, uint _count) : Base(_str ? _str : "", _count) { }
		String(const char* _first, const char* _last) : Base(_first, _last) { }

		String& operator = (const String& _rhs) { Base::operator=(_rhs); return *this; }
		String& operator = (String&& _temp) { Base::operator=(Move(_temp)); return *this; }
		String& operator = (const Base& _rhs) { Base::operator=(_rhs); return *this; }
		String& operator = (Base&& _temp) { Base::operator=(Move(_temp)); return *this; }
		String& operator = (const char* _str) { Base::operator=(_str ? _str : ""); return *this; }
		String& operator = (char _ch) { Base::operator=(_ch); return *this; }

		operator const char* (void) const { return c_str(); }
		char operator [] (uint _idx) const { return at(_idx); }

		bool operator == (const Base& _str) const { return compare(_str) == 0; }
		bool operator == (const char* _str) const { return compare(_str) == 0; }
		bool operator != (const Base& _str) const { return compare(_str) != 0; }
		bool operator != (const char* _str) const { return compare(_str) != 0; }
		bool operator < (const Base& _str) const { return compare(_str) < 0; }
		bool operator < (const char* _str) const { return compare(_str) < 0; }
		bool operator <= (const Base& _str) const { return compare(_str) <= 0; }
		bool operator <= (const char* _str) const { return compare(_str) <= 0; }

		String& operator += (const String& _lhs) { append(_lhs); return *this; }
		String& operator += (char _lhs) { append(&_lhs, 1); return *this; }
		String& operator += (const char* _lhs) { append(_lhs); return *this; }
		String& operator += (const Base& _lhs) { append(_lhs); return *this; }
		String operator + (const String& _lhs) const { return String(*this).Append(_lhs); }
		String operator + (char _lhs) const { return String(*this).Append(_lhs); }
		String operator + (const char* _lhs) const { return String(*this).Append(_lhs); }
		String operator + (const Base& _lhs) const { return String(*this).Append(_lhs); }
		friend String operator+ (char _lhs, const String& _rhs) { return String(_lhs).Append(_rhs); }
		friend String operator+ (const char* _lhs, const String& _rhs) { return String(_lhs).Append(_rhs); }

		String& Append(char _ch) { append(&_ch, 1); return *this; }
		String& Append(uint _count, char _ch) { append(_count, _ch); return *this; }
		String& Append(const char* _str) { append(_str); return *this; }
		String& Append(const char* _str, uint _length) { append(_str, _length); return *this; }
		String& Append(const Base& _str) { append(_str); return *this; }
		String& Append(const Base& _str, uint _length) { append(_str, _length); return *this; }
		void Split(const char* _delimiters, StringArray& _dst) const { Split(c_str(), _delimiters, _dst); }

		bool IsEmpty(void) const { return empty(); }
		bool NonEmpty(void) const { return !empty(); }
		uint Length(void) const { return (uint)length(); }
		char At(uint _idx) const { return at(_idx); }
		const char* CStr(void) const { return c_str(); }
		const char* Ptr(void) const { return c_str(); }
		const char* Ptr(uint _offset) const { return c_str() + _offset; }
		String SubStr(uint _offset) const { return substr(_offset, npos); }
		String SubStr(uint _offset, uint _count) const { return substr(_offset, _count); }
		char Back(void) const { return back(); }
		uint32 NameHash(uint32 _hash = 0) const { return NameHash(CStr(), _hash); }

		static String Format(const char* _fmt, ...);
		static String FormatV(const char* _fmt, va_list _args);
		static void Split(const char* _str, const char* _delimiters, StringArray& _dst);
		/// Get case-insensitivity hash.
		static uint32 NameHash(const char* _str, uint32 _hash = 0);
		static int Compare(const char* _a, const char* _b, bool _ignoreCase = false);

		static const String Empty;

	protected:
#ifndef USE_STL
		struct Buffer
		{
			volatile int refs;
			uint length;
			char str[1];
		};
		Buffer m_buffer;
		NOT_IMPLEMENTED_YET_EX("String without STL");
#endif
	};

	//----------------------------------------------------------------------------//
	// StringHash 
	//----------------------------------------------------------------------------//

	struct StringHash
	{
		StringHash(void) : hash(0) { }
		StringHash(uint32 _value) : hash(_value) { }
		StringHash(const String& _str, uint32 _hash = 0) : hash(String::NameHash(_str, _hash)) { }
		StringHash(const char* _str, uint32 _hash = 0) : hash(String::NameHash(_str, _hash)) { }

		StringHash& operator = (const char* _str) { hash = String::NameHash(_str, hash); return *this; }
		StringHash& operator += (const char* _str) { hash = String::NameHash(_str, hash); return *this; }

		operator uint32 (void) const { return hash; }

		bool operator == (const StringHash& _rhs) const { return hash == _rhs.hash; }
		bool operator != (const StringHash& _rhs) const { return hash != _rhs.hash; }
		bool operator < (const StringHash& _rhs) const { return hash < _rhs.hash; }
		bool operator <= (const StringHash& _rhs) const { return hash <= _rhs.hash; }

		uint32 hash;
	};

	//----------------------------------------------------------------------------//
	// CharStream
	//----------------------------------------------------------------------------//

	struct CharStream
	{
		CharStream(const char* _str) : s(_str), l(1), c(1) { }
		operator const char* (void) { return s; }
		char operator [] (int i) { return s[i]; }
		const char* operator ++ (void) { Next(); return s; }
		const char* operator ++ (int) { const char* p = s; Next(); return p; }
		const char* operator += (int _c) { ASSERT(_c >= 0); Next(_c); return s; }

		void Next(uint _c = 1)
		{
			while (*s && _c--)
			{
				if (strchr("\n\r", *s))
				{
					if (s[0] == '\r' && s[1] == '\n')
						++s;
					c = 0;
					++l;
				}
				c += (*s == '\t') ? (4 - c % 4) : 1; // tab
				++s;
			}
		}
		uint Skip(const char* _cset, bool _op = true)
		{
			ASSERT(_cset != nullptr);
			const char* _start = s;
			while (*s && (strchr(_cset, *s) != 0) == _op)
				Next();
			return (uint)(s - _start);
		}
		bool AnyOf(const char* _cset, bool _op = true)
		{
			ASSERT(_cset != nullptr);
			return *s && (strchr(_cset, *s) != 0) == _op;
		}

		const char* s; // stream
		uint l, c; // line, column
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}