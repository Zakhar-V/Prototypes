#include "String.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// String 
	//----------------------------------------------------------------------------//

	TODO_EX("String", "add ToLower, ToUpper, Compare, Match, Equals, UTF8, Iterators");
	TODO_EX("String", "Standalone implementation (not use std::string)");

	const String String::Empty;

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
				_res.append(p, e - p);
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
				_res.append(buf, s);

			p = e;
		}
		return _res;
#endif
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
	uint32 String::NameHash(const char* _str, uint32 _hash)
	{
		if (_str)
		{
			char _lwr[2] = { 0, 0 };
			while (*_str)
			{
				_lwr[0] = *_str++;
				_hash = *strlwr(_lwr) + (_hash << 6) + (_hash << 16) - _hash;
			}
		}
		return _hash;
	}
	//----------------------------------------------------------------------------//
	int String::Compare(const char* _a, const char* _b, bool _ignoreCase)
	{
		return _ignoreCase ? stricmp(_a ? _a : "", _b ? _b : "") : strcmp(_a ? _a : "", _b ? _b : "");
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//  
	//----------------------------------------------------------------------------//
}
