#pragma once

#include "Core.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	typedef uint16 Index;
	class HardwareBuffer;
	typedef Ptr<HardwareBuffer> HardwareBufferPtr;
	class Texture;
	typedef Ptr<Texture> TexturePtr;
	class FrameBuffer;
	typedef Ptr<FrameBuffer> FrameBufferPtr;
	class Shader;
	typedef Ptr<Shader> ShaderPtr;
	class ShaderStage;
	typedef Ptr<ShaderStage> ShaderStagePtr;
	class ShaderInstance;
	typedef Ptr<ShaderInstance> ShaderInstancePtr;


	//----------------------------------------------------------------------------//
	// Vertex
	//----------------------------------------------------------------------------//

	struct Vertex // 48 bytes
	{
		Vec3 pos; // 12	0
		union // 36 12
		{
			struct
			{
				float texCoord[2]; // 8	12..20
				float texCoord2[2]; // 8 20..28
				uint8 color[4]; // 4 28..32
				uint8 normal[4]; // 4 32..36
				uint8 tangent[4]; // 4 36..40
				uint8 indices[4]; // 4 40..44
				uint8 weights[4]; // 4 44..48

			} mesh;

			struct
			{
				float velocity[3]; // 12 12..24
				float age; // 4	24..28
				uint8 color[4]; //4	28..32
				float size; // 4 32..36
				float unused; // 4 36..40
				float rotation; // 4 40..44
				//float prevPos[3]; // 12 36..48

			} particle;

			struct // 36
			{
				float texCoord[2]; // 8	12..20
				float texCoord2[2]; // 8 20..28
				uint8 color[4]; // 4 28..32
				float size[2]; // 8 32..40
				float rotation; // 4 40..44
				int32 axis; // 4 44..48 // for billboards

			} sprite;
		};
	};

	//----------------------------------------------------------------------------//
	// HardwareBuffer
	//----------------------------------------------------------------------------//

	enum HardwareBufferType : uint8
	{
		HBT_VERTEX,
		HBT_INDEX,
		HBT_UNIFORM,
	};

	enum HardwareBufferUsage : uint8
	{
		HBU_STATIC,
		HBU_DYNAMIC,
	};

	class HardwareBuffer : public RefCounted
	{
	public:
		HardwareBuffer(HardwareBufferType _type, HardwareBufferUsage _usage = HBU_STATIC);
		virtual ~HardwareBuffer(void);
		void Realloc(uint _size);
		void Write(uint _offset, uint _size, const void* _data);
		HardwareBufferType GetType(void) { return m_type; }
		uint GetSize(void) { return m_size; }

	public:
		uint _Handle(void) { return m_handle; }
		uint _VertexArray(void) { return m_vertexArray; }

	protected:
		HardwareBufferType m_type;
		HardwareBufferUsage m_usage;
		uint m_size;
		uint m_handle;
		uint m_vertexArray;
	};

	//----------------------------------------------------------------------------//
	// PixelFormat
	//----------------------------------------------------------------------------//

	enum PixelFormat : uint8
	{
		PF_UNKNOWN,

		PF_R8_UNORM,
		PF_RG8_UNORM,
		PF_RGB8_UNORM,
		PF_RGBA8_UNORM,

		PF_R8_UINT,
		PF_RG8_UINT,
		PF_RGB8_UINT,
		PF_RGBA8_UINT,

		PF_R16_FLOAT,
		PF_RG16_FLOAT,
		PF_RGB16_FLOAT,
		PF_RGBA16_FLOAT,

		PF_R32_FLOAT,
		PF_RG32_FLOAT,
		PF_RGB32_FLOAT,
		PF_RGBA32_FLOAT,

		PF_RG11B10_FLOAT,

		PF_D24S8,

		PF_DXT1, // RGB
		PF_DXT1A, // RGBA (alpha = 1 bit)
		PF_DXT3, // RGBA
		PF_DXT5, // RGBA
		PF_RGTC1, // R
		PF_RGTC2, // RG

		MAX_PIXEL_FORMATS,
	};

	enum PixelFormatFlags : uint16
	{
		PFF_COLOR = 0x1,
		PFF_DEPTH_STENCIL = 0x2,
		PFF_UNORM = 0x4,
		PFF_UINT = 0x8,
		PFF_HALF = 0x10,
		PFF_FLOAT = 0x20,
		PFF_COMPRESSED = 0x40,
		PFF_WITH_ALPHA = 0x80,
		PFF_PACKED = 0x100,
		PFF_INTEGER = 0x200,
		PFF_UNSIGNED_INTEGER = PFF_INTEGER | 0x400,
		PFF_SIGNED_INTEGER = PFF_INTEGER | 0x800,
	};

	struct PixelFormatInfo
	{
		PixelFormat id;
		const char* name;
		uint8 bits;
		uint8 channels;
		uint16 flags;
		// internal
		uint16 internalFormat;
		uint16 format;
		uint16 type;
	};

	const PixelFormatInfo& GetPixelFormatInfo(PixelFormat _format);
	PixelFormat GetCompressedPixelFormat(PixelFormat _format);
	PixelFormat GetUncompressedPixelFormat(PixelFormat _format);
	//PixelFormat GetClosestPixelFormatForTextureBuffer(PixelFormat _format);

	//----------------------------------------------------------------------------//
	// Texture
	//----------------------------------------------------------------------------//

	enum TextureType : uint8
	{
		TT_2D,
		TT_2D_ARRAY,
		TT_2D_MULTISAMPLE,
		TT_3D,
		TT_CUBE,
	};

	enum TextureFlags : uint8
	{
		TF_MIPMAPS = 0x1,
		TF_COMPRESSION = 0x2,
	};

	/*enum TextureUsage : uint8
	{
		TU_IMAGE,
		TU_BUFFER,
		TU_TARGET,
	};*/

	enum TextureFilter : uint8
	{
		TF_NEAREST,
		TF_LINEAR,
		TF_BILINEAR,
		TF_TRILINEAR,
	};

	enum TextureWrap : uint8
	{
		TW_REPEAT,
		TW_CLAMP,
	};

	class Texture : public Resource
	{
	public:
		Texture(const String& _name, TextureType _type, uint _flags = 0);
		virtual ~Texture(void);
		void Realloc(PixelFormat _format, uint _width, uint _height, uint _depth);
		void Realloc(uint _width, uint _height) { Realloc(m_format, _width, _height, m_size.z); }
		void Write(uint _lod, uint _zOffset, uint _width, uint _height, PixelFormat _format, const void* _data);
		void GenerateLods(void);
		TextureType GetType(void) { return m_type; }
		PixelFormat GetFormat(void) { return m_format; }
		const Vec3ui& GetSize(void) { return m_size; }
		uint GetLodCount(void) { return m_lodCount; }
		uint GetBaseLod(void) { return m_baseLod; }

	public:
		uint _Handle(void) { return m_handle; }

	protected:
		TextureType m_type;
		//TextureUsage m_usage;
		//bool m_useCompression;
		uint m_flags;
		PixelFormat m_format;
		Vec3ui m_size;
		uint m_baseLod;
		uint m_lodCount;
		uint m_availableLodCount;
		uint m_memorySize;
		uint m_handle;
		static uint s_memorySize;
	};

	//----------------------------------------------------------------------------//
	// TextureManager
	//----------------------------------------------------------------------------//

#define GTextureManager TextureManager::Get()

	class TextureManager : public Singleton < TextureManager >
	{
	public:

		Texture* GetDefaultTexture2D(void) { return m_defaultTexture2D; }

	protected:
		friend class Texture;
		friend class RenderSystem;

		TextureManager(void);
		~TextureManager(void);
		bool _Init(void);
		void _Destroy(void);

		TexturePtr m_defaultTexture2D;
	};

	//----------------------------------------------------------------------------//
	// Sampler
	//----------------------------------------------------------------------------//

	class Sampler : public Resource
	{
	protected:
		TextureFilter m_filter;
		TextureWrap m_wrap;

	};
	typedef Ptr<Sampler> SamplerPtr;

	//----------------------------------------------------------------------------//
	// FrameBuffer
	//----------------------------------------------------------------------------//

	enum FrameBufferType : uint8
	{
		FBT_COLOR = 0x1,
		FBT_DEPTH = 0x2,
		FBT_STENCIL = 0x4,
		FBT_DEPTH_STENCIL = FBT_DEPTH | FBT_STENCIL,
		FBT_ALL = FBT_COLOR | FBT_DEPTH_STENCIL,
	};

	enum{ MAX_RENDER_TARGETS = 8 };

	class FrameBuffer : public RefCounted
	{
	public:
		FrameBuffer(void);
		virtual ~FrameBuffer(void);
		void SetSize(const Vec2ui& _size);
		const Vec2ui& GetSize(void) { return m_size; }
		void SetColorTarget(uint _slot, Texture* _target, uint _zOffset = 0);
		Texture* GetColorTarget(uint _slot) { ASSERT(_slot < MAX_RENDER_TARGETS); m_color[_slot].target; }
		uint GetColorZOffset(uint _slot) { ASSERT(_slot < MAX_RENDER_TARGETS); m_color[_slot].zOffset; }
		void SetDepthStencilTarget(Texture* _target, uint _zOffset = 0);
		Texture* GetDepthStencilTarget(Texture* _target) { return m_depthStencil.target; }
		uint GetDepthStencilZOffset(Texture* _target) { return m_depthStencil.zOffset; }
		void GenerateLods(void);

	public:
		uint _Handle(void) { return m_handle; }

	protected:

		struct Slot
		{
			TexturePtr target;
			uint zOffset = 0;
		};

		Slot m_color[MAX_RENDER_TARGETS];
		Slot m_depthStencil;
		Vec2ui m_size;
		uint m_handle;
	};

	//----------------------------------------------------------------------------//
	// ShaderStage
	//----------------------------------------------------------------------------//

	enum ShaderStageType : uint8
	{
		SST_VERTEX,
		SST_FRAGMENT,
		SST_GEOMETRY,
		MAX_SHADER_STAGES,
	};

	class ShaderStage : public RefCounted
	{
	public:
		const String& GetName(void) { return m_name; }
		const String& GetLog(void) { return m_log; }

	protected:
		friend class Shader;
		friend class ShaderInstance;

		ShaderStage(Shader* _creator, const String& _name, ShaderStageType _type, const String& _entry, uint32 _id, uint32 _defsId);
		virtual ~ShaderStage(void);
		bool _Compile(void);

		Shader* m_shader;
		ShaderStageType m_type;
		String m_name;
		String m_entry;
		uint32 m_id;
		uint32 m_defsId;
		uint m_handle;
		String m_log;
		bool m_compiled;
		bool m_valid;
	};

	//----------------------------------------------------------------------------//
	// ShaderParam
	//----------------------------------------------------------------------------//

	enum ShaderParamType : uint8
	{
		SPT_UNKNOWN,

		SPT_UNIFORM_INT,
		SPT_UNIFORM_VEC2I,
		SPT_UNIFORM_VEC3I,
		SPT_UNIFORM_VEC4I,
		SPT_UNIFORM_FLOAT,
		SPT_UNIFORM_VEC2,
		SPT_UNIFORM_VEC3,
		SPT_UNIFORM_VEC4,
		SPT_UNIFORM_MAT34,
		SPT_UNIFORM_MAT44,

		SPT_UNIFORM_BLOCK,

		SPT_SAMPLER_2D,
		SPT_SAMPLER_2D_ARRAY,
		SPT_SAMPLER_2D_MULTISAMPLE,
		SPT_SAMPLER_3D,
		SPT_SAMPLER_CUBE,
		SPT_SAMPLER_BUFFER,
	};

	struct ShaderBlockItem
	{
		String name;
		ShaderParamType type;
		uint alignment;
		uint offset;
		uint stride;
		uint count;
	};

	struct ShaderBlockSignature
	{
		Array<ShaderBlockItem> items;
		uint alignent = 0;
		uint size = 0;
		uint stride = 0;
	};

	struct ShaderParam
	{
		bool IsUniform(void) const { return type >= SPT_UNIFORM_INT && type <= SPT_UNIFORM_MAT44; }
		bool IsBuffer(void) const { return type == SPT_UNIFORM_BLOCK; }
		bool IsTexture(void) const { return type >= SPT_SAMPLER_2D && type <= SPT_SAMPLER_BUFFER; }

		String name;
		ShaderParamType type = SPT_UNKNOWN;
		uint count = 0;
		//ShaderBlockSignature bufferSignature;
		uint size = 0;
		uint slot = 0;
		int internalId = -1;

		static const ShaderParam Null;
	};

	//----------------------------------------------------------------------------//
	// ShaderInstance
	//----------------------------------------------------------------------------//

	class ShaderInstance : public RefCounted
	{
	public:
		const String& GetName(void) { return m_name; }
		const String& GetLog(void) { return m_log; }

		int GetParamIndex(const String& _name)
		{
			auto _id = m_paramNames.find(_name);
			return _id != m_paramNames.end() ? _id->second : -1;
		}
		uint GetParamCount(void) { return m_params.size(); }
		const ShaderParam& GetParamInfo(uint _index) { return _index < m_params.size() ? m_params[_index] : ShaderParam::Null; }

		int GetUniformIndex(uint _index) { return _index < m_uniforms.size() ? m_uniforms[_index] : -1; }
		uint GetUniformsCount(void) { return m_uniforms.size(); }
		const ShaderParam& GetUniformInfo(uint _index) { return GetParamInfo(GetUniformIndex(_index)); }

		int GetTextureIndex(uint _index) { return _index < m_textures.size() ? m_textures[_index] : -1; }
		uint GetTexturesCount(void) { return m_textures.size(); }
		const ShaderParam& GetTextureInfo(uint _index) { return GetParamInfo(GetTextureIndex(_index)); }

		int GetBufferIndex(uint _index) { return _index < m_buffers.size() ? m_buffers[_index] : -1; }
		uint GetBuffersCount(void) { return m_buffers.size(); }
		const ShaderParam& GetBufferInfo(uint _index) { return GetParamInfo(GetBufferIndex(_index)); }

	public:
		uint _Handle(void) { return m_handle; }

	protected:
		friend class Shader;

		ShaderInstance(Shader* _creator, const String& _name, const String& _vsEntry, const String& _fsEntry, const String& _gsEntry, uint32 _id, uint32 _defsId);
		virtual ~ShaderInstance(void);
		bool _Link(void);

		ShaderPtr m_shader;
		String m_name;
		uint32 m_id;
		uint32 m_defsId;
		ShaderStagePtr m_stages[MAX_SHADER_STAGES];
		uint m_handle;
		String m_log;
		bool m_linked;
		bool m_valid;

		HashMap<String, uint> m_paramNames;
		Array<ShaderParam> m_params;
		Array<uint> m_uniforms;
		Array<uint> m_textures;
		Array<uint> m_buffers;
	};

	//----------------------------------------------------------------------------//
	// Shader
	//----------------------------------------------------------------------------//

	class Shader : public Resource
	{
	public:
		Shader(const String& _name);
		virtual ~Shader(void);
		const String& GetLog(void) { return m_log; }
		const String& GetData(uint _index) { ASSERT(_index <= MAX_SHADER_STAGES); return m_data[_index]; }
		ShaderInstancePtr CreateInstance(const String& _vsEntry, const String& _fsEntry, const String& _gsEntry, const String& _defs);

		static bool GetSourceName(uint32 _uid, String& _name);

	protected:
		friend class ShaderManager;	// ctor
		friend class ShaderStage; // _GetDefs, _Load
		friend class ShaderInstance;  // _CreateStage, m_shaderInstances

		bool _Load(void);
		ShaderStage* _CreateStage(ShaderStageType _type, const String& _entry, uint32 _defsId);
		uint32 _AddDefs(const String& _defs);
		const StringArray& _GetDefs(uint32 _defsId);

		uint m_uid; // Resource::m_id ?
		String m_data[MAX_SHADER_STAGES + 1];
		String m_log;
		HashMap<uint32, ShaderPtr> m_includes;
		HashMap<uint32, Shader*> m_dependents;
		HashMap<uint32, ShaderStagePtr> m_stageInstances; // стадии шейдера не выгружаются на протяжении всей работы приложения
		HashMap<uint32, ShaderInstance*> m_shaderInstances;
		HashMap<uint32, StringArray> m_defs;


		static HashMap<uint32, String> s_names;
		static uint32 s_nextId;
	};

	//----------------------------------------------------------------------------//
	// ShaderManager
	//----------------------------------------------------------------------------//

#define GShaderManager ShaderManager::Get()

	class ShaderManager : public Singleton < ShaderManager >
	{
	public:
		ShaderPtr LoadShader(const String& _name);

	protected:
		friend class Shader;
		friend class RenderSystem;

		ShaderManager(void);
		~ShaderManager(void);
		bool _Init(void);
		void _Destroy(void);

		HashMap<uint32, ShaderPtr> m_shaders; // шейдера не выгружаются на протяжении всей работы приложения
	};

	//----------------------------------------------------------------------------//
	// UniformGroup
	//----------------------------------------------------------------------------//




	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	enum { PRIMITIVE_RESTART = 0xffff };

	enum PrimitiveType : uint8
	{
		PT_POINTS,
		PT_LINES,
		PT_LINES_ADJACENCY,
		PT_LINE_STRIP,
		PT_LINE_STRIP_ADJACENCY,
		PT_LINE_LOOP,
		PT_TRIANGLES,
		PT_TRIANGLES_ADJACENCY,
		PT_TRIANGLE_STRIP,
		PT_TRIANGLE_STRIP_ADJACENCY,
	};

	enum CompareFunc : uint
	{
		CF_NEVER,
		CF_ALWAYS,
		CF_LESS,
		CF_LEQUAL,
		CF_EQUAL,
		CF_NEQUAL,
		CF_GREATER,
		CF_GEQUAL,
	};

	//----------------------------------------------------------------------------//
	// BlendState
	//----------------------------------------------------------------------------//

	enum BlendFactor : uint8
	{
		BF_ZERO,
		BF_ONE,
		BF_SRC_COLOR,
		BF_INV_SRC_COLOR,
		BF_DST_COLOR,
		BF_INV_DST_COLOR,
		BF_SRC_ALPHA,
		BF_INV_SRC_ALPHA,
		BF_DST_ALPHA,
		BF_INV_DST_ALPHA,
		BF_CONSTANT,
		BF_INV_CONSTANT,
	};

	enum BlendFunc : uint8
	{
		BF_ADD,
		BF_SUBTRACT,
		BF_REVERSE_SUBTRACT,
		BF_MIN,
		BF_MAX,
	};

	struct BlendStateDesc
	{
		bool enabled = false;
		bool alphaToCoverage = false;
		BlendFactor src = BF_ONE;
		BlendFactor	dst = BF_ZERO;
		BlendFunc func = BF_ADD;
		bool separated = false;
		BlendFactor srcAlpha = BF_ONE;
		BlendFactor	dstAlpha = BF_ZERO;
		BlendFunc funcAlpha = BF_ADD;
	};

	class BlendState : public NonCopyable
	{
	public:
		const BlendStateDesc& GetDesc(void) { return m_desc; }

	protected:
		friend class RenderSystem;

		BlendState(const BlendStateDesc& _desc);
		~BlendState(void);

		void _Bind(void);

		BlendStateDesc m_desc;
	};

	//----------------------------------------------------------------------------//
	// BlendState
	//----------------------------------------------------------------------------//

	enum StencilOp : uint
	{
		SO_KEEP,
		SO_ZERO,
		SO_REPLACE,
		SO_INC,
		SO_INC_WRAP,
		SO_DEC,
		SO_DEC_WRAP,
		SO_INVERT,
	};

	struct StencilOpDesc
	{
		StencilOp fail;
		StencilOp zfail;
		StencilOp pass;
		CompareFunc func;
	};

	struct StencilStateDesc
	{
		StencilOpDesc op;
		StencilOpDesc backOp;

		int8 ref;
		uint8 mask;
		bool twosided;
		bool enabled;
	};

	class StencilState : public NonCopyable
	{
	public:

	protected:
		friend class RenderSystem;
	};

	struct DepthTestDesc
	{
		CompareFunc func;
		bool write;
		bool enabled;
	};

	class DepthState : public NonCopyable
	{
	public:

	protected:
		friend class RenderSystem;
	};

	enum PolygonFace
	{
		PF_FRONT,
		PF_BACK,
		PF_BOTH,
	};

	enum PolygonMode
	{
		PM_SOLID,
		PM_WIREFRAME,
		PM_POINTS,
	};

	enum FrontFace
	{
		FF_COUNTERCLOCKWISE,
		FF_CLOCKWISE,
	};


	enum ColorMask : uint8
	{
		CM_RED = 0x1,
		CM_GREEN = 0x2,
		CM_BLUE = 0x4,
		CM_ALPHA = 0x8,
		CM_RGB = CM_RED | CM_GREEN | CM_BLUE,
		CM_RGBA = CM_RGB | CM_ALPHA,
	};

	enum FillMode : uint8
	{
		FM_SOLID,
		FM_WIREFRAME,
	};

	enum CullMode : uint8
	{
		//CM_NONE = 0,
		CM_FRONT,
		CM_BACK,
	};

	struct RasterizerStateDesc
	{
		FillMode fill;
		CullMode cull;
		/*    BOOL FrontCounterClockwise;
		INT DepthBias;
		FLOAT DepthBiasClamp;
		FLOAT SlopeScaledDepthBias;
		BOOL DepthClipEnable;
		BOOL ScissorEnable;
		BOOL MultisampleEnable;
		BOOL AntialiasedLineEnable;*/
	};

	class RasterizerState : public NonCopyable
	{
	public:

	protected:
		friend class RenderSystem;
	};



	//----------------------------------------------------------------------------//
	// OcclusionQuery
	//----------------------------------------------------------------------------//

	class OcclusionQuery : public RefCounted
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//


	class Effect
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class SubMaterial
	{

	};

	class Material : public Resource
	{

	};

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

#define GRenderSystem RenderSystem::Get()

	class RenderSystem : public Singleton < RenderSystem >
	{
	public:

		void SetGeometry(HardwareBuffer* _vertices, HardwareBuffer* _indices);
		void SetShader(ShaderInstance* _shader);
		void SetUniformRaw(int _index, uint _count, const void* _value);

		void SetFrameBuffer(FrameBuffer* _fb);

		void BeginFrame(void);
		void EndFrame(void);

	public:
		friend class Engine;

		RenderSystem(void);
		~RenderSystem(void);
		bool _Init(void);
		void _Destroy(void);

		ShaderInstancePtr m_currentShader;
		FrameBufferPtr m_currentFrameBuffer;
	};


	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
