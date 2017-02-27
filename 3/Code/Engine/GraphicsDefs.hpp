#pragma once

#include "Base.hpp"

#ifdef _DEBUG
#define DEBUG_RC // Enable debugging
#endif

#define USE_GL
//#define USE_D3D11
//#define USE_D3D12

struct SDL_Window;

namespace Engine
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	//#define gRenderSystem ge::RenderSystem::Get()
#define gRenderDevice Engine::RenderDevice::Get()
#define gShaderModel Engine::RenderDevice::Get()->GetShaderModel()

	enum DebugOutputLevel : int
	{
		DOL_Disabled,
		DOL_High,
		DOL_Normal,
		DOL_Low,
	};

	enum ShaderModel : uint8
	{
		SM_Unknown = 0,
		//SM3 = 30,
		SM4 = 41,
		SM5 = 50,
	};

	enum RendererType : uint8
	{
		RT_Unknown = 0,
		RT_GL,
		//RT_GLES,
		//RT_D3D11,
		//RT_D3D12,
		//RT_Vulkan,
	};

	enum RenderContextProfile : uint8
	{
		RCP_Core, //!< Use core functional of shader model.
		RCP_Compatible, //!< For OpenGL: use compatible profile context. For others renderers it same as ge::RCP_Core. 
		RCP_Forward, //!< Use maximal available shader model.
	};


	// limits
	enum : uint
	{
		MAX_VERTEX_STREAMS = 8,
		MAX_VERTEX_ATTRIBS = 16,
		MAX_VERTEX_AUX_ATTRIBS = 5,
		MAX_VERTEX_TEXCOORDS = 4,
		MAX_SHADER_STAGES = 3,
	};

	enum CompareFunc : uint8
	{
		CF_Never = 0, // false
		CF_Always, // true
		CF_Less, // <
		CF_LessEqual, // <=
		CF_Equal, // ==
		CF_NotEqual, // !=
		CF_Greater,	// >
		CF_GreaterEqual, // >=
	};

	enum StencilOp : uint8
	{
		SO_Keep,
		SO_Zero,
		SO_Replace,
		SO_Invert,
		SO_IncrWrap,
		SO_DecrWrap,
		SO_Incr,
		SO_Decr,
	};

	struct StencilFunc
	{
		StencilOp fail;
		StencilOp zfail;
		StencilOp zpass;
		CompareFunc func;
	};

	struct DepthStencilDesc
	{
		bool depthTest;
		bool depthWrite;
		CompareFunc depthFunc;

		bool stencilTest;
		uint8 stencilRef;
		uint8 stencilMask;
		bool stencilFuncSeparate;
		StencilFunc stencilFunc;
		StencilFunc stencilBackFunc;
	};

	enum BlendFunc : uint8
	{

	};

	enum BlendFactor : uint8
	{

	};


	enum MappingMode : uint8
	{
		MM_Read,
		MM_Write,
		MM_Discard,
	};

	enum HwBufferUsage
	{
		HBU_Default,
		HBU_Dynamic,
		HBU_Readback,
	};

	enum VertexAttribType : uint8
	{
		VAT_Unknown = 0,
		VAT_Half2,
		VAT_Half4,
		VAT_Float,
		VAT_Float2,
		VAT_Float3,
		VAT_Float4,
		VAT_UByte4,
		VAT_UByte4N,
		VAT_Byte4N,
		VAT_UShort2,
		VAT_UShort2N,
		VAT_UShort4,
		VAT_UShort4N,
		VAT_Short4N,
		//VAT_UInt64,
	};

	enum VertexAttrib : uint8
	{
		VA_Position,
		VA_Normal,
		VA_Tangent,
		VA_Color,
		VA_BoneIndices,
		VA_BoneWeights,
		VA_TexCoord0,
		VA_TexCoord1,
		VA_TexCoord2,
		VA_TexCoord3,
		VA_Aux0,
		VA_Aux1,
		VA_Aux2,
		VA_Aux3,
		VA_Aux4,
		VA_Aux5,
	};

	struct VertexAttribDesc
	{
		VertexAttribType type = VAT_Unknown;
		uint8 stream = 0;
		uint16 offset = 0;
		uint16 stride = 0;
		uint16 divisor = 0;
	};

	struct VertexFormatDesc
	{
		VertexFormatDesc(void) { }
		VertexFormatDesc& operator () (VertexAttrib _attrib, VertexAttribType _type, uint8 _stream, ptrdiff_t _offset, ptrdiff_t _stride, uint16 _divisor)
		{
			ASSERT(_stream < MAX_VERTEX_STREAMS);
			ASSERT(_offset < 0xffff);
			ASSERT(_stride < 0xffff);

			VertexAttribDesc& _va = attribs[_attrib];
			_va.type = _type;
			_va.stream = _stream;
			_va.offset = (uint16)_offset;
			_va.stride = (uint16)_stride;
			_va.divisor = (uint16)_divisor;

			return *this;
		}
		const VertexAttribDesc& operator [] (uint _index) const { return attribs[_index]; }
		VertexAttribDesc& operator [] (uint _index) { return attribs[_index]; }

		VertexAttribDesc attribs[MAX_VERTEX_ATTRIBS];

		static const VertexFormatDesc Empty;
	};



	enum TextureType : uint8
	{
		TT_Unknown,	//!< no type.
		TT_2D, //!< 2D texture.
		TT_2DArray, //!< Array of 2D textures.
		TT_2DMultisample,  //!<	2D mutlisample texture. \note PixelFormat must be one of uncompressed formats.
		TT_3D, //!< 3D texture.
		TT_Cube, //!< Cube map texture.
		TT_Buffer, //!< Texture buffer. \note PixelFormat must be with 1, 2 or 4 components and with type of component one of: byte, short, int, half, float.
	};

	enum TextureUsage : uint8
	{
		TU_Default, //!< optimized for usage on GPU.
		TU_Dynamic,	//!< optimized for frequent updating on CPU (read and write).
	};

	enum PixelFormat : uint8
	{
		PF_Unknown = 0,

		// unorm 
		PF_R8,
		PF_RG8,
		PF_RGBA8,
		PF_RGB5A1,
		PF_RGB10A2,

		// float
		PF_R16F,
		PF_RG16F,
		PF_RGBA16F,
		PF_R32F,
		PF_RG32F,
		PF_RGBA32F,
		PF_RG11B10F,

		// integer
		PF_R16UI,
		PF_RG16UI,
		PF_R32UI,
		PF_RGB10A2UI,

		// depth/stencil
		PF_D24S8,
		PF_D32F,

		// compressed
		PF_RGTC1,
		PF_RGTC2,
		PF_DXT1,
		PF_DXT1A,
		PF_DXT3,
		PF_DXT5,
	};

	enum CubemapFace : uint8
	{
		CF_PositiveX = 0,
		CF_NegativeX,
		CF_PositiveY,
		CF_NegativeY,
		CF_PositiveZ,
		CF_NegativeZ,
	};

	enum TextureFilter : uint8
	{
		TF_Nearest,
		TF_Linear,
		TF_Bilinear,
		TF_Trilinear,
	};

	enum TextureWrap : uint8
	{
		TW_Repeat,
		TW_Clamp,
	};

	enum ShaderType : uint8
	{
		ST_Vertex,
		ST_Fragment,
		ST_Geometry,
	};

	enum ShaderState : uint8
	{
		SS_Initial,
		SS_Compiling,
		SS_Compiled,
		SS_Invalid,
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#define MULTIRENDER

#ifdef MULTIRENDER

#define MR_OVERRIDE override
#define MR_BASECLASS(A, B) A


#else

#define MR_BASECLASS(A, B) B
#define MR_OVERRIDE

#endif

	enum GpuMemoryType : uint
	{
		GMT_Buffer = 0x01000000,
		GMT_Texture = 0x02000000,
		GMT_Internal = 0x03000000,
	};

	enum GpuMemoryPool : uint
	{
		GMP_Static = 0x00010000,
		GMP_Streaming = 0x00020000,
		GMP_Cache = 0x00030000,
		GMP_Temp = 0x00040000,
		GMP_Staging = 0x00050000,
		GMP_Compute = 0x00060000,
	};

	enum GpuMemoryGroup : uint
	{
		GMG_Other = 0x0000,
		GMG_Special = 0x0100, // for object-specific data
		GMG_Instancing = 0x0200, // for object-specific data
		GMG_Vertex = 0x0300, // for vertex buffers
		GMG_Index = 0x0400, // for index buffers
		GMG_Texture = 0x0500, // for textures
		GMG_RenderTarget = 0x0600, // for render target textures
	};

	typedef uint8 GpuMemoryGroupId;

	enum : uint
	{
		GM_TypeMask = 0xff000000,
		GM_TypeShift = 24,
		GM_PoolMask = 0x00ff0000,
		GM_PoolShift = 16,
		GM_GroupMask = 0xff00,
		GM_GroupShift = 8,
		GM_GroupIdMask = 0x00ff,
		GM_GroupIdShift = 0,
	};


	// interfaces
#ifdef MULTIRENDER

	// new names ...
	class RHBuffer;
	class RHVertexFormat;
	class RHTexture;
	class RHRenderTarget;

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class HardwareBuffer : public RCBase
	{
	public:
		virtual uint8* Map(MappingMode _mode, uint _offset, uint _size) = 0;
		virtual void Unmap(void) = 0;
		virtual void CopyFrom(HardwareBuffer* _src, uint _srcOffset, uint _dstOffset, uint _size) = 0;
		virtual HwBufferUsage GetUsage(void) = 0;
		virtual uint GetSize(void) = 0;
		virtual uint GetMemoryGroup(void) = 0; // TODO: remove
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class HardwareVertexFormat : public NonCopyable
	{
	public:
		virtual const VertexFormatDesc& GetDesc(void) = 0;

	protected:
		virtual ~HardwareVertexFormat(void) { }
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class HardwareTexture : public RCBase
	{
	public:
		virtual bool Realloc(uint _width, uint _height, uint _depth = 0) = 0;
		virtual uint8* Map(MappingMode _mode, uint _lod) = 0;
		virtual void Unmap(void) = 0;
		virtual HardwareBuffer* GetBuffer(void) = 0;
		virtual void SetLodRange(uint _baseLod, uint _numLods = 0) = 0;

	protected:
	};

	//----------------------------------------------------------------------------//
	// RenderContext
	//----------------------------------------------------------------------------//

#define gRenderContext Engine::RenderContext::Get()

	class RenderContext : public Singleton<RenderContext>
	{
	public:


		
		virtual void BeginFrame() = 0;
		virtual void EndFrame(void) = 0;
	
		// [Geometry]

		virtual void SetVertexFormat(HardwareVertexFormat* _format) = 0;
		virtual void SetVertexBuffer(uint _stream, HardwareBuffer* _buffer, uint _offset = 0, uint _stride = 0) = 0;
		//virtual void SetIndexBuffer(HardwareBuffer* _buffer, uint _offset) = 0;

		// [Render to texture]
		//virtual void SetRenderTarget(HardwareRenderTarget* _rt, uint _lod, uint _firstLayer, uint _numLayers) = 0;

		// [Render to vertex buffer]

	protected:

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class RenderDevice : public Singleton<RenderDevice>
	{
	public:

		ShaderModel GetShaderModel(void) { return m_shaderModel; }
		bool IsDebugContext(void) { return m_debugContext; }

	protected:
		friend class RenderSystem;

		RenderDevice(void) { }
		~RenderDevice(void) { }
		virtual bool _Init(ShaderModel _shaderModel, RenderContextProfile _profile, bool _debugContext) = 0;
		virtual SDL_Window* _GetSDLWindow(void) = 0;

		ShaderModel m_shaderModel;
		RenderContextProfile m_profile;
		bool m_debugContext;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


#endif

}