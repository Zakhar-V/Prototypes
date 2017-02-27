#pragma once

#include "Object.hpp"
#include "Math.hpp"

struct SDL_Window;

namespace Engine
{
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// new API
	//----------------------------------------------------------------------------//

	/*namespace xxx
	{
		typedef Ptr<class GpuBufferObject> GpuBufferObjectPtr;
		class GpuVertexFormat;
		typedef Ptr<class GpuTextureObject> GpuTextureObjectPtr;
		typedef Ptr<class GpuSamplerObject> GpuSamplerObjectPtr;
		typedef Ptr<class GpuShaderObject> GpuShaderObjectPtr;
		class GpuDepthStencilState;
		class GpuBlendState;
		class GpuRasterizerState;

		class GpuDepthStencilState
		{

		};

		class GpuBlendState
		{

		};

		class GpuRasterizerState
		{

		};

		class GpuBufferObject
		{

		};

		class GpuVertexFormat
		{

		};

		class GpuTextureObject
		{

		};
		
		class GpuSamplerObject
		{

		};

		class GpuShaderObject
		{

		};
	
		class GpuRenderContext
		{

		};

		class GpuRenderSystem : public Singleton<RenderSystem>
		{

		};

		//////////////////////////////////////////////////////

		class HwBuffer
		{

		};

		class HwTexture
		{

		};

		class HwSampler
		{

		};

		class HwRenderContext
		{

		};

		//////////////////////////////////////////////////////

		

		class GpuTextureObject
		{

		};

		//class Graphics

		class Graphics
		{

		};
	}	 */

	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gRenderSystem Engine::RenderSystem::Get()
#define gRenderFeatures Engine::RenderSystem::Get()->GetFeatures()

	typedef Ptr<class HardwareBuffer> HardwareBufferPtr;
	typedef Ptr<class HardwareShaderStage> HardwareShaderStagePtr;


	enum RenderSystemType
	{
		RST_Unknown,
		RST_D3D11,
		//RST_GL,
		//RST_D3D12,
		//RST_Vulkan,
	};

	enum RenderSystemVersion : uint
	{
		RSV_Unknown = 0,
		RSV_10_1 = 101, // DX 10.1 / GL 3.3
		RSV_11_0 = 110, // DX 11.0 / GL 4.3
		//RSV_12 = 120, // DX 12.0 / Vulkan
	};

	enum NativeShaderLanguage
	{
		NSL_Unknown = 0,
		NSL_HLSL,
		NSL_GLSL,
		//NSL_GLSL_ES,
	};

	struct RenderSystemFeatures
	{
		// [System]

		RenderSystemType type = RST_Unknown;
		RenderSystemVersion version = RSV_Unknown;

		// [Adapter]

		String adapterName = "Unknown";
		size_t dedicatedVideoMemory = 1024 * 1024 * 512; // 512 MB by default
		// vendor ?
		
		// [Vertex]

		uint maxVertexAttribDivisor = (uint)-1;
		uint maxVertexStreams = 16;

		// [Shader]

		NativeShaderLanguage shaderLang = NSL_Unknown;
		uint shaderVersion = 0; // HLSL(4.1 = 41, 5.0 = 50), GL(330, 430)
	};

	//----------------------------------------------------------------------------//
	// HardwareBuffer
	//----------------------------------------------------------------------------//

	enum HardwareBufferType	: uint8
	{
		HBT_Vertex,
		HBT_Index,
		HBT_Uniform,
		HBT_DrawIndirect,
		HBT_Texture, //!<\note cannot be created directly. use Engine::RenderSystem::CreateTexture with Engine::TT_Buffer.

		//BUFFER_SHADER_STORAGE,
	};

	enum HardwareBufferUsage : uint8
	{
		HBU_Default, //!< optimized for usage on GPU. \note buffer can be read or updated on CPU (not very much effective)
		HBU_DynamicRead, //!< optimized for reading on CPU. \note buffer cannot be updated on CPU.
		HBU_DynamicWrite, //!< optimized for overwriting on CPU. Use Engine::MM_WriteDiscard for updating buffer. \note buffer cannot be attached to output stage. \note buffer cannot be read on CPU.
	};

	enum MappingMode : uint8
	{
		MM_None = AM_None,
		MM_Read = AM_Read,
		MM_Write = AM_Write, //!< For partial write of mapped data. Not very much effective, because required read of actual data before mapping data to CPU.
		MM_ReadWrite = AM_ReadWrite,
		MM_WriteDiscard = AM_Write | 0x4, //!< For full overwrite of mapped data. Effective for buffer with Engine::HBU_DynamicWrite.
	};

	class HardwareBuffer : public RefCounted
	{
	public:

		HardwareBufferUsage GetUsage(void) { return m_usage; }
		uint GetSize(void) { return m_size; }
		uint GetElementSize(void) { return m_elementSize; }

		virtual uint8* Map(MappingMode _mode, uint _offset, uint _size) = 0;
		virtual void Unmap(void) = 0;

	protected:
		HardwareBuffer(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize);
		~HardwareBuffer(void);

		HardwareBufferType m_type;
		HardwareBufferUsage m_usage;
		uint m_size;
		uint m_elementSize;
	};

	//----------------------------------------------------------------------------//
	// VertexFormat
	//----------------------------------------------------------------------------//

	enum : uint
	{
		MAX_VERTEX_STREAMS = 8,
		MAX_VERTEX_ATTRIBS = 16,
		MAX_VERTEX_AUX_ATTRIBS = 5,
		MAX_VERTEX_TEXCOORDS = 4,
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
		uint16 divisor = 0;
		uint32 offset = 0;
	};

	struct VertexFormatDesc
	{
		VertexFormatDesc(void) { }
		VertexFormatDesc& operator () (VertexAttrib _attrib, VertexAttribType _type, uint8 _stream, ptrdiff_t _offset, uint16 _divisor = 0)
		{
			ASSERT(_stream < MAX_VERTEX_STREAMS);

			VertexAttribDesc& _va = attribs[_attrib];
			_va.type = _type;
			_va.stream = _stream;
			_va.offset = (uint32)_offset;
			_va.divisor = (uint16)_divisor;

			return *this;
		}
		const VertexAttribDesc& operator [] (uint _index) const { return attribs[_index]; }
		VertexAttribDesc& operator [] (uint _index) { return attribs[_index]; }

		VertexAttribDesc attribs[MAX_VERTEX_ATTRIBS];

		static const VertexFormatDesc Empty;
	};

	class VertexFormat : public NonCopyable
	{
	public:
		const VertexFormatDesc& GetDesc(void) { return m_desc; }

	protected:
		VertexFormat(const VertexFormatDesc& _desc);
		virtual ~VertexFormat(void);

		VertexFormatDesc m_desc;
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
		TT_Buffer
	};

	enum PixelFormat
	{
		PF_UnknownType,

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

	class Texture : public RefCounted
	{
	public:

	protected:
		uint m_width = 0;
		uint m_height = 0;
		uint m_depth = 0;
		//TextureType m_type = TEXTURE_UNKNOWN_TYPE;
		//PixelFormat m_format = PIXEL_UNKNOWN_TYPE;
	};

	//----------------------------------------------------------------------------//
	// Sampler
	//----------------------------------------------------------------------------//

	class Sampler : public RefCounted
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	// HardwareShaderStage
	//----------------------------------------------------------------------------//

	enum ShaderStageType
	{
		SST_Vertex,
		SST_Fragment,
		SST_Geometry,
	};

	class HardwareShaderStage : public RefCounted
	{
	public:
		ShaderStageType GetType(void) { return m_type; }

	protected:
		HardwareShaderStage(ShaderStageType _type);
		~HardwareShaderStage(void);

		ShaderStageType	m_type;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	enum ShaderType
	{
		ST_Vertex,
		ST_Fragment,
		ST_Geometry,
	};

	class ShaderSource : public RefCounted
	{
	public:

	protected:
	};

	class Shader : public RefCounted
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	enum FrameBufferType : uint
	{
		FBT_Color = 0x1,
		FBT_Depth = 0x2,
		FBT_Stencil = 0x4,
		FBT_DepthStencil = FBT_Depth | FBT_Stencil,
		FBT_All = FBT_Color | FBT_DepthStencil,
	};

	enum IndexFormat : uint
	{
		IF_UShort = 2,
		IF_UInt = 4,
	};

	enum PrimitiveType
	{
		PT_Points,
		PT_Lines,
		PT_LineStrip,
		PT_LineLoop,
		PT_Triangles,
		PT_TriangleStrip,
		// ... patches, adjacency
	};

	struct DrawIndirectCommand
	{
		uint numVertices;
		uint numInstances;
		uint firstVertex;
		uint baseInstance;
	};

	struct DrawIndexedIndirectCommand
	{
		uint numIndices;
		uint numInstances;
		uint firstIndex;
		int baseVertex;
		uint baseInstance;
	};

	//----------------------------------------------------------------------------//
	// RenderContext
	//----------------------------------------------------------------------------//

	class RenderContext	: public NonCopyable
	{
	public:

		bool IsDeferred(void) { return m_deferred; }

		virtual void BeginCommands(void) = 0;
		virtual void EndCommands(void) = 0;

		virtual void SetVertexFormat(VertexFormat* _format) = 0;
		virtual void SetVertexBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset, uint _stride) = 0;
		virtual void SetIndexBuffer(HardwareBuffer* _buffer, IndexFormat _format, uint _offset) = 0;
		virtual void SetPrimitiveType(PrimitiveType _type) = 0;

		virtual void Draw(uint _numVertices, uint _numInstances, uint _firstVertex, uint _baseInstance = 0) = 0;
		virtual void DrawIndexed(uint _numIndices, uint _numInstances, uint _firstIndex, int _baseVertex, uint _baseInstance = 0) = 0;
		virtual void DrawIndirect(HardwareBuffer* _buffer, uint _offset = 0) = 0;
		virtual void DrawIndexedIndirect(HardwareBuffer* _buffer, uint _offset = 0) = 0;

	protected:
		RenderContext(bool _deferred = false) : m_deferred(_deferred) { }
		virtual ~RenderContext(void) { }

		bool m_deferred;
	};

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	class RenderSystem : public Singleton<RenderSystem>, public RenderContext
	{
	public:

		static bool Create(void);
		static void Destroy(void);


		SDL_Window* GetSDLWindow(void) { return m_window; }
		const RenderSystemFeatures& GetFeatures(void) { return m_features; }

		virtual void BeginFrame(void);
		virtual void EndFrame(void);


		/// Add unique vertex format.
		virtual VertexFormat* AddVertexFormat(const VertexFormatDesc& _desc) = 0;

		///\param[in] _elementSize specify size of each element in buffer. It's obligatory when _type is Engine::HBU_Uniform, use as hint otherwise. 
		virtual HardwareBufferPtr CreateBuffer(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize, const void* _data = nullptr) = 0;


	protected:
		RenderSystem(void);
		virtual ~RenderSystem(void);

		bool _Init(void);
		void _Destroy(void);
		virtual bool _InitDriver(void) { return true; }
		virtual void _DestroyDriver(void) { }

		SDL_Window* m_window;
		RenderSystemFeatures m_features;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
