#include "../Base.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Atomic
	//----------------------------------------------------------------------------//

	int32 AtomicAdd(int32& _atomic, int32 _value);
	void AtomicLock(int& _atomic);
	void AtomicUnlock(int& _atomic);

	//----------------------------------------------------------------------------//
	// String
	//----------------------------------------------------------------------------//

	const String String::Empty;
	int String::s_memory = 0;

	//----------------------------------------------------------------------------//
	String& String::Clear(void)
	{
		if (m_buffer->length > 0)
		{
			if (AtomicAdd(m_buffer->refs, 1) != 1)
			{
				_Release(m_buffer, -2);
				m_buffer = _Null();
			}
			else
			{
				m_buffer->length = 0;
				m_buffer->str[0] = 0;
				_Release(m_buffer);
			}
		}
		return *this;
	}
	//----------------------------------------------------------------------------//
	String& String::Reserve(uint _size)
	{
		_Unique(m_buffer, _size, false);
		return *this;
	}
	//----------------------------------------------------------------------------//
	String& String::Append(uint _count, char _ch)
	{
		if (_ch && _count)
		{
			char* _dst = _Unique(m_buffer, m_buffer->length + _count, true)->str + (m_buffer->length += _count);
			while (_count--)
				*--_dst = _ch;
		}
		return *this;
	}
	//----------------------------------------------------------------------------//
	String& String::Append(const char* _str, int _length, bool _quantizeMemory)
	{
		if (_str && *_str)
		{
			_length = _length < 0 ? (int)strlen(_str) : _length;
			_Unique(m_buffer, m_buffer->length + _length, _quantizeMemory);
			memcpy(m_buffer->str + m_buffer->length, _str, _length);
			m_buffer->length += _length;
			m_buffer->str[m_buffer->length] = 0;
		}
		return *this;
	}
	//----------------------------------------------------------------------------//
	String String::Trim(const char* _cset, bool _left, bool _right) const
	{
		if (!_cset)
			return *this;

		const char* _s = m_buffer->str;
		const char* _e = m_buffer->str + m_buffer->length;
		if (_left) while (_s < _e && strchr(_cset, *_s))
			++_s;
		if (_right) while (_s < _e && strchr(_cset, _e[-1]))
			--_e;

		uint _length = (uint)(_e - _s);
		if (_length == m_buffer->length)
			return *this;
		if (_length == 0)
			return Empty;
		return String(_s, _e);
	}
	//----------------------------------------------------------------------------//
	String String::Trim(const char* _str, const char* _cset, bool _left, bool _right)
	{
		if (!_str || !_str[0])
			return Empty;
		if (!_cset || !_cset[0])
			return _str;

		const char* _s = _str;
		const char* _e = _str + strlen(_str);
		if (_left) while (_s < _e && strchr(_cset, *_s))
			++_s;
		if (_right) while (_s < _e && strchr(_cset, _e[-1]))
			--_e;
		return String(_s, _e);
	}
	//----------------------------------------------------------------------------//
	String String::Format(const char* _fmt, ...)
	{
		va_list _args;
		va_start(_args, _fmt);
		String _str = FormatV(_fmt, _args);
		va_end(_args);
		return _str;
	}
	//----------------------------------------------------------------------------//
	String String::FormatV(const char* _fmt, va_list _args)
	{
#if 0
		char _buff[4096];
		vsnprintf_s(_buff, sizeof(_buff), _fmt, _args);
		return _buff;
#else
		//this code from ptypes: pputf.cxx
		struct
		{
			bool operator () (char _ch) const
			{
				return strchr(" #+-~.0123456789", _ch) != 0;
			}
		} _check_fmtopts;
		enum fmt_type_t { FMT_NONE, FMT_CHAR, FMT_SHORT, FMT_INT, FMT_LONG, FMT_LARGE, FMT_STR, FMT_PTR, FMT_DOUBLE, FMT_LONG_DOUBLE };

		String _res;
		fmt_type_t fmt_type;
		const char *e, *p = _fmt;
		char buf[4096], fbuf[128];
		while (p && *p != 0)
		{
			// write out raw data between format specifiers
			e = strchr(p, '%');
			if (e == 0)
				e = p + strlen(p);
			if (e > p)
				_res.Append(p, e);
			if (*e != '%')
				break;
			if (*++e == '%') // write out a single '%' 
			{
				_res += '%';
				p = e + 1;
				continue;
			}


			// build a temporary buffer for the conversion specification
			fbuf[0] = '%';
			char* f = fbuf + 1;
			bool modif = false;

			// formatting flags and width specifiers
			while (_check_fmtopts(*e) && uint(f - fbuf) < sizeof(fbuf) - 5)
				*f++ = *e++, modif = true;

			// prefixes
			switch (*e)
			{
			case 'h':
				fmt_type = FMT_SHORT;
				*f++ = *e++;
				break;

			case 'L':
				fmt_type = FMT_LONG_DOUBLE;
				*f++ = *e++;
				break;

			case 'l':
			{
				if (*++e == 'l')
				{
#                   if defined(_MSC_VER) || defined(__BORLANDC__)
					*f++ = 'I'; *f++ = '6'; *f++ = '4';
#                   else
					*f++ = 'l'; *f++ = 'l';
#                   endif
					++e;
					fmt_type = FMT_LARGE;
				}
				else
				{
					*f++ = 'l';
					fmt_type = FMT_LONG;
					// x64 ?
				}
			} break;

			default:
				fmt_type = FMT_NONE;
				break;
			};

			// format specifier
			switch (*e)
			{
			case 'c':
				fmt_type = FMT_CHAR;
				*f++ = *e++;
				break;

			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				if (fmt_type < FMT_SHORT || fmt_type > FMT_LARGE)
					fmt_type = FMT_INT;
				*f++ = *e++;
				break;

			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':
				if (fmt_type != FMT_LONG_DOUBLE)
					fmt_type = FMT_DOUBLE;
				*f++ = *e++;
				break;

			case 's':
				fmt_type = FMT_STR;
				*f++ = *e++;
				break;

			case 'p':
				fmt_type = FMT_PTR;
				*f++ = *e++;
				break;
			};
			if (fmt_type == FMT_NONE)
				break;
			*f = 0;

			// some formatters are processed here 'manually', while others are passed to snprintf
			int s = 0;
			switch (fmt_type)
			{
			case FMT_NONE: // to avoid compiler warning 
				break;
			case FMT_CHAR:
				if (modif)
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
				else
					_res += (char)(va_arg(_args, int));
				break;

			case FMT_SHORT:
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
				break;

			case FMT_INT:
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
				break;

			case FMT_LONG:
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long));
				break;

			case FMT_LARGE:
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int64));
				break;

			case FMT_STR:
			{
				if (modif)
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, const char*));
				else
				{
					const char* _str = va_arg(_args, const char*);
					if (_str)
						_res += _str;
					//else _res += "<null>";
				}
			}break;

			case FMT_PTR:
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, void*));
				break;
			case FMT_DOUBLE:

				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, double));
				for (; s > 0; --s)
				{
					if (buf[s - 1] == '0')
						buf[s - 1] = 0;
					else if (buf[s - 1] == '.')
					{
						buf[--s] = 0;
						break;
					}
					else
						break;
				}
				break;

			case FMT_LONG_DOUBLE:
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long double));
				break;
			}

			if (s > 0)
				_res.Append(buf, s);

			p = e;
		}
		return _res;
#endif
	}
	//----------------------------------------------------------------------------//
	uint32 String::Hash(const char* _str, uint32 _hash)
	{
		if (_str) while (*_str)
			_hash = *_str++ + (_hash << 6) + (_hash << 16) - _hash;
		return _hash;
	}
	//----------------------------------------------------------------------------//
	uint32 String::Hashi(const char* _str, uint32 _hash)
	{
		if (_str) while (*_str)
			_hash = ToLower(*_str++) + (_hash << 6) + (_hash << 16) - _hash;
		return _hash;
	}
	//----------------------------------------------------------------------------//
	int String::Compare(const char* _a, const char* _b, bool _ignoreCase)
	{
		_a = _a ? _a : "";
		_b = _b ? _b : "";
		if (_ignoreCase)
		{
			while (*_a && ToLower(*_a++) == ToLower(*_b++));
			return *_a - *_b;
		}
		return strcmp(_a, _b);
	}
	//----------------------------------------------------------------------------//
	bool String::Match(const char* _str, const char* _pattern, bool _ignoreCase)
	{
		// '*' - any symbols sequence
		// ' '(space) - any number of spaces
		// '['charset']' - set of symbols
		// '['lower'-'upper']' - range of symbols
		// "file.ext" matched "*.ext"

		if (!_str || !_str[0])
			return false;
		if (!_pattern || !_pattern[0])
			return false;

		Array<std::pair<const char*, const char*>> _stack; // <string, pattern>
		_stack.push_back({ _str, _pattern });
		for (const char *_s = _str, *_p = _pattern;;)
		{
		$_frame:
			if (*_p == 0 && *_s == 0)
				goto $_true;
			if (*_s == 0 && *_p != '*')
				goto $_false;
			if (*_p == '*') // подстановка
			{
				++_p;
				if (*_p == 0) goto $_true;
				for (;; )
				{
					_stack.back().first = _s;
					_stack.back().second = _p;
					_stack.push_back({ _s, _p });
					goto $_frame;
				$_xmatch_true:
					goto $_true;
				$_xmatch_false:
					if (*_s == 0) goto $_false;
					++_s;
				}
			}
			if (*_p == '?') // любой символ 
				goto $_match;
			if (*_p == ' ') // любое количество пробелов
			{
				if (*_s == ' ')
				{
					for (; _p[1] == ' '; ++_p);
					for (; _s[1] == ' '; ++_s);
					goto $_match;
				}
				goto $_false;
			}
			if (*_p == '[') // набор символов
			{
				for (++_p; ; )
				{
					if (*_p == ']' || *_p == 0)
						goto $_false;
					if (*_p == '\\') // управл€ющий символ
					{
						if (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')
							++_p;
					}
					if (_ignoreCase ? (ToLower(*_p) == ToLower(*_s)) : (*_p == *_s))
						break;

					if (_p[1] == '-') // диапазон
					{
						if (!_p[2])
							goto $_false;
						if (_ignoreCase)
						{
							char l = ToLower(*_s);
							char u = ToUpper(*_s);
							if (_p[0] <= l && _p[2] >= l)
								break;
							if (_p[0] >= l && _p[2] <= l)
								break;
							if (_p[0] <= u && _p[2] >= u)
								break;
							if (_p[0] >= u && _p[2] <= u)
								break;
						}
						else
						{
							if (_p[0] <= *_s && _p[2] >= *_s)
								break;
							if (_p[0] >= *_s && _p[2] <= *_s)
								break;
						}
						_p += 2;
					}
					++_p;
				}
				while (*_p != ']')
				{
					if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // управл€ющий символ
						++_p;
					if (*_p == 0)
					{
						--_p;
						break;
					}
					++_p;
				}
				goto $_match;
			}
			if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // управл€ющий символ
				++_p;
			if (_ignoreCase ? (ToLower(*_p) != ToLower(*_s)) : (*_p != *_s))
				goto $_false;

		$_match:
			++_p;
			++_s;
			continue;
		$_true:
			if (_stack.size() > 1)
			{
				_stack.pop_back();
				_s = _stack.back().first;
				_p = _stack.back().second;
				goto $_xmatch_true;
			}
			return true;
		$_false:
			if (_stack.size() > 1)
			{
				_stack.pop_back();
				_s = _stack.back().first;
				_p = _stack.back().second;
				goto $_xmatch_false;
			}
			return false;
		}
		//return false; // 4702
	}
	//----------------------------------------------------------------------------//
	const char* String::Find(const char* _str1, const char* _str2, bool _ignoreCase)
	{
		_str1 = _str1 ? _str1 : "";
		_str2 = _str2 ? _str2 : "";
		if (_ignoreCase)
		{
			for (; *_str1; *_str1++)
			{
				for (const char *a = _str1, *b = _str2; ToLower(*a++) == ToLower(*b++);)
				{
					if (!*b)
						return _str1;
				}
			}
			return nullptr;
		}
		return strstr(_str1, _str2);
	}
	//----------------------------------------------------------------------------//
	void String::Split(const char* _str, const char* _delimiters, StringArray& _dst)
	{
		if (_str)
		{
			const char* _end = _str;
			if (_delimiters)
			{
				while (*_str)
				{
					while (*_str && strchr(_delimiters, *_str))
						++_str;
					_end = _str;
					while (*_end && !strchr(_delimiters, *_end))
						++_end;
					if (_str != _end)
						_dst.push_back(String(_str, _end));
					_str = _end;
				}
			}
			else
				_end = _str + strlen(_str);
			if (_str != _end)
				_dst.push_back(String(_str, _end));
		}
	}
	//----------------------------------------------------------------------------//
	String String::ToLower(const char* _str)
	{
		if (!_str || !_str[0])
			return Empty;

		String _r = _str;
		for (char* s = _r.m_buffer->str; *s; ++s)
			*s = ToLower(*s);
		return _r;
	}
	//----------------------------------------------------------------------------//
	String String::ToUpper(const char* _str)
	{
		if (!_str || !_str[0])
			return Empty;

		String _r = _str;
		for (char* s = _r.m_buffer->str; *s; ++s)
			*s = ToUpper(*s);
		return _r;
	}
	//----------------------------------------------------------------------------//
	String::Buffer* String::_Null(void)
	{
		static Buffer _null(0, 0);
		AtomicAdd(_null.refs, 1);
		return &_null;
	}
	//----------------------------------------------------------------------------//
	String::Buffer* String::_New(const char* _str, uint _length, uint _size)
	{
		if (_length == 0 && _size == 0)
			return _Null();

		_size = _size ? _size : _length;
		Buffer* _newBuffer = new (new uint8[_size + sizeof(Buffer)]) Buffer(_size, _length);
		AtomicAdd(s_memory, _newBuffer->size);
		if (_length > 0 && _str)
			memcpy(_newBuffer->str, _str, _length);
		return _newBuffer;
	}
	//----------------------------------------------------------------------------//
	String::Buffer* String::_Unique(Buffer*& _buffer, uint _newLength, bool _quantizeMemory)
	{
		bool _isShared = AtomicAdd(_buffer->refs, 1) > 1; // shared buffer cannot be modified

		if (_newLength > _buffer->size || _isShared)
		{
			Buffer* _oldBuffer = _buffer;
			_buffer = _New(_buffer->str, _buffer->length, (_newLength > _buffer->size && _quantizeMemory) ? (_newLength << 1) : _newLength);
			_Release(_oldBuffer);
		}
		else
			_Release(_buffer);

		return _buffer;
	}
	//----------------------------------------------------------------------------//
	String::Buffer* String::_AddRef(Buffer* _buffer)
	{
		AtomicAdd(_buffer->refs, 1);
		return _buffer;
	}
	//----------------------------------------------------------------------------//
	void String::_Release(Buffer* _buffer, int _refs)
	{
		ASSERT(_refs < 0);
		if (AtomicAdd(_buffer->refs, _refs) == 0)
		{
			AtomicAdd(s_memory, -int(_buffer->size));
			delete[] reinterpret_cast<uint8*>(_buffer);
		}
	}
	//----------------------------------------------------------------------------//
	uint String::_Length(const char* _str, int _length)
	{
		return _str ? (_length < 0 ? (uint)strlen(_str) : _length) : 0;
	}
	//----------------------------------------------------------------------------//
	uint16 String::DecodeUtf8(const char*& _utf8)
	{
		uint8 c = *_utf8 ? *_utf8++ : 0;
		if (c > 0x7f)
		{
			uint8 c2 = *_utf8++;
			if (c & 0x20) // 0x0800..0xffff 
			{
				uint8 c3 = *_utf8++;
				return ((c & 0xf) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);
			}
			return ((c & 0x1f) << 6) | (c2 & 0x3f); // 0x80..0x07ff
		}
		return c;
	}
	//----------------------------------------------------------------------------//
	/*WString String::DecodeUtf8(const String& _utf8)
	{
	WString _ucs2;
	const char* s = _utf8.c_str();
	if (s) while (*s)
	_ucs2 += DecodeUtf8(s);
	return _ucs2;
	}*/
	//----------------------------------------------------------------------------//
	void String::EncodeUtf8(String& _outUtf8, const wchar_t _inUcs2)
	{
		if (_inUcs2 <= 0x7f) // 0xxxxxxx
		{
			_outUtf8 += (char)(_inUcs2);
		}
		else if (_inUcs2 < 0x7ff)	// 110xxxxx 10xxxxxx
		{
			_outUtf8 += (char)(0xc0 | (0x1f & (_inUcs2 >> 6)));
			_outUtf8 += (char)(0x80 | (0x3f & _inUcs2));
		}
		else if (_inUcs2 < 0xffff) // 1110xxxx 10xxxxxx 10xxxxxx
		{
			_outUtf8 += (char)(0xe0 | (0xf & (_inUcs2 >> 12)));
			_outUtf8 += (char)(0x80 | (0x3f & (_inUcs2 >> 6)));
			_outUtf8 += (char)(0x80 | (0x3f & _inUcs2));
		}
		/*else if (_code < 0x10ffff)
		{
		_outUtf8 += (char)(0xf0 | (0x7 & (_inUcs2 >> 18)));
		_outUtf8 += (char)(0x80 | (0x3f & (_inUcs2 >> 12)));
		_outUtf8 += (char)(0x80 | (0x3f & (_inUcs2 >> 6)));
		_outUtf8 += (char)(0x80 | (0x3f & _inUcs2));
		}*/
	}
	//----------------------------------------------------------------------------//
	String String::EncodeUtf8(const wchar_t* _ucs2)
	{
		String _utf8;
		const wchar_t* s = _ucs2;
		if (s) while (*s)
			EncodeUtf8(_utf8, *s++);
		return _utf8;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Name
	//----------------------------------------------------------------------------//

	const Name::Item Name::s_empty("");
	HashMap<uint, const Name::Item*> Name::s_names = { { 0, &s_empty } };
	int Name::s_mutex;

	//----------------------------------------------------------------------------//
	const Name::Item* Name::_AddItem(const String& _name)
	{
		if (_name.IsEmpty())
			return nullptr;

		uint _hash = _name.Hash(_name);

		AtomicLock(s_mutex);

		auto _it = s_names.find(_hash);
		if (_it != s_names.end())
		{
			AtomicUnlock(s_mutex);
			return _it->second;
		}

		Item* _item = new Item(_name);
		s_names[_hash] = _item;

		AtomicUnlock(s_mutex);

		return _item;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// CharStream
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	void CharStream::Next(uint _c)
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
	//----------------------------------------------------------------------------//
	uint CharStream::CharStream::Skip(const char* _cset, bool _op)
	{
		ASSERT(_cset != nullptr);
		const char* _start = s;
		while (*s && (strchr(_cset, *s) != 0) == _op)
			Next();
		return (uint)(s - _start);
	}
	//----------------------------------------------------------------------------//
	bool CharStream::AnyOf(const char* _cset, bool _op)
	{
		ASSERT(_cset != nullptr);
		return *s && (strchr(_cset, *s) != 0) == _op;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Checksum
	//----------------------------------------------------------------------------//

	// COPYRIGHT (C) 1986 Gary S. Brown. http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/libkern/crc32.c
	const uint32 Crc32Table[256] =
	{
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	//----------------------------------------------------------------------------//
	// ConfigParser
	//----------------------------------------------------------------------------//

	// (*EBNF*)
	// CR = ? US - ASCII character 13 ? ;
	// LF = ? US - ASCII character 10 ? ;
	// NewLine = (CR, LF) | CR | LF;
	// AllSymbols = ? all visible characters ? ;
	// ControlChars = ':' | '=' | ',' | ';' | '{' | '}' | '"' | "'" | '[' | ']';
	// Divisor = ',' | ';';
	// LineComment = "//", { AllSymbols - NewLine }, NewLine;
	// MultilineComment = "/*", { AllSymbols }, "*/";
	// Comment = LineComment | MultilineComment;
	// WhiteSpace = ? US - ASCII character 32 ? | ? US - ASCII character 9 ? ;
	// Space = { WhiteSpace | NewLine | Comment };
	// CString = '"', { AllSymbols - ('"' | NewLine) }, '"'; (*C-Like strings with escape sequences*)
	// VString = "'", { AllSymbols - "'" }, "'"; (*symbol "'" can be written as "\'" *)
	// String = CString | VString;
	// Bool = "true" | "false"; (*ignore case*)
	// Null = "null"; (*ignore case*)
	// Identifier = { AllSymbols - (WhiteSpace | NewLine | ControlChars | Comment) };
	// Rhs = [Space], (Object | Array | Identifier | String | Number | Bool | Null); (*value*)
	// Name = [Space], (Identifier | String);
	// Type = Name, [Space], ':';
	// TypeName = (Type, Name) | Name | Type;
	// Lhs = TypeName, [Space], '=';
	// Expr = [Lhs], Rhs;
	// Body = { [{Expr}, [Divisor]], };
	// Array = [Space], '[', Body, [Space], ']';
	// Object = [Space], '{', Body, [Space], '}';	(*associative array*)
	// Document = Object;
	// DefaultName = '__unnamed'; (*default name for element in associative arrays*) 

	struct ConfigParser
	{
		ConfigParser() : s("") { }

		bool Parse(Config& _dst, const String& _src, String* _errorString, int* _errorLine)
		{
			s = CharStream(_src);
			if (ParseBody(_dst, 0))
				return true;
			if (_errorString)
				*_errorString = error;
			if (_errorLine)
				*_errorLine = s.l;
			return false;
		}

	protected:

		CharStream s;
		String error;

		void SkipSpace(void)
		{
			const char* _start;
			do
			{
				_start = s;

				s.Skip(" \t\n\r"); // white spaces and new lines

				if (s[0] == '/' && s[1] == '/')	// line comment
				{
					s += 2;
					s.Skip("\n\r", false);
				}
				else if (s[0] == '*' && s[1] == '/') // multiline comment
				{
					s += 2;
					while (*s && !(s[0] == '*' && s[1] == '/'))
						++s;
					s += 2;
				}

			} while (*s && _start != s);
		}
		bool SkipDivisor(void)
		{
			bool _skipped = false;
			SkipSpace();
			if (s.AnyOf(",;"))
			{
				++s;
				_skipped = true;
			}
			SkipSpace();
			return _skipped;
		}

		bool IsSpace(void) { return s.AnyOf(" \t\n\r") || (s[0] == '/' && (s[1] == '/' || s[1] == '*')); }
		bool IsControl(void) { return s.AnyOf(":=,;{}[]\"'"); }
		bool IsString(void) { return s.AnyOf("\"'"); }
		bool IsIdentifier(void) { return *s && !IsSpace() && !IsControl(); }
		bool IsName(void) { return IsString() || IsIdentifier(); }
		bool IsDivisor(void) { return s.AnyOf(",;"); }

		enum
		{
			R_Error = -1,
			R_False = 0,
			R_True = 1,
		};

		bool Expected(char _ch)
		{
			if (*s != _ch)
			{
				error = "Expected '";
				error += _ch;
				error += "' not was found";
				return false;
			}
			++s;
			return true;
		}

		bool ParseString(String& _value)
		{
			ASSERT(IsString());
			if (*s == '"') // C-Like string
			{
				++s;
				while (*s)
				{
					if (*s == '\\') // escape sequence
					{
						if (s[1] == 'n')
							_value += '\n';
						else if (s[1] == 'r')
							_value += '\r';
						else if (s[1] == 't')
							_value += '\t';
						else if (s[1] == '\'')
							_value += '\'';
						else if (s[1] == '\"')
							_value += '\"';
						else if (s[1] == '\\')
							_value += '\\';
						else
						{
							error = "Unknown escape sequence";
							return false;
						}
						s += 2;
					}
					else if (*s == '"') // end of string
					{
						++s;
						break;
					}
					else if (strchr("\n\r", *s)) // new line
					{
						error = "New line in string";
						return false;
					}
					else // other
						_value += *s++;
				}
				return true;
			}
			else if (*s == '\'') // verbatim string
			{
				++s;
				while (*s)
				{
					if (s[0] == '\\' && s[1] == '\'') // apostrophe
					{
						_value += '\'';
						s += 2;
					}
					else if (*s == '\n' || *s == '\r') // new line
					{
						_value += '\n';
						++s;
					}
					else if (*s == '\'') // end of string
					{
						++s;
						break;
					}
					else // other
						_value += *s++;
				}
				return true;
			}
			return true;
		}

		bool ParseIdentifier(String& _value)
		{
			ASSERT(IsIdentifier());
			while (IsIdentifier())
				_value += *s++;
			return true;
		}

		bool ParseName(String& _value)
		{
			ASSERT(IsName());
			return IsString() ? ParseString(_value) : ParseIdentifier(_value);
		}

		int ParseLhs(String& _name, String& _type)
		{
			CharStream _start = s;

			// read name or type
			SkipSpace();
			if (IsName() && !ParseName(_name))
				return R_Error;
			SkipSpace();

			// possibly is this value?
			if (IsDivisor())
			{
				s = _start;
				_name.Clear();
				return R_False;
			}

			// it was a type. read name ...
			if (*s == ':')
			{
				_type = _name;
				_name.Clear();
				++s;
				SkipSpace();
				if (IsName() && !ParseName(_name))
					return R_Error;

				SkipSpace();
				return Expected('=') ? R_True : R_Error;
			}

			// possibly is this value?
			if (*s == '=')
			{
				++s;
				return R_True;
			}

			// it really a value
			_name.Clear();
			s = _start;
			return R_False;
		}

		bool ParseRhs(Config& _value, char _end)
		{
			SkipSpace();
			if (*s == '{')
			{
				++s;
				if (!ParseBody(_value, '}'))
					return false;

				if (!Expected('}'))
					return false;
			}
			else if (*s == '[')
			{
				++s;
				if (!ParseBody(_value, ']'))
					return false;

				if (!Expected(']'))
					return false;
			}
			else if (IsString())
			{
				String _str;
				if (!ParseString(_str))
					return false;
				_value = _str;
			}
			else if (IsIdentifier())
			{
				String _str;
				if (!ParseIdentifier(_str))
					return false;

				if (!stricmp(_str, "true"))
					_value = true;
				else if (!stricmp(_str, "false"))
					_value = true;
				else if (!stricmp(_str, "null"))
					_value.SetNull();
				else
				{
					_value = _str;

					// parse number
					if (strchr("+-0123456789", _str[0]))
					{
						char* _endp = nullptr;
						double _num = strtod(_str, &_endp);
						if (_endp == _str.Ptr(_str.Length()))
							_value = _num;
					}
				}
			}
			else if (*s != _end)
			{
				if (!*s)
					error = "Expected %value% not was found";
				else
				{
					error = "Unexpected token '";
					error += *s;
					error += "'";
				}
				return false;
			}

			SkipDivisor();
			return true;
		}

		bool ParseExpr(String& _name, String& _type, Config& _value, char _end)
		{
			if (ParseLhs(_name, _type) == R_Error)
				return false;

			return ParseRhs(_value, _end);
		}

		bool ParseBody(Config& _node, char _end)
		{
			Config _child;
			String _name, _type;
			for (SkipSpace(); *s && *s != _end; SkipSpace())
			{
				_child.SetNull();
				_name.Clear();
				_type.Clear();

				if (!ParseExpr(_name, _type, _child, _end))
					return false;

				if (_end == ']')
					_node.Append(_name, _type) = Move(_child);
				else
					_node.Add(_name, _type) = Move(_child);
			}
			return true;
		}
	};

	//----------------------------------------------------------------------------//
	// ConfigPrinter
	//----------------------------------------------------------------------------//

	struct ConfigPrinter
	{
		static void PrintRoot(String& _dst, const Config& _src)
		{
			if (_src.IsNode())
				PrintNode(_dst, _src, 0, true);
			else if (!_src.IsNull())
				PrintRhs(_dst, _src, 0);
		}

	protected:

		static void PrintRhs(String& _dst, const Config& _src, uint _depth)
		{
			switch (_src.Type())
			{
			case CT_Null:
				_dst += "null";
				break;

			case CT_Bool:
				_dst += _src.AsBool() ? "true" : "false";
				break;

			case CT_Number:
			{
				char _buff[64];
				gcvt(_src.AsNumber(), 10, _buff);
				char* _end = _buff + strlen(_buff) - 1;
				if (*_end == '.')
					*_end = 0;
				_dst += _buff;

			} break;

			case CT_String:
				PrintString(_dst, _src.AsString(), true);
				break;

			case CT_Array:
			case CT_Object:
			{

				bool _isMultiline = _src.Size() > 4;
				for (uint i = 0, s = _src.Size(); !_isMultiline && i < s; ++i)
				{
					_isMultiline |= _src[i].IsNode() || _src[i].IsString() || _src.Type(i).Length() > 20 || _src.Name(i).Length() > 20;
				}

				_dst += _src.IsArray() ? "[ " : "{ ";
				if (_src.Size() && _isMultiline)
					_dst += '\n';

				PrintNode(_dst, _src, _depth + 1, _isMultiline);

				if (_src.Size() && _isMultiline)
					PrintOffset(_dst, _depth);
				_dst += _src.IsArray() ? "]" : "}";

			} break;

			}
		}

		static void PrintNode(String& _dst, const Config& _src, uint _depth, bool _isMultiline)
		{
			for (uint i = 0, s = _src.Size(); i < s; ++i)
			{
				if (_isMultiline)
					PrintOffset(_dst, _depth);
				PrintLhs(_dst, _src.Name(i), _src.Type(i));
				PrintRhs(_dst, _src[i], _depth);
				if (!_isMultiline && i + 1 < s)
					_dst += ',';
				//	_dst += _isMultiline ? ';' : ',';
				_dst += ' ';
				if (_isMultiline)
					_dst += '\n';
			}
		}

		static void PrintOffset(String& _dst, uint _depth)
		{
			_dst.Append(_depth, '\t');
		}

		static void PrintString(String& _dst, const char* _str, bool _value)
		{
			if (!_str || !*_str)
			{
				if (_value)
					_dst += "''"; // empty string
				return;
			}

			bool _hasNewLines = strchr(_str, '\n') ||
				strchr(_str, '\r');
			bool _hasVStringChar = strchr(_str, '\'') != nullptr;
			bool _asString =
				_hasNewLines ||
				_hasVStringChar ||
				strchr(_str, ':') ||
				strchr(_str, '=') ||
				strchr(_str, ',') ||
				strchr(_str, ';') ||
				strchr(_str, '{') ||
				strchr(_str, '}') ||
				strchr(_str, '[') ||
				strchr(_str, ']') ||
				strchr(_str, ' ') ||
				strchr(_str, '\t') ||
				strstr(_str, "//") ||
				strstr(_str, "/*") ||
				strstr(_str, "\"");

			if (_asString)
			{
				if ((_hasNewLines && strlen(_str) > 80) || (!_hasNewLines && !_hasVStringChar && !strchr(_str, '\\'))) // verbatim string
				{
					_dst += '\'';
					for (const char* s = _str; *s; ++s)
					{
						if (*s == '\'')
							_dst += "\\'";
						else if (*s == '\n' || *s == '\r')
						{
							if (s[0] == '\r' && s[1] == '\n')
								++s;
							_dst += '\n';
						}
						else
							_dst += *s;
					}
					_dst += '\'';
				}
				else // c-like string
				{
					_dst += '\"';
					for (const char* s = _str; *s; ++s)
					{
						if (*s == '\"')
							_dst += "\\\"";
						else if (*s == '\\')
							_dst += "\\\\";
						else if (*s == '\n' || *s == '\r')
						{
							if (s[0] == '\r' && s[1] == '\n')
								++s;
							_dst += "\\n";
						}
						else
							_dst += *s;
					}
					_dst += '\"';
				}
			}
			else
				_dst += _str;
		}

		static void PrintLhs(String& _dst, const String& _name, const String& _type)
		{
			if (_type.NonEmpty())
			{
				PrintString(_dst, _type, false);
				_dst += ':';
			}

			if (_name.NonEmpty())
				PrintString(_dst, _name, false);

			if (_type.NonEmpty() || _name.NonEmpty())
				_dst += " = ";
		}
	};

	//----------------------------------------------------------------------------//
	// Config
	//----------------------------------------------------------------------------//

	const Config Config::Null;
	const String Config::Unnamed = "__unnamed";

	//----------------------------------------------------------------------------//
	Config::~Config(void)
	{
		if (m_type == CT_String)
			delete m_str;
		else if (m_type >= CT_Array)
			delete m_node;
	}
	//----------------------------------------------------------------------------//
	Config::Config(const Config& _other) :
		m_type(_other.m_type),
		m_raw(_other.m_raw)
	{
		if (m_type == CT_String)
			m_str = new String(*_other.m_str);
		else if (m_type >= CT_Array)
			m_node = new Node(*_other.m_node);
	}
	//----------------------------------------------------------------------------//
	Config::Config(Config&& _temp) :
		m_type(_temp.m_type),
		m_raw(_temp.m_raw)
	{
		_temp.m_type = CT_Null;
		_temp.m_raw = 0;
	}
	//----------------------------------------------------------------------------//
	Config& Config::operator = (const Config& _other)
	{
		if (m_type != _other.m_type)
		{
			if (m_type == CT_String)
				delete m_str;
			else if (m_type >= CT_Array)
				delete m_node;

			m_type = _other.m_type;

			if (m_type == CT_String)
				m_str = new String(*_other.m_str);
			else if (m_type >= CT_Array)
				m_node = new Node(*_other.m_node);
			else
				m_raw = _other.m_raw;
		}
		else
		{
			if (m_type == CT_String)
				*m_str = *_other.m_str;
			else if (m_type >= CT_Array)
				*m_node = *_other.m_node;
			else
				m_raw = _other.m_raw;
		}
		return *this;
	}
	//----------------------------------------------------------------------------//
	Config& Config::operator = (Config&& _temp)
	{
		Swap(m_type, _temp.m_type);
		Swap(m_raw, _temp.m_raw);
		return *this;
	}
	//----------------------------------------------------------------------------//
	Config* Config::Search(const String& _name, uint* _index) const
	{
		if (m_type == CT_Object)
		{
			auto _it = m_node->indices.find(_name.IsEmpty() ? Unnamed : _name);
			if (_it != m_node->indices.end())
			{
				if (_index)
					*_index = _it->second;
				return &m_node->childs[_it->second];
			}
		}
		else if (m_type == CT_Array && _name.NonEmpty())
		{
			for (auto i = m_node->names.begin(), e = m_node->names.end(); i != e; ++i)
			{
				if (i->second.first == _name)
				{
					if (_index)
						*_index = i->first;
					return &m_node->childs[i->first];
				}
			}
		}
		return nullptr;
	}
	//----------------------------------------------------------------------------//
	const Config& Config::Child(const String& _name) const
	{
		Config* _child = Search(_name);
		return _child ? *_child : Null;
	}
	//----------------------------------------------------------------------------//
	const Config& Config::Child(uint _index) const
	{
		if (m_type >= CT_Array && _index < (uint)m_node->childs.size())
			return m_node->childs[_index];
		return Null;
	}
	//----------------------------------------------------------------------------//
	const String& Config::Name(uint _index) const
	{
		if (m_type >= CT_Array && _index < (uint)m_node->childs.size())
			return m_node->names[_index].first;
		return String::Empty;
	}
	//----------------------------------------------------------------------------//
	const String& Config::Type(const String& _name) const
	{
		uint _index;
		Config* _child = Search(_name, &_index);
		return _child ? m_node->names[_index].second : String::Empty;
	}
	//----------------------------------------------------------------------------//
	const String& Config::Type(uint _index) const
	{
		if (m_type >= CT_Array && _index < (uint)m_node->childs.size())
			return m_node->names[_index].second;
		return String::Empty;
	}
	//----------------------------------------------------------------------------//
	Config& Config::Add(const String& _name, const String& _type)
	{
		uint _index = 0;

		if (m_type == CT_Array)
		{
			// search existent object with name
			if (_name.NonEmpty())
			{
				Config* _exists = Search(_name, &_index);
				if (_exists)
				{
					// set new type
					String& _ctype = m_node->names[_index].second;
					if (_ctype.IsEmpty())
						_ctype = _type;

					return *_exists;
				}
			}

			// add new child to an array
			_index = (uint)m_node->childs.size();
			if (_name.NonEmpty() || _type.NonEmpty())
				m_node->names[_index] = std::pair<String, String>(_name, _type);
			m_node->childs.push_back(Config());

			return m_node->childs.back();
		}

		// convert this to object
		if (m_type != CT_Object)
		{
			if (m_type == CT_String)
				delete m_str;

			m_type = CT_Object;
			m_node = new Node;

		}

		// search existent child with name
		Config* _exists = Search(_name.IsEmpty() ? Unnamed : _name, &_index);
		if (_exists)
		{
			// set new type
			String& _ctype = m_node->names[_index].second;
			if (_ctype.IsEmpty())
				_ctype = _type;

			return *_exists;
		}

		// add new child at index
		_index = (uint)m_node->childs.size();
		if (_name.IsEmpty())
		{
			m_node->names[_index] = std::pair<String, String>(Unnamed, _type);
			m_node->indices[Unnamed] = _index;
		}
		else
		{
			m_node->names[_index] = std::pair<String, String>(_name, _type);
			m_node->indices[_name] = _index;
		}
		m_node->childs.push_back(Config());

		return m_node->childs.back();
	}
	//----------------------------------------------------------------------------//
	Config& Config::Append(const String& _name, const String& _type)
	{
		if (m_type != CT_Array)
		{
			if (m_type == CT_Object)
				m_node->indices.clear();
			else
			{
				if (m_type == CT_String)
					delete m_str;
				m_node = new Node;
			}

			m_type = CT_Array;
		}

		if (_name.NonEmpty() || _type.NonEmpty())
			m_node->names[(uint)m_node->childs.size()] = std::pair<String, String>(_name, _type);
		m_node->childs.push_back(Config());

		return m_node->childs.back();
	}
	//----------------------------------------------------------------------------//
	Config& Config::SetNull(void)
	{
		if (m_type == CT_String)
			delete m_str;
		else if (m_type >= CT_Array)
			delete m_node;
		m_type = CT_Null;
		m_raw = 0;
		return *this;
	}
	//----------------------------------------------------------------------------//
	Config& Config::Clear(void)
	{
		if (m_type == CT_String)
			m_str->Clear();
		else if (m_type >= CT_Array)
		{
			m_node->childs.clear();
			m_node->names.clear();
			if (m_type == CT_Object)
				m_node->indices.clear();
		}
		else
			m_raw = 0;
		return *this;
	}
	//----------------------------------------------------------------------------//
	bool Config::Parse(const String& _str, String* _errorString, int* _errorLine)
	{
		ConfigParser _parser;
		return _parser.Parse(*this, _str, _errorString, _errorLine);
	}
	//----------------------------------------------------------------------------//
	String Config::Print(void) const
	{
		String _str;
		ConfigPrinter::PrintRoot(_str, *this);
		return _str;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
