#pragma once

#include "Core.hpp"

struct SDL_Window;
typedef void* SDL_GLContext;

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//
	
	typedef Ptr<class BufferObject> BufferObjectPtr;

	typedef Ptr<class ShaderSource> ShaderSourcePtr;
	typedef Ptr<class Shader> ShaderPtr;


	enum PrimitiveType : uint8
	{
		PT_Points,
		PT_Lines,
		PT_Triangles,
		// todo
	};

	enum IndexFormat : uint8
	{
		IF_UShort = 2,
		IF_UInt = 4,
	};

	//----------------------------------------------------------------------------//
	// BufferObject
	//----------------------------------------------------------------------------//


	enum MappingMode : uint8
	{
		MM_None = 0,
		MM_Read = 0x1,
		MM_ReadWrite = 0x3,
		MM_Discard = 0x6,
	};

	class BufferObject : public RefCounted
	{
	public:

		uint GetSize(void) { return m_size; }
		uint GetElementSize(void) { return m_elementSize; }
		uint8* Map(MappingMode _mode, uint _offset, uint _size);
		void Unmap(void);
		void Copy(BufferObject* _src, uint _srcOffset, uint _dstOffset, uint _size);

	protected:
		friend class RenderContext;
		friend class Texture;

		BufferObject(bool _dynamic, uint _size, uint _esize, const void* _data);
		~BufferObject(void);

		uint m_size; // in bytes
		uint m_elementSize; // hint
		uint m_handle;
	};

	//----------------------------------------------------------------------------//
	// VertexAttrib
	//----------------------------------------------------------------------------//

	enum VertexSemantic : uint8
	{
		VS_MaxAttribs = 16,
		VS_Position = 0,
		VS_Normal,
		VS_Tangent,
		VS_Color,
		VS_Color2,
		VS_Weights,
		VS_Indices,
		VS_TexCoord,
		VS_TexCoord2,
		VS_TexCoord3,
		VS_TexCoord4,
		VS_LightMap,
		VS_Aux0,
		VS_Aux1,
		VS_Aux2,
		VS_Aux3,
	};

	enum VertexType : uint8
	{
		VT_Unknown,
		VT_Half2,
		VT_Half4,
		VT_Float,
		VT_Float2,
		VT_Float3,
		VT_Float4,
		VT_UByte4,
		VT_UByte4N,
		VT_Byte4,
		VT_Byte4N,
	};

	enum : uint
	{
		MAX_VERTEX_ATTRIBS = VS_MaxAttribs,
		MAX_VERTEX_STREAMS = 8,
	};

	struct VertexAttrib
	{
		VertexType type = VT_Unknown;
		VertexSemantic semantic = VS_MaxAttribs;
		uint8 stream = 0;
		uint8 divisor = 0;
		uint16 offset = 0;
	};

	//----------------------------------------------------------------------------//
	// VertexFormat
	//----------------------------------------------------------------------------//

	class VertexFormat : public NonCopyable
	{
	public:

	protected:
		friend class RenderContext;

		VertexFormat(Array<VertexAttrib>&& _attribs, uint _streams, uint _mask);
		~VertexFormat(void);
		uint _Bind(uint _activeAttribs);

		uint m_streams;
		uint m_attribMask;
		Array<VertexAttrib> m_attribs;

		static bool _InitCache(void);
		static void _DestroyCache(void);
		static VertexFormat* _AddInstance(const VertexAttrib* _attribs);
		static VertexFormat* _GetInstance(uint _index) { return s_instances[_index]; }

		static HashMap<uint, uint> s_indices;
		static Array<VertexFormat*> s_instances;
	};

	//----------------------------------------------------------------------------//
	// Texture
	//----------------------------------------------------------------------------//

	enum TextureType
	{
		TT_Unknown,
		TT_2D,
		TT_3D,
		TT_Cube,
		TT_Array,
		TT_Multisample,
		TT_Buffer,
	};

	//----------------------------------------------------------------------------//
	// ShaderDefines
	//----------------------------------------------------------------------------//

	class ShaderDefines
	{
	public:
		ShaderDefines(void);
		ShaderDefines(const String& _str);
		ShaderDefines(ShaderDefines&& _temp);
		ShaderDefines(const ShaderDefines& _other);
		ShaderDefines& operator = (ShaderDefines&& _temp);
		ShaderDefines& operator = (const ShaderDefines& _rhs);

		ShaderDefines& Clear(void);
		ShaderDefines& AddString(const String& _str);
		ShaderDefines& AddDef(const String& _def, const String& _val = "");
		ShaderDefines& AddDefs(const ShaderDefines& _defs);
		HashMap<String, String>& GetDefs(void) { m_hash = 0; return m_defs; }
		const HashMap<String, String>& GetDefs(void) const { return m_defs; }
		uint GetHash(void) const { _Update(); return m_hash; }
		const String& BuildText(void) const { _Update(); return m_text; }

		static uint AddUnique(const ShaderDefines& _defs);
		static const ShaderDefines& GetUnique(uint _id);

		static const ShaderDefines Empty;

	protected:
		void _Update(void) const;

		mutable HashMap<String, String> m_defs;
		mutable uint m_hash;
		mutable String m_text;

		static HashMap<uint, ShaderDefines> s_instances;
	};

	//----------------------------------------------------------------------------//
	// ShaderSource
	//----------------------------------------------------------------------------//

	enum ShaderType : uint8
	{
		ST_Vertex,
		ST_Fragment,
		ST_Geometry,
	};

	class ShaderSource : public Resource
	{
	public:
		CLASS(ShaderSource);

		bool BeginReload(void) override;

		bool IsValid(void) { return m_valid; }
		const String& GetSource(void) { return m_source; }
		const String& GetLog(void) { return m_errors; }

		ShaderPtr CreateInstance(ShaderType _type, const ShaderDefines& _defs = ShaderDefines::Empty);


	protected:
		friend class RenderContext;
		friend class Shader;

		ShaderSource(void);
		~ShaderSource(void);

		void _InitResource(ResourceManager* _mgr, const String& _name, uint _uid, uint _flags) override;
		void _GetFileName(void) override;
		bool _Load(void) override;
		bool _Load(File& _f) override;

		bool _IsIncluded(ShaderSource* _include);
 		void _Invalidate(void);
		void _SetSource(const String& _src);
		bool _ProcessSource(void);
		String _Parse(const char* _src);
		static uint16 _AddShaderName(const String& _name);
		static bool _GetShaderName(uint16 _id, String& _name);
		static String _ParseGLSLLog(const char* _log);

		uint16 m_nameId;
		bool m_processed;
		bool m_loaded;
		uint m_checksum;
		String m_rawSource;
		String m_source;
		String m_errors;		
		HashSet<ShaderSourcePtr> m_includes;
		HashSet<ShaderSource*> m_dependents;
		HashMap<uint, Shader*> m_instances;

		static Array<String> s_names;
		static HashMap<uint, uint16> s_nameIndices;
	};

	//----------------------------------------------------------------------------//
	// Shader
	//----------------------------------------------------------------------------//

	enum ShaderParamType
	{
		SPT_Unknown,
		SPT_Int,
		SPT_Vec2i,
		SPT_Vec3i,
		SPT_Vec4i,
		SPT_Float,
		SPT_Vec2,
		SPT_Vec3,
		SPT_Vec4,
		SPT_Mat34,
		SPT_Mat44,
		SPT_Buffer,
		SPT_Texture,
	};

	/*enum ShaderParamFlags
	{
	SPF_Texture2D = TT_2D,
	SPF_Texture3D = TT_3D,
	SPF_TextureCube = TT_Cube,
	SPF_TextureArray = TT_Array,
	SPF_TextureMultisample = TT_Multisample,
	SPF_TextureBuffer = TT_Buffer,
	SPF_TextureMask = 0x3f,
	SPF_TextureInteger = 0x100,
	SPF_TextureShadow = 0x200,
	};*/

	struct ShaderParam
	{
		String name; // name in shader
		ShaderParamType type = SPT_Unknown;
		uint size = 0; // size of element
		uint count = 0;
		uint slot = 0; // if type is SPT_Texture or SPT_Struct
					   //uint flags = 0;
		int location = -1; // for internal usage
	};

	//----------------------------------------------------------------------------//
	// ShaderBinds
	//----------------------------------------------------------------------------//

	typedef Ptr<class ShaderBindings> ShaderBindingsPtr;

	class ShaderBindings : public RefCounted
	{
	public:

		uint GetBufferSlot(const String& _name)
		{
			auto _it = m_bufferSlots.find(_name);
			return _it == m_bufferSlots.end() ? 0 : _it->second;
		}
		void SetBufferSlot(const String& _name, uint _index)
		{
			if (!_name.empty())
				m_bufferSlots[_name] = _index;
		}

		uint GetTextureSlot(const String& _name)
		{
			auto _it = m_textureSlots.find(_name);
			return _it == m_textureSlots.end() ? 0 : _it->second;
		}
		void SetTextureSlot(const String& _name, uint _index)
		{
			if (!_name.empty())
				m_textureSlots[_name] = _index;
		}

	protected:

		HashMap<String, uint> m_bufferSlots;
		HashMap<String, uint> m_textureSlots;
	};

	//----------------------------------------------------------------------------//
	// Shader
	//----------------------------------------------------------------------------//

	class Shader : public RefCounted
	{
	public:

	protected:
		friend class ShaderSource;
		friend class RenderContext;

		Shader(ShaderType _type, ShaderSource* _src, uint _defs, const String& _name, uint _uid);
		~Shader(void);

		void _Invalidate(void);
		bool _Compile(void);
		void _Reflect(void);

		String m_name;
		ShaderSourcePtr m_source;
		ShaderType m_type;
		uint m_defs;
		uint m_uid;
		uint m_handle;
		bool m_compiled;
		bool m_valid;

		Array<ShaderParam> m_params;
		HashMap<String, uint> m_names;
		HashMap<String, uint> m_textures;
		HashMap<String, uint> m_buffers;
		ShaderBindingsPtr m_bindings;

		struct CacheItem
		{
			uint checksum = 0;
			uint format = 0;
			uint size = 0;
			uint8* data = nullptr;
		};

		static bool _InitCache(void);
		static void _DestroyCache(void);
		static void _LoadCache(void);
		static void _SaveCache(void);
		static bool _GetCacheItem(uint _id, uint _checksum, CacheItem& _item);
		static void _SetCacheItem(uint _id, uint _checksum, uint _format, uint _size, uint8* _data);

		static bool s_cacheLoaded;
		static bool s_cacheChanged;
		static HashMap<uint, CacheItem> s_cache;
		static HashSet<Shader*> s_uncompiledShaders;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


#if 0
	class Shader : public RefCounted
	{
	public:

		const String& GetRawSource(void) { return m_rawSource; }
		const String& GetLog(void) { return m_log; }

		//bool IsCreatedFromFile(void) { return m_createdFromFile; }
		const String& GetSource(void) { return m_source; }

		//bool 

	protected:
		friend class ShaderInstance;

		Shader(const String& _name, uint _uid);
		~Shader(void);
		void _Invalidate(void);
		bool _Load(bool _checkFileTime = false);
		bool _IsIncluded(Shader* _include);
		bool _LogError(const char* _err, ...);
		bool _Parse(void);
		ShaderInstancePtr _AddInstance(ShaderType _type, uint _defs);
		void _RemoveInstance(uint _defs);

		bool m_valid;
		bool m_processed = false;
		bool m_loaded = false;
		String m_name;
		uint16 m_nameId;
		uint m_uid;
		time_t m_fileTime;
		time_t m_lastTime;
		String m_log;
		String m_rawSource;
		String m_source;
		HashSet<ShaderPtr> m_includes;
		HashSet<Shader*> m_dependents;
		HashMap<uint, ShaderInstance*> m_instances;

	protected:

		static bool _InitMgr(void);
		static void _DestroyMgr(void);
		static Shader* _Create(const String& _name);
		static uint16 _AddName(const String& _name);
		static bool _GetName(String& _name, uint16 _id);
		static String _ParseGLSLLog(const char* _log);

		static Array<String> s_names;
		static HashMap<uint, uint16> s_nameIndices;
		static HashMap<uint, ShaderPtr> s_cache;
	};

	//----------------------------------------------------------------------------//
	// ShaderInstance
	//----------------------------------------------------------------------------//

	class ShaderInstance : public RefCounted
	{
	public:

	protected:
		friend class Shader;
		friend class ShaderProgram;

		ShaderInstance(Shader* _shader, ShaderType _type, uint _defs, const String& _name, uint _uid);
		~ShaderInstance(void);
		void _Invalidate(void);
		void _Compile(void);
		bool _Wait(void);

		String m_name;
		ShaderPtr m_shader;
		ShaderType m_type;
		bool m_valid;
		bool m_compiled;
		bool m_inCompile;
		uint m_uid;
		uint m_defs;
		uint m_handle;
		String m_log;
		HashSet<ShaderProgram*> m_programs;
	};

	//----------------------------------------------------------------------------//
	// ShaderProgram
	//----------------------------------------------------------------------------//

	enum TextureType
	{
		TT_Unknown,
		TT_2D,
		TT_3D,
		TT_Cube,
		TT_Array,
		TT_Multisample,
		TT_Buffer,
	};

	enum ShaderParamType
	{
		SPT_Unknown,
		SPT_Int,
		SPT_Vec2i,
		SPT_Vec3i,
		SPT_Vec4i,
		SPT_Float,
		SPT_Vec2,
		SPT_Vec3,
		SPT_Vec4,
		SPT_Mat34,
		SPT_Mat44,
		SPT_Buffer,
		SPT_Texture,
	};

	/*enum ShaderParamFlags
	{
		SPF_Texture2D = TT_2D,
		SPF_Texture3D = TT_3D,
		SPF_TextureCube = TT_Cube,
		SPF_TextureArray = TT_Array,
		SPF_TextureMultisample = TT_Multisample,
		SPF_TextureBuffer = TT_Buffer,
		SPF_TextureMask = 0x3f,
		SPF_TextureInteger = 0x100,
		SPF_TextureShadow = 0x200,
	};*/

	struct ShaderParam
	{
		String name; // name in shader
		ShaderParamType type = SPT_Unknown;
		uint size = 0; // size of element
		uint count = 0;
		uint slot = 0; // if type is SPT_Texture or SPT_Struct
		//uint flags = 0;
		int location = -1; // for internal usage
	};

	class ShaderProgram : public RefCounted
	{
	public:

		int GetParam(const String& _name);
		void BindBuffer(uint _param, uint _slot);
		void BindTexture(uint _param, uint _slot);

	protected:


		ShaderProgram(uint _uid, ShaderInstance* _vs, ShaderInstance* _fs, ShaderInstance* _gs, uint _outputAttribs);
		~ShaderProgram(void);

		void _Link(void);

		uint m_uid;
		uint m_outputAttribs;
		ShaderInstancePtr m_stages[3];
		uint m_handle;
		bool m_linked;
		bool m_valid;
		String m_log;


		Array<ShaderParam> m_params;
		HashMap<String, uint> m_names;
		HashMap<String, uint> m_textures;
		HashMap<String, uint> m_buffers;

	protected:

		static bool _InitMgr(void);
		static void _DestroyMgr(void);
		static ShaderProgramPtr _Create(ShaderInstance* _vs, ShaderInstance* _fs, ShaderInstance* _gs, uint _outputAttribs);

		static HashMap<uint, ShaderProgram*> s_cache;
	};

#endif

	//----------------------------------------------------------------------------//
	// RenderContext
	//----------------------------------------------------------------------------//

#define gRenderContext Engine::RenderContext::Get()

	class RenderContext final : public Singleton<RenderContext>
	{
	public:

		SDL_Window* _GetSDLWindow(void) { return m_window; }

		VertexFormat* AddVertexFormat(const VertexAttrib* _attribs);

		uint ReloadShaders(void);

		//ShaderObjectPtr CreateShader(ShaderType _type);
		//ProgramObjectPtr CreateProgram(ShaderObject* _vs, ShaderObject* _fs, ShaderObject* _gs);

		//uint16 AddShaderName(const String& _name);
		//String GetShaderName(uint _name);

		void SetVertexFormat(VertexFormat* _fmt);
		void SetVertexBuffer(uint _slot, BufferObject* _buffer, uint _offset, uint _stride);
		void SetIndexFormat(IndexFormat _fmt);
		void SetIndexBuffer(BufferObject* _buffer, uint _offset);
		void SetPrimitiveType(PrimitiveType _type);
		void SetDrawIndirectBuffer(BufferObject* _buffer, uint _offset);


		void Draw(uint _baseVertex, uint _count, uint _numInstances = 1);
		void DrawIndexed(uint _baseVertex, uint _baseIndex, uint _count, uint _numInstances = 1);

		// 


	protected:
		friend class System;

		static bool _Create(void);
		static void _Destroy(void);

		RenderContext(void);
		~RenderContext(void);
		bool _Init(void);

		void _BeginFrame(void);
		void _EndFrame(void);

		SDL_Window* m_window = nullptr;
		SDL_GLContext m_context = nullptr;


		VertexFormat* m_vertexFormat = nullptr;
		BufferObjectPtr m_vertexBuffers[MAX_VERTEX_STREAMS];

		IndexFormat m_indexFormat = IF_UShort;
		uint m_indexFormatGL = 0;
		BufferObjectPtr m_indexBuffer;
		uint m_indexBufferOffset = 0;

		PrimitiveType m_primitiveType = PT_Points;
		uint m_primitiveTypeGL = 0;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
