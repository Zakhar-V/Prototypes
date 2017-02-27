#include "File.hpp"
#include <io.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#	include <direct.h>
#else
#	include <dirent.h>
#endif
#ifdef _WIN32
#	include <Windows.h>
#endif

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Path utils
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	bool IsFullPath(const char* _path)
	{
#ifdef _WIN32
		return _path && _path[1] == ':';
#else // linux
		return _path && _path[0] == '/';
#endif
	}
	//----------------------------------------------------------------------------//
	void SplitFilename(const String& _filename, String* _device, String* _dir, String* _name, String* _shortname, String* _ext)
	{
		uint _l = _filename.Length();
		if (_device)
		{
#ifdef _WIN32
			const char* c = strchr(_filename, ':');
			if (c)
				*_device = String(_filename, c);
#else
			const char* c = _filename.c_str();
			if (*c && strchr("/\\", *c))
			{
				while (*++c && !strchr("/\\", *c));
				*_device = String(_filename.Ptr(1), c);
			}
#endif
		}
		if (_dir)
		{
			uint c = _l;
			for (; c > 0 && strchr("/\\", _filename[c - 1]); --c);
			for (; c > 0 && !strchr("/\\", _filename[c - 1]); --c);
			*_dir = _filename.SubStr(0, c);
		}
		if (_name || _ext || _shortname)
		{
			uint _np = _l;
			for (; _np > 0 && !strchr("/\\", _filename[_np - 1]); --_np);
			String _n = _filename.SubStr(_np);
			if (_name)
				*_name = _n;
			if (_ext || _shortname)
			{
				uint _pp = _l - _np;
				for (; _pp > 0 && _n[_pp] != '.'; --_pp);
				if (_shortname)
					*_shortname = _pp > 0 ? _n.SubStr(0, _pp) : _n;
				if (_ext)
					*_ext = _pp > 0 ? _n.SubStr(_pp + 1) : String::Empty;
			}
		}
	}
	//----------------------------------------------------------------------------//
	void SplitPath(const char* _path, String* _device, StringArray* _items)
	{
		static const String _P1 = ".";
		static const String _P2 = "..";

		if (_path && *_path)
		{
			const char* p = _path;

#ifdef _WIN32
			const char* c = strchr(_path, ':');
			if (c)
			{
				if (_device)
					*_device = String(p, c);
				p = ++c;
			}
			else c = _path;
#else
			const char* c = _path;
			if (strchr("/\\", *c))
			{
				while (*++c && !strchr("/\\", *c));
				if (_device)
					*_device = String(p + 1, c);
				p = ++c;
			}
#endif
			if (_items)
			{
				_items->clear();
				_items->reserve(10);
				String _item;
				for (;;)
				{
					if (!*c || strchr("\\/", *c))
					{
						if (c != p)
						{
							_item = String(p, (uint)(c - p));
							if (_item == _P2)
							{
								if (_items->empty() || _items->back() == _P2)
									_items->push_back(_P2);
								else
									_items->pop_back();
							}
							else if (_item != _P1)
								_items->push_back(_item);
						}
						p = c + 1;
					}
					if (!*c++)
						break;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	String MakePath(const String& _device, const StringArray& _items)
	{
		String _path;
#ifdef _WIN32
		if (_device.NonEmpty())
			_path += _device + ":/";
#else
		if (_device.NonEmpty())
			_path += "/" + _device + "/";
#endif
		//if (_items.IsEmpty()) _path += ".";
		for (uint i = 0; i < _items.size(); ++i)
		{
			if (i > 0)
				_path += "/";
			_path += _items[i];
		}
		return _path;
	}
	//----------------------------------------------------------------------------//
	String MakeFullPath(const char* _path, const char* _root)
	{
		static const String _P1 = ".";
		static const String _P2 = "..";

		if (_path && *_path)
		{
			String _d, _pd;
			StringArray _p, _f;
			if (IsFullPath(_path) || !(_root && *_root))
			{
				char _last = _path[strlen(_path) - 1];
				if (!strchr(_path, '\\') &&
					!strstr(_path, "..") &&
					!strstr(_path, "./") &&
					!strstr(_path, "/.") &&
					!strstr(_path, "//") &&
					_last != '/')
					return _path;

				SplitPath(_path, &_pd, &_p);
				return MakePath(_pd, _p);
			}

			SplitPath(_path, &_pd, &_p);
			SplitPath(_root, &_d, &_f);
			for (uint i = 0; i < _p.size(); ++i)
			{
				if (_p[i] == _P2)
				{
					if (_f.empty() || _f.back() == _P2)
						_f.push_back(_P2);
					else
						_f.pop_back();
				}
				else if (_p[i] != _P1)
					_f.push_back(_p[i]);
			}
			return MakePath(_d, _f);
		}
		return String::Empty;
	}
	//----------------------------------------------------------------------------//

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
		std::swap(m_type, _temp.m_type);
		std::swap(m_raw, _temp.m_raw);
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
	// FileStreamHandle
	//----------------------------------------------------------------------------//
	
	//----------------------------------------------------------------------------//
	FileStreamHandle::FileStreamHandle(FILE* _handle, AccessMode _access) :
		m_handle(_handle),
		m_access(_access),
		m_size(0)
	{
		ASSERT(_handle != nullptr);
		ASSERT(_access != AM_None);

		if (!(m_access & AM_Write))
		{
			long _pos = ftell(m_handle);
			fseek(m_handle, 0, SEEK_END);
			m_size = (uint)ftell(m_handle);
			fseek(m_handle, _pos, SEEK_SET);
		}
	}
	//----------------------------------------------------------------------------//
	FileStreamHandle::~FileStreamHandle(void)
	{
		fclose(m_handle);
	}
	//----------------------------------------------------------------------------//
	uint FileStreamHandle::Size(void)
	{
		if (m_access & AM_Write)
		{
			long _pos = ftell(m_handle);
			fseek(m_handle, 0, SEEK_END);
			m_size = (uint)ftell(m_handle);
			fseek(m_handle, _pos, SEEK_SET);
		}
		return m_size;
	}
	//----------------------------------------------------------------------------//
	uint FileStreamHandle::Tell(void)
	{
		return (uint)ftell(m_handle);
	}
	//----------------------------------------------------------------------------//
	bool FileStreamHandle::EoF(void)
	{
		return feof(m_handle) != 0;
	}
	//----------------------------------------------------------------------------//
	void FileStreamHandle::Seek(int _pos, int _origin)
	{
		fseek(m_handle, _pos, _origin);
	}
	//----------------------------------------------------------------------------//
	uint FileStreamHandle::Read(void* _dst, uint _size)
	{
		ASSERT(_dst || !_size);
		if (m_access & AM_Read)
			return fread(_dst, 1, _size, m_handle);
		return 0;
	}
	//----------------------------------------------------------------------------//
	uint FileStreamHandle::Write(const void* _src, uint _size)
	{
		ASSERT(_src || !_size);
		if (m_access & AM_Write)
			return fwrite(_src, 1, _size, m_handle);
		return 0;
	}
	//----------------------------------------------------------------------------//
	void FileStreamHandle::Flush(void)
	{
		if (m_access & AM_Write)
			fflush(m_handle);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// MemoryStreamHandle
	//----------------------------------------------------------------------------//

	// ...

	//----------------------------------------------------------------------------//
	// DataStream
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	DataStream::DataStream(void) :
		m_handle(nullptr),
		m_size(0)
	{
	}
	//----------------------------------------------------------------------------//
	DataStream::DataStream(DataStream&& _temp) :
		m_name(_temp.m_name),
		m_handle(_temp.m_handle),
		m_size(_temp.m_size)
	{
	}
	//----------------------------------------------------------------------------//
	DataStream::DataStream(const String& _name, DataStreamHandle* _handle) :
		m_name(_name),
		m_handle(_handle),
		m_size(_handle ? _handle->Size() : 0)
	{
	}
	//----------------------------------------------------------------------------//
	DataStream::~DataStream(void)
	{
		if(m_handle)
			delete m_handle;
	}
	//----------------------------------------------------------------------------//
	DataStream& DataStream::operator = (DataStream&& _temp)
	{
		Swap(m_handle, _temp.m_handle);
		m_name = _temp.m_name;
		m_size = _temp.m_size;
		return *this;
	}
	//----------------------------------------------------------------------------//
	void DataStream::SetPos(int _pos, bool _relative)
	{
		if (m_handle) 
			m_handle->Seek(_pos, _relative ? SEEK_CUR : SEEK_SET);
	}
	//----------------------------------------------------------------------------//
	uint DataStream::GetPos(void)
	{
		return m_handle ? m_handle->Tell() : 0;
	}
	//----------------------------------------------------------------------------//
	void DataStream::ToEnd(void)
	{
		if (m_handle)
			m_handle->Seek(0, SEEK_END);
	}
	//----------------------------------------------------------------------------//
	bool DataStream::AtAnd(void)
	{
		return !m_handle || m_handle->EoF();
	}
	//----------------------------------------------------------------------------//
	uint DataStream::Read(void* _dst, uint _size)
	{
		return m_handle ? m_handle->Read(_dst, _size) : 0;
	}
	//----------------------------------------------------------------------------//
	uint DataStream::Write(const void* _src, uint _size)
	{
		return m_handle ? m_handle->Write(_src, _size) : 0;
	}
	//----------------------------------------------------------------------------//
	void DataStream::Flush(void)
	{
		if (m_handle) 
			m_handle->Flush();
	}
	//----------------------------------------------------------------------------//
	String DataStream::ReadString(int _maxLength)
	{
		String _r;
		if (m_handle)
		{
			uint _readed, _length, _pos = GetPos(), _size = _maxLength < 0 ? m_size : _maxLength;
			if (_pos > m_size) 
				_pos = m_size;
			if (_size + _pos > m_size)
				_size = m_size - _pos;

			Array<char> _buf(_size + 1);
			_readed = Read(&_buf[0], _size);
			_buf[_readed] = 0;
			_length = strlen(&_buf[0]);
			_r = &_buf[0];

			if (_length + 1 < _readed)
				SetPos(_pos + _length + 1); // null-terminated string
		}
		return _r;
	}
	//----------------------------------------------------------------------------//

	namespace
	{
		class VirtualFs
		{

		};

		class DiskFs : public VirtualFs
		{

		};

		class ZipFs	: public VirtualFs
		{

		};

		//----------------------------------------------------------------------------//
		// ZipStreamHandle
		//----------------------------------------------------------------------------//

		//----------------------------------------------------------------------------//
		// ZipStreamHandle
		//----------------------------------------------------------------------------//

		//----------------------------------------------------------------------------//
		// ZipStreamHandle
		//----------------------------------------------------------------------------//
	}


	//----------------------------------------------------------------------------//
	// FileSystem
	//----------------------------------------------------------------------------//

	FileSystem FileSystem::s_instance;

	//----------------------------------------------------------------------------//
	FileSystem::FileSystem(void)
	{
#	ifdef _WIN32
	{
		char _buf[4096];
		uint _length = GetModuleFileNameA(0, _buf, sizeof(_buf));
		_buf[_length] = 0;
		SplitFilename(MakeFullPath(_buf), nullptr, &m_appDir, &m_appName);
	}
#	else
		NOT_IMPLEMENTED_YET("Get app name");
		...
#	endif

		m_root = m_appDir;
		chdir(m_root);
	}
	//----------------------------------------------------------------------------//
	FileSystem::~FileSystem(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::SetRoot(const String& _path)
	{
		if (_path.NonEmpty())
		{
			String _fullPath = MakeFullPath(_path, m_appDir);
			struct stat _st;
			if (stat(_fullPath, &_st) == 0 && (_st.st_mode & S_IFDIR))
			{
				m_root = _fullPath;
				chdir(m_root);
				return true;
			}
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	void FileSystem::AddSearchDir(const String& _path, bool _highPrio, bool _hidden)
	{
		if (_path.NonEmpty())
		{
			SearchDir _sd;
			_sd.path = MakeFullPath(_path);
			_sd.fullPath = MakeFullPath(_path, m_root);
			_sd.hidden = _hidden;
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (!String::Compare(i->fullPath, _sd.fullPath, FS_IGNORE_CASE))
				{
					if (_highPrio)
					{
						m_dirs.erase(i);
						break;
					}
					else
					{
						i->hidden = _hidden;
						return;
					}
				}
			}
			_highPrio ? m_dirs.push_front(_sd) : m_dirs.push_back(_sd);
		}
	}
	//----------------------------------------------------------------------------//
	void FileSystem::RemoveSearchDir(const String& _path)
	{
		if (_path.NonEmpty())
		{
			String _fullPath = MakeFullPath(_path, m_root);
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (!String::Compare(i->fullPath, _fullPath, FS_IGNORE_CASE))
				{
					m_dirs.erase(i);
					return;
				}
			}
		}
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::GetSearchDir(uint _index, SearchDir& _path)
	{
		if (_index < m_dirs.size())
		{
			auto _it = m_dirs.begin();
			while (_index--) ++_it;
			_path = *_it;
			return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	String FileSystem::Search(const String& _path, uint _mask)
	{
		if (_path.NonEmpty())
		{
			uint _st_mode = 0;
			if (_mask & FF_File)
				_st_mode |= S_IFREG;
			if (_mask & FF_Dir)
				_st_mode |= S_IFDIR;

			// full path
			struct stat _st;
			if (IsFullPath(_path))
				return stat(_path, &_st) == 0 && (_st.st_mode & _st_mode) ? MakeFullPath(_path) : String::Empty;

			// in directories
			String _fullPath, _npath = MakeFullPath(_path);
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				_fullPath = i->fullPath + "/" + _npath;
				if (stat(_fullPath, &_st) == 0 && (_st.st_mode & _st_mode))
					return _fullPath;
			}

			// in root directory
			_fullPath = m_root + "/" + _npath;
			if (stat(_fullPath, &_st) == 0 && (_st.st_mode & _st_mode))
				return _fullPath;
		}
		return String::Empty;
	}
	//----------------------------------------------------------------------------//
	String FileSystem::SearchFile(const String& _path, const StringArray& _dirs, const StringArray& _exts)
	{
		if (_path.NonEmpty())
		{
			// full path
			struct stat _st;
			if (IsFullPath(_path))
				return stat(_path, &_st) == 0 && (_st.st_mode & S_IFREG) ? MakeFullPath(_path) : String::Empty;

			// in directories
			String _result, _npath = MakeFullPath(_path);;
			for (auto i = m_dirs.begin(); i != m_dirs.end(); ++i)
			{
				if (_SearchFile(_npath, i->fullPath, _dirs, _exts, _result))
					return _result;
			}

			// in root directory
			//if (_SearchFile(_npath, m_root, _dirs, _exts, _result))
			//	return true;
		}
		return String::Empty;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::_SearchFile(const String& _path, const StringArray& _exts, String& _result)
	{
		struct stat _st;
		_result = _path;
		if (stat(_result, &_st) == 0 && (_st.st_mode & S_IFREG))
			return true;

		for (size_t i = 0, s = _exts.size(); i < s; ++i)
		{
			_result = _path + "." + _exts[i];
			if (stat(_result, &_st) == 0 && (_st.st_mode & S_IFREG))
				return true;
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::_SearchFile(const String& _path, const String& _root, const StringArray& _dirs, const StringArray& _exts, String& _result)
	{
		struct stat _st;
		if (stat(_root, &_st) == 0 && (_st.st_mode & S_IFDIR))
		{
			if (_SearchFile(_root + "/" + _path, _exts, _result))
				return true;

			String _dir;
			for (size_t i = 0, s = _dirs.size(); i < s; ++i)
			{
				_dir = _root + "/" + _dirs[i];
				if (stat(_dir, &_st) == 0 && (_st.st_mode & S_IFDIR))
				{
					if (_SearchFile(_dir + "/" + _path, _exts, _result))
						return true;
				}
			}
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::CreateDir(const String& _path)
	{
		LOG_MSG(LL_Error, "Couldn't create dir \"%s\"", *_path);
		return false;
	}
	//----------------------------------------------------------------------------//
	bool FileSystem::GetInfo(const String& _path, FileInfo& _info, uint _mask)
	{
		_info.path = Search(_path, _mask);
		if (_info.path.NonEmpty())
		{
			struct stat _st;
			if (stat(_info.path, &_st) == 0)
			{
				struct tm _t;
				_t = *localtime(&_st.st_atime);
				_info.ltime = mktime(&_t);
				_t = *gmtime(&_st.st_atime);
				_info.gtime = mktime(&_t);
				_info.size = (uint)_st.st_size;
				_info.flags = (_st.st_mode & S_IFREG) ? FF_File : FF_Dir;
				return true;
			}
		}
		return false;
	}
	//----------------------------------------------------------------------------//
	DataStream FileSystem::Open(const String& _name)
	{
		String _nname = MakeFullPath(_name);
		if (_nname.NonEmpty())
		{
			FILE* _handle = nullptr;
			String _path = Search(_name, FF_File);
			if (_path.IsEmpty())
			{
				LOG_MSG(LL_Error, "Couldn't search file \"%s\"", *_nname);
			}
			else
			{
				_handle = fopen(_path, "rb");
				if (!_handle)
				{
					LOG_MSG(LL_Error, "Couldn't open file \"%s\"", *_nname);
				}
			}
			return DataStream(_nname, _handle ? new FileStreamHandle(_handle, AM_Read) : nullptr);
		}
		return DataStream();
	}
	//----------------------------------------------------------------------------//
	DataStream FileSystem::Create(const String& _name, bool _overwrite)
	{
		String _nname = MakeFullPath(_name);
		if (_nname.NonEmpty())
		{
			String _path = IsFullPath(_nname) ? MakeFullPath(_nname, m_root) : _nname;
			CreateDir(FilePath(_path));
			FILE* _handle = fopen(_path, _overwrite ? "wb" : "r+b");
			if (!_handle)
			{
				LOG_MSG(LL_Error, "Couldn't create file \"%s\"", *_nname);
			}
			return DataStream(_nname, _handle ? new FileStreamHandle(_handle, AM_Write) : nullptr);
		}
		return DataStream();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
