///\file RenderDriver.hpp
/// Low-level generic renderer

#pragma once

#include "Resource.hpp"
#include "Window.hpp"
#include "Math.hpp"

#if _DEBUG_RC
#	define DEBUG_RC
#endif

#ifdef _USE_GL
#	define USE_GL
#endif

#ifdef _USE_GLES
#	define USE_GLES
#endif

#if defined(_USE_D3D11) && defined(_WIN32)
#	define USE_D3D11
#endif

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class RenderSystem;
	typedef Ptr<RenderSystem> RenderSystemPtr;

#define gRenderSystem ge::RenderSystem::Get()

	enum RenderSystemType : uint
	{
		RST_Null = 0,
		RST_GL,
		RST_GLES,
		RST_D3D11,
	};

	enum ShaderModel : uint
	{
		SM_Null,

		/// ShaderModel 3.0
		/// * D3D11 9.3, GL 2.1, GLES 2.0
		/// * AMD Radeon R500 Series (Radeon X1xxx)
		/// * NVIDIA GeForce 6 Series
		SM3,

		/// ShaderModel 4.1
		/// * D3D11 10.1, GL 3.3
		/// * AMD Radeon R600 Series (Radeon HD 3xxx)
		/// * NVIDIA GeForce 200 Series
		SM4,

		/// ShaderModel 5.0
		/// * D3D11 11.0, GL 4.3
		/// * AMD Radeon R800 Series (Radeon HD 5xxx)
		/// * NVIDIA GeForce 400 Series
		SM5,
	};

	enum RenderSystemFlags : uint
	{
		RSF_AnyShaderModel = 0x1, ///!< Create with available shader model, if requested type is not supported.
		RSF_AnyType = 0x2, ///!< Create available render system, if requested type is not available.
		RSF_sRGB = 0x4, ///!< Support sRGB color space.
		RSF_DebugOutput = 0x8, ///!< Create debug context.
		RSF_GL_CoreProfile = 0x10, ///!< Create core profile context (for OpenGL render system only).
#ifdef DEBUG_RC
		RSF_Default = RSF_AnyType | RSF_AnyShaderModel | RSF_sRGB | RSF_DebugOutput, ///!< Default flags.
#else
		RSF_Default = RSF_AnyType | RSF_AnyShaderModel | RSF_sRGB, ///!< Default flags.
#endif
	};

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// WIP
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

#pragma region WIP

	class HardwareBuffer;
	typedef Ptr<HardwareBuffer> HardwareBufferPtr;
	class VertexArray;
	typedef Ptr<VertexArray> VertexArrayPtr;

	class Shader;
	typedef Ptr<Shader> ShaderPtr;
	class ShaderStage;
	typedef Ptr<ShaderStage> ShaderStagePtr;
	class ShaderProgram;
	typedef Ptr<ShaderProgram> ShaderProgramPtr;
	class Texture;
	class SamplerState;
	class BlendState;
	class DepthState;
	class RasterizerState;
	class Buffer;
	class FrameBuffer;
	class RenderTarget;

	enum IndexType : uint8
	{
		IF_UShort = 2,
		IF_UInt = 4,
	};

	enum PrimitiveType : uint8
	{
		PT_Points,
		PT_Lines,
		PT_LineStrip,
		PT_LineStrips, ///!<\since ShaderModel 4 and above
		PT_LineLoop,
		PT_LinesAdjacency, ///!<\since ShaderModel 4 and above
		PT_LineStripAdjacency, ///!<\since ShaderModel 4 and above
		PT_LineStripsAdjacency, ///!<\since ShaderModel 4 and above
		PT_LinePatches, ///!<\since ShaderModel 5 and above
		PT_Triangles,
		PT_TriangleStrip,
		PT_TriangleStrips, ///!<\since ShaderModel 4 and above
		PT_TrianglesAdjacency, ///!<\since ShaderModel 4 and above
		PT_TriangleStripAdjacency, ///!<\since ShaderModel 4 and above
		PT_TriangleStripsAdjacency, ///!<\since ShaderModel 4 and above
		PT_TrianglePatches, ///!<\since ShaderModel 5 and above
		PT_QuadPatches, ///!<\since ShaderModel 5 and above
		PT_Patches5, ///!<\since ShaderModel 5 and above
		PT_Patches6, ///!<\since ShaderModel 5 and above
		PT_Patches7, ///!<\since ShaderModel 5 and above
		PT_Patches8, ///!<\since ShaderModel 5 and above
	};

	//----------------------------------------------------------------------------//
	// Vertex Format
	//----------------------------------------------------------------------------//

	enum VertexAttrib
	{
		VA_Position = 0,

		VA_Normal,
		VA_Tangent,

		VA_BoneIndices,
		VA_BoneWeights,

		VA_Color,

		// up to 4 texture coordinates
		VA_TexCoord0,
		VA_TexCoord1,
		VA_TexCoord2,
		VA_TexCoord3,

		// up to 6 generic attributes
		VA_Generic0,
		VA_Generic1,
		VA_Generic2,
		VA_Generic3,
		VA_Generic4,
		VA_Generic5,

		MAX_VERTEX_ATTRIBS,	// 16
	};

	enum VertexAttribFormat : uint8
	{
		VAF_None = 0,
		VAF_Half2,
		VAF_Half4,
		VAF_Float,
		VAF_Float2,
		VAF_Float3,
		VAF_Float4,
		VAF_UByte4,
		VAF_UByte4N,
		VAF_Byte4N,
		VAF_UShort2,
		VAF_UShort2N,
		VAF_UShort4,
		VAF_UShort4N,
	};

	enum : uint
	{
		MAX_VERTEX_STREAMS = 8,
	};

	struct VertexAttribDesc
	{
		VertexAttribFormat format = VAF_None;
		uint8 stream = 0;
		uint16 offset = 0;
	};

	struct VertexFormatDesc
	{
		VertexAttribDesc attribs[MAX_VERTEX_ATTRIBS];

		VertexFormatDesc& Attrib(VertexAttrib _attrib, VertexAttribFormat _format, uint _offset, uint8 _stream = 0)
		{
			ASSERT(_attrib < MAX_VERTEX_ATTRIBS);
			ASSERT(_stream < MAX_VERTEX_STREAMS);
			attribs[_attrib].format = _format;
			attribs[_attrib].stream = _stream;
			attribs[_attrib].offset = (uint16)_offset;
			return *this;
		}
		const VertexAttribDesc operator [] (uint _attrib) const
		{
			ASSERT(_attrib < MAX_VERTEX_ATTRIBS);
			return attribs[_attrib];
		}
		VertexAttribDesc operator [] (uint _attrib)
		{
			ASSERT(_attrib < MAX_VERTEX_ATTRIBS);
			return attribs[_attrib];
		}
	};

	class ENGINE_API VertexFormat : public NonCopyable
	{
	public:
		VertexFormat(const VertexFormatDesc& _desc) : m_desc(_desc) { }
		virtual ~VertexFormat(void) { }

		const VertexFormatDesc& GetDesc(void) { return m_desc; }

	protected:

		VertexFormatDesc m_desc;
	};

	//----------------------------------------------------------------------------//
	// GEOMETRY
	//----------------------------------------------------------------------------//

	enum HardwareBufferUsage : uint8
	{
		HBU_Default, ///!< Default usage. Can be used for output buffer and rarely updateable data on cpu-side.
		HBU_Dynamic, ///!< Often updateable data on cpu-side. Cannot be used for output buffer.
		HBU_Staging, ///!< Data in local memory for effective transfering of data between cpu and gpu. Cannot be used for input and output buffers.
	};

	enum MappingMode : uint8
	{
		MM_None = AM_None, ///!< No access.
		MM_ReadOnly = AM_Read, ///!< Read only.
		MM_ReadWrite = AM_ReadWrite, ///!< Read and write.
		MM_WriteDiscard = AM_Write | 0x4, ///!< Overwrite all mapped data.
		MM_WriteNoOverwrite = AM_Write | 0x8, ///!< Write to part of mapped data.
	};

	class HardwareBuffer : public Object
	{
	public:
		HardwareBuffer(HardwareBufferUsage _usage = HBU_Default, AccessMode _cpuAccess = AM_ReadWrite, uint _size = 0) : 
			m_usage(_usage), m_cpuAccess(_cpuAccess), m_size(_size) { }

		HardwareBufferUsage GetUsage(void) { return m_usage; }
		uint GetSize(void) { return m_size; }
		virtual uint8* Map(MappingMode _mode) = 0;
		virtual uint8* Map(MappingMode _mode, uint _offset, uint _size) = 0;
		virtual void Unmap(void) = 0;
		virtual void Write(const void* _src, uint _offset, uint _size) = 0;
		virtual void Read(void* _dst, uint _offset, uint _size) = 0;
		virtual void CopyFrom(HardwareBuffer* _src, uint _srcOffset, uint _dstOffset, uint _size) = 0;

	protected:
		uint m_size;
		HardwareBufferUsage m_usage;
		AccessMode m_cpuAccess;
	};

	class VertexArray : public Object
	{
	public:

		virtual void SetBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset = 0, uint _stride = 0) = 0;
		virtual VertexFormat* GetFormat(void) = 0;

	protected:
	};

	//----------------------------------------------------------------------------//
	// TEXTURE
	//----------------------------------------------------------------------------//

	enum TextureFormat
	{
		TF_Unknown = 0,
		
		TF_R8,
		TF_RG8,
		TF_RGBA8,
		
		TF_R16F,
		TF_RG16F,
		TF_RGBA16F,

		TF_R32F,
		TF_RG32F,
		TF_RGBA32F,

		TF_RGB10A2, ///!<\note for rendertarget only
		TF_RG11B10F, ///!<\note for rendertarget only
		TF_D24S8, ///!<\note for rendertarget only
		TF_R5G6B5, ///!<\note for rendertarget only

		// compressed,
		TF_RGTC1,
		TF_RGTC2,
		TF_DXT1,
		TF_DXT1A,
		TF_DXT3,
		TF_DXT5,
		//TODO: more compressed formats for mobile devices
	};

	/*struct RawImageData
	{
		uint8* pixels = nullptr;
		TextureFormat format = TF_Unknown;
		uint bpp = 0; ///!< bits per pixel
		uint width = 0;
		uint height = 0;
		uint depth = 0;
		uint mips = 0;
		uint memorySize = 0;
	};*/

	enum TextureType
	{
		TT_Unknown,
		TT_2D,
		TT_3D,
		TT_Cube,
		// SM3 and above
		TT_Array,
		TT_2DMultisample,
	};

	enum TextureUsage
	{
		TU_Resource,
		TU_RenderTarget,
	};

	class TexturePool : public Singleton<TexturePool>
	{
	public:

	protected:
		
		Mutex m_mutex;

	};

	class TextureObject : public Object
	{
	public:

		TextureObject(TextureType _type, TextureUsage _usage, TextureFormat _format, uint _flags);

		//virtual void ResizeRenderTarget(uint _width, uint _height, uint _msaa = 0);
		

		virtual void WriteLod(uint _lod, const void* _data) = 0;
		

	protected:

		TextureType m_type;
		TextureFormat m_format;
	};

	class Texture : public Resource
	{
	public:

	protected:
	};

	class RenderTarget : public Object
	{
	public:

		virtual TextureObject* GetTexture(void) = 0;

	protected:

	};

	class FrameBuffer : public Object
	{
	public:

	protected:
	};



	//----------------------------------------------------------------------------//
	// SHADER
	//----------------------------------------------------------------------------//

	//class 

	enum ShaderType
	{
		ST_Vertex,
		ST_Fragment,
		//ST_Geometry,
		//ST_TessEval,
		//ST_TessComp,
		//ST_Compute,
	};


	class Shader : public Resource
	{
	public:

	protected:

	};

	class ShaderStage : public Object
	{

	};

	class ShaderProgram : public Object
	{

	};




	class GpuResource : public Object
	{
	public:

	protected:
	};

	class GpuBuffer : public GpuResource
	{
	public:

	protected:
	};

	class GpuMesh : public GpuResource
	{
	public:

	protected:
	};

	class GpuRenderTarget : public GpuResource
	{
	public:

	protected:
	};

	class GpuProgram : public GpuResource
	{
	public:

	protected:
	};

	class GpuQuery : public GpuResource
	{
	public:

	protected:
	};

	class GpuBlendState : public GpuResource
	{
	public:

	protected:
	};

	class GpuDepthState : public GpuResource
	{
	public:

	protected:
	};

	class GpuRasterizerState : public GpuResource
	{
	public:

	protected:
	};


	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////



#pragma endregion // WIP

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	//----------------------------------------------------------------------------//
	// Renderer
	//----------------------------------------------------------------------------//

	class ENGINE_API RenderSystem : public Object, public Singleton<RenderSystem>
	{
	public:
		OBJECT(RenderSystem);

		RenderSystem(void);
		~RenderSystem(void);

		// [CAPS]
		uint GetMaxRenderTargets(void) { return m_maxRenderTargets; }

		virtual void DebugPoint(const char* _title) = 0;

		virtual bool RegisterThread(void) = 0;
		virtual void UnregisterThread(void) = 0;

		virtual void BeginFrame(void) = 0;
		virtual void EndFrame(void) = 0;

		/// Create window. Please use ge::WindowSystem::CreateWindow instead it.
		virtual WindowPtr CreateWindow(void* _externalWindow = nullptr) = 0;

		virtual VertexFormat* CreateVertexFormat(const VertexFormatDesc& _desc) = 0;
		virtual HardwareBufferPtr CreateBuffer(HardwareBufferUsage _usage, AccessMode _cpuAccess, uint _size, const void* _data = nullptr) = 0;
		virtual VertexArrayPtr CreateVertexArray(VertexFormat* _format) = 0;

		virtual void SetGeometry(VertexArray* _vertices, HardwareBuffer* _indices, IndexType _indexType = IF_UShort, uint _indexOffset = 0) = 0;

		virtual void Draw(PrimitiveType _type, uint _baseVertex, uint _numElements) = 0;
		virtual void DrawInstanced(PrimitiveType _type, uint _baseVertex, uint _numElements, uint _numInstances = 1) = 0;
		virtual void DrawIndexed(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements) = 0;
		virtual void DrawIndexedInstanced(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements, uint _numInstances = 1) = 0;

		static RenderSystemPtr Create(RenderSystemType _type = RST_Null, ShaderModel _shaderModel = SM_Null, uint _flags = RSF_Default);

	protected:

		RenderSystemType m_type;
		ShaderModel m_shaderModel;
		uint m_maxRenderTargets;

		//uint m_supportedTextureFormats;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//


}
