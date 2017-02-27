#pragma once


#include "Base.hpp"

#ifdef _DEBUG
#define DEBUG_RC // Enable debugging
#endif

#define USE_GL
//#define USE_D3D11
//#define USE_D3D12

#define MULTIRENDER


struct SDL_Window;

namespace Engine
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#define gRenderDevice Engine::RenderDevice::Get()
#define gShaderModel Engine::RenderDevice::Get()->GetShaderModel()

#ifdef MULTIRENDER
#	define MR_OVERRIDE override
#	define MR_BASECLASS(A, B) A
#else
#	define MR_BASECLASS(A, B) B
#	define MR_OVERRIDE
#endif

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//


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

#ifdef MULTIRENDER

	class Buffer;
	class VertexFormat;

	class ShaderStage;
	class GpuProgram;


	class DepthStencilState;
	class RasterizerState;
	class BlendState;

	class FramebufferState;
	class RenderTarget;
	class DepthStencilTarget;


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
		virtual uint GetMemoryGroup(void) = 0;
	};

	class InputLayout
	{
	public:

	protected:
	};

	class HardwareVertexFormat : public NonCopyable
	{
	public:
		virtual const VertexFormatDesc& GetDesc(void) = 0;

	protected:
		virtual ~HardwareVertexFormat(void) { }
	};

	class HardwareShader
	{

	};

	class HardwareProgram
	{

	};

	class RenderContext : public Singleton<RenderContext>
	{

	};

	class RenderDevice : public Singleton<RenderDevice>
	{

	};


	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//



	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#endif


	class CommandList
	{
		virtual void SetPipelineState() = 0;

		virtual void DrawIndexed(uint _baseIndex, uint _count, uint _baseVertex) = 0;
		virtual void Draw(uint _baseIndex, uint _count) = 0;
		virtual void DrawIndexedInstanced(uint _baseIndex, uint _count, uint _baseVertex, uint _baseInstance, uint _numInstances) = 0;
		virtual void DrawInstanced(uint _count, uint _baseVertex, uint _baseInstance, uint _numInstances) = 0;
		virtual void SetIndexBuffer() = 0;
		virtual void SetVertexBuffer() = 0; // stride?
		virtual void SetUniformBuffer() = 0;
		virtual void SetBlendColor(const Vec4& _color) = 0;
		virtual void SetStencilRef(uint _front, uint _back) = 0;
		virtual void SetLineWidth(float _width) = 0;
		virtual void SetPolygonOffset(float _scale, float _bias) = 0;
		virtual void SetAlphaRef(float _ref) = 0; // deprecated?
		virtual void SetViewport(const Recti& _rect) = 0;
		virtual void SetScissor(const Recti& _rect) = 0;
		virtual void SetFrontFace(bool _ccw) = 0;

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
