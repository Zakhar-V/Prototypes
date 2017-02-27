#include "Lib.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// String utils
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	String StrFormat(const char* _fmt, ...)
	{
		va_list _args;
		va_start(_args, _fmt);
		String _str = StrFormatV(_fmt, _args);
		va_end(_args);
		return _str;
	}
	//----------------------------------------------------------------------------//
	String StrFormatV(const char* _fmt, va_list _args)
	{
#if 0
		char _buff[4096];
		vsnprintf_s(_buff, sizeof(_buff), _fmt, _args);
		return _buff;
#else
		//this code from ptypes: pputf.cxx
		struct { bool operator () (char _ch) const { return strchr(" #+-~.0123456789", _ch) != 0; } } _check_fmtopts;
		enum fmt_type_t { FMT_NONE, FMT_CHAR, FMT_SHORT, FMT_INT, FMT_LONG, FMT_LARGE, FMT_STR, FMT_PTR, FMT_DOUBLE, FMT_LONG_DOUBLE };

		String _res;
		fmt_type_t fmt_type;
		const char *e, *p = _fmt;
		char buf[4096], fbuf[128];
		while (p && *p != 0)
		{
			// write out raw data between format specifiers
			e = strchr(p, '%');
			if (e == 0) e = p + strlen(p);
			if (e > p) _res.append(p, e - p);
			if (*e != '%') break;
			if (*++e == '%') { _res += '%', p = e + 1; continue; }  // write out a single '%'

			// build a temporary buffer for the conversion specification
			fbuf[0] = '%';
			char* f = fbuf + 1;
			bool modif = false;

			// formatting flags and width specifiers
			while (_check_fmtopts(*e) && uint(f - fbuf) < sizeof(fbuf) - 5)	*f++ = *e++, modif = true;

			// prefixes
			switch (*e)
			{
			case 'h': fmt_type = FMT_SHORT; *f++ = *e++; break;
			case 'L': fmt_type = FMT_LONG_DOUBLE; *f++ = *e++; break;
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
			default: fmt_type = FMT_NONE; break;
			};

			// format specifier
			switch (*e)
			{
			case 'c': fmt_type = FMT_CHAR; *f++ = *e++; break;
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				if (fmt_type < FMT_SHORT || fmt_type > FMT_LARGE) fmt_type = FMT_INT;
				*f++ = *e++;
				break;
			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':
				if (fmt_type != FMT_LONG_DOUBLE) fmt_type = FMT_DOUBLE;
				*f++ = *e++;
				break;
			case 's': fmt_type = FMT_STR; *f++ = *e++; break;
			case 'p': fmt_type = FMT_PTR; *f++ = *e++; break;
			};
			if (fmt_type == FMT_NONE) break;
			*f = 0;

			// some formatters are processed here 'manually', while others are passed to snprintf
			int s = 0;
			switch (fmt_type)
			{
			case FMT_NONE: break; // to avoid compiler warning
			case FMT_CHAR:
				if (modif) s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
				else _res += (char)(va_arg(_args, int));
				break;
			case FMT_SHORT: s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int)); break;
			case FMT_INT: s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int)); break;
			case FMT_LONG: s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long)); break;
			case FMT_LARGE: s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, __int64)); break;
			case FMT_STR:
			{
				if (modif) s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, const char*));
				else
				{
					const char* _str = va_arg(_args, const char*);
					if (_str) _res += _str;
					//else _res += "<null>";
				}

			}break;
			case FMT_PTR: s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, void*)); break;
			case FMT_DOUBLE:
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, double));
				for (; s > 0; --s)
				{
					if (buf[s - 1] == '0') buf[s - 1] = 0;
					else if (buf[s - 1] == '.') { buf[--s] = 0; break; }
					else break;
				}
				break;
			case FMT_LONG_DOUBLE: s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long double)); break;
			}
			if (s > 0) _res.append(buf, s);
			p = e;
		}
		return _res;
#endif
	}
	//----------------------------------------------------------------------------//
	String StrUpper(const char* _str)
	{
		String _r = _str ? _str : STR_BLANK;
		if (!_r.empty())
		{
#		ifdef WIN32
			CharUpperBuffA(const_cast<char*>(_r.c_str()), (uint)_r.length());
#		else
			strupr(const_cast<char*>(_r.c_str()));
#		endif
		}
		return _r;
	}
	//----------------------------------------------------------------------------//
	String StrLower(const char* _str)
	{
		String _r = _str ? _str : STR_BLANK;
		if (!_r.empty())
		{
#		ifdef WIN32
			CharLowerBuffA(const_cast<char*>(_r.c_str()), (uint)_r.length());
#		else
			strlwr(const_cast<char*>(_r.c_str()));
#		endif
		}
		return _r;
	}
	//----------------------------------------------------------------------------//
	void SplitString(const char* _str, const char* _delimiters, StringArray& _dst)
	{
		if (_str)
		{
			const char* _end = _str;
			if (_delimiters)
			{
				while (*_str)
				{
					while (*_str && strchr(_delimiters, *_str)) ++_str;
					_end = _str;
					while (*_end && !strchr(_delimiters, *_end)) ++_end;
					if (_str != _end) _dst.push_back(String(_str, _end));
					_str = _end;
				}
			}
			else _end = _str + strlen(_str);
			if(_str != _end) _dst.push_back(String(_str, _end));
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// CheckSum
	//----------------------------------------------------------------------------//

	// COPYRIGHT (C) 1986 Gary S. Brown. http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/libkern/crc32.c
	const uint32 CRC32Table[256] =
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
	// Path utils
	//----------------------------------------------------------------------------//

	namespace
	{
		static const String p1 = ".";
		static const String p2 = "..";
	}

	//----------------------------------------------------------------------------//
	bool IsFullPath(const String& _path, uint* _dp)
	{
#	if defined(WIN32)
		uint _p = (uint)_path.find(':');
		if (_dp) *_dp = _p;
		return _p != String::npos;
#	elif defined(LINUX)
		uint _p1 = _path.find('/');
		uint _p2 = _path.find('\\');
		if (_p1 == 0 || _p2 == 0)
		{
			if (_dp)
			{
				_p1 = _path.find('/', 1);
				_p2 = _path.find('\\', 1);
				if (_p1 < _p2) *_dp = _p1;
				else if (_p1 > _p2) *_dp = _p2;
				else *_dp == _path.length();
			}
			return true;
		}
		return false;
#	else
		NOT_IMPLEMENTED_YET();
#	endif
	}
	//----------------------------------------------------------------------------//
	void SplitFilename(const String& _filename, String* _device, String* _dir, String* _name, String* _shortname, String* _ext)
	{
		uint _l = (uint)_filename.length();
		if (_device)
		{
			uint _dp;
#		if defined(WIN32)
			*_device = IsFullPath(_filename, &_dp) ? _filename.substr(0, _dp) : STR_BLANK;
#		elif defined(LINUX)
			*_device = IsFullPath(_filename, &_dp) ? _filename.substr(1, _dp) : STR_BLANK;
#		else
			NOT_IMPLEMENTED_YET();
#		endif
		}
		if (_dir)
		{
			uint c = _l;
			for (; c > 0 && strchr("/\\", _filename[c - 1]); --c);
			for (; c > 0 && !strchr("/\\", _filename[c - 1]); --c);
			*_dir = _filename.substr(0, c);
		}
		if (_name || _ext || _shortname)
		{
			uint _np = _l;
			for (; _np > 0 && !strchr("/\\", _filename[_np - 1]); --_np);
			String _n = _filename.substr(_np, String::npos);
			if (_name) *_name = _n;
			if (_ext || _shortname)
			{
				uint _pp = _l - _np;
				for (; _pp > 0 && _n[_pp] != '.'; --_pp);
				if (_shortname) *_shortname = _pp > 0 ? _n.substr(0, _pp) : _n;
				if (_ext) *_ext = _pp > 0 ? _n.substr(_pp + 1, String::npos) : "";
			}
		}
	}
	//----------------------------------------------------------------------------//
	void SplitPath(const String& _path, String* _device, StringArray* _items)
	{
#	ifndef WIN32
		NOT_IMPLEMENTED_YET();
#	endif
		if (_path.empty()) return;

		const char* b = _path.c_str();
		const char* p = _path.c_str();
		const char* c = strchr(_path.c_str(), ':');

		if (c)
		{
			if (_device) *_device = _path.substr(0, c - p);
			p = ++c;
		}
		else c = b;

		if (_items)
		{
			_items->clear();
			_items->reserve(10);
			String _item;
			for (; *c; ++c)
			{
				if (strchr("\\/", *c))
				{
					if (c != p)
					{
						_item = _path.substr(p - b, c - p);
						if (_item == p2)
						{
							if (_items->empty() || _items->back() == p2) _items->push_back(p2);
							else _items->pop_back();
						}
						else if (_item != p1) _items->push_back(_item);
					}
					p = c + 1;
				}
			}
			if (c != p)
			{
				_item = _path.substr(p - b, c - p);
				if (_item == p2)
				{
					if (_items->empty() || _items->back() == p2) _items->push_back(p2);
					else _items->pop_back();
				}
				else if (_item != p1) _items->push_back(_item);
			}
		}
	}
	//----------------------------------------------------------------------------//
	String MakeFilename(const String& _name, const String& _ext)
	{
		String _fname = _name;
		if (!_ext.empty()) _fname += '.' + _ext;
		return _fname;
	}
	//----------------------------------------------------------------------------//
	String MakePath(const String& _device, const StringArray& _items)
	{
#	ifndef WIN32
		NOT_IMPLEMENTED_YET();
#	endif
		String _path;
		if (!_device.empty()) _path += _device + ":/";
		if (_items.empty()) _path += ".";
		for (uint i = 0; i < _items.size(); ++i)
		{
			if (i == 0) _path += _items[i];
			else _path += "/" + _items[i];
		}
		return _path;
	}
	//----------------------------------------------------------------------------//
	String MakeFullPath(const String& _path, const String& _root)
	{
#	ifndef WIN32
		NOT_IMPLEMENTED_YET();
#	endif
		if (_path.empty()) return STR_BLANK;

		String _d, _pd;
		StringArray _p, _f;

		SplitPath(_path, &_pd, &_p);
		if (_path.find(':') != String::npos) return MakePath(_pd, _p);

		SplitPath(_root, &_d, &_f);

		for (uint i = 0; i < _p.size(); ++i)
		{
			if (_p[i] == p2)
			{
				if (_f.empty() || _f.back() == p2) _f.push_back(p2);
				else _f.pop_back();
			}
			else if (_p[i] != p1) _f.push_back(_p[i]);
		}
		return MakePath(_d, _f);
	}
	//----------------------------------------------------------------------------//
	String MakeNormPath(const String& _path, bool _toLower)
	{
#	ifndef WIN32
		NOT_IMPLEMENTED_YET();
#	endif
		String _npath = _path;
		if (!_path.empty())
		{
			if (strchr(_path.c_str(), '\\') || strstr(_path.c_str(), "..") || strstr(_path.c_str(), ".\\") || strstr(_path.c_str(), "./") || strstr(_path.c_str(), "\\.") || strstr(_path.c_str(), "/."))
			{
				_npath = MakeFullPath(_path);
			}
			if (_toLower)
			{
				_npath = StrLower(_npath);
			}
		}
		return _npath;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Quat
	//----------------------------------------------------------------------------//

	const Quat Quat::ZERO(0, 0, 0, 0);
	const Quat Quat::IDENTITY(0, 0, 0, 1);

	//----------------------------------------------------------------------------//
	// Mat34
	//----------------------------------------------------------------------------//

	const Mat34 Mat34::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	const Mat34 Mat34::IDENTITY(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0);

	//----------------------------------------------------------------------------//
	// Mat44
	//----------------------------------------------------------------------------//

	const Mat44 Mat44::ZERO(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	const Mat44 Mat44::IDENTITY(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

	//----------------------------------------------------------------------------//
	// Plane
	//----------------------------------------------------------------------------//

	const Plane Plane::ZERO(Vec3::ZERO, 0);

	//----------------------------------------------------------------------------//
	// AlignedBox
	//----------------------------------------------------------------------------//

	const AlignedBox AlignedBox::ZERO(Vec3::ZERO, Vec3::ZERO);
	const AlignedBox AlignedBox::INF(999999.9f, -999999.9f);
	const uint16 AlignedBox::LINES[24] = { 0, 1, 1, 2, 2, 3, 3, 0, 3, 5, 2, 4, 1, 7, 0, 6, 4, 5, 5, 6, 6, 7, 7, 4 };
	const uint16 AlignedBox::QUADS[24] = { 5, 3, 2, 4, 4, 2, 1, 7, 7, 1, 0, 6, 6, 0, 3, 5, 6, 5, 4, 7, 2, 3, 0, 1 };
	const uint16 AlignedBox::TRIANGLES[36] = { 5, 3, 2, 5, 2, 4, 4, 2, 1, 4, 1, 7, 7, 1, 0, 7, 0, 6, 6, 0, 3, 6, 3, 5, 6, 5, 4, 6, 4, 7, 3, 0, 1, 3, 1, 2 };

	//----------------------------------------------------------------------------//
	// Frustum
	//----------------------------------------------------------------------------//

	const uint16 Frustum::LINES[24] = { 0, 1, 1, 2, 2, 3, 3, 0, 3, 5, 2, 4, 1, 7, 0, 6, 4, 5, 5, 6, 6, 7, 7, 4 };
	const uint16 Frustum::QUADS[24] = { 4, 2, 3, 5, 7, 1, 2, 4, 7, 4, 5, 6, 5, 3, 0, 6, 6, 0, 1, 7, 2, 1, 0, 3 };

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
