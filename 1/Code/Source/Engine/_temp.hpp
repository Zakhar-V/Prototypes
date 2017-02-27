///\file Render3D.hpp
/// Low-level graphics driver

#pragma once

#include "Object.hpp"
#include "Window.hpp"
#include "Math.hpp"

namespace ge
{

	/*
	многопоточность:
	нужно из любого потока:
		создание ресурсов
		чтение/запись текстуры (для стримминга)
		чтение/запись буфера ?
	только в потоке рендеринга
		рендеринг
	*/

	class Texture
	{
	public:

		/// remove texture from gpu
		virtual void Unload(void) = 0;
		/// begin loading
		virtual void Load(bool _async = true) = 0;
		uint GetAvailableLods(void);
	};

	/* TODO
		- отказаться от per-instance атрибутов вершины
		- вернуть 
	*/

	/*
	buffer flags
	map/unmap textures
	lodBias
	msaa resolve
	shader parameters
	*/

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	typedef Ptr<class HwObject> HwObjectPtr;
	typedef Ptr<class HwBuffer> HwBufferPtr;
	typedef Ptr<class HwVertexFormat> HwVertexFormatPtr;
	//typedef Ptr<class HwVertexArray> HwVertexArrayPtr;
	typedef Ptr<class HwTexture> HwTexturePtr;
	//typedef Ptr<class HwTextureSet> HwTextureSetPtr;
	typedef Ptr<class HwSampler> HwSamplerPtr;
	//typedef Ptr<class HwSamplerSet> HwSamplerSetPtr;
	//typedef Ptr<class HwFrameBuffer> HwFrameBufferPtr;
	typedef Ptr<class HwShaderStage> HwShaderStagePtr;
	typedef Ptr<class HwShaderProgram> HwShaderProgramPtr;
	typedef Ptr<class HwQueryBuffer> HwQueryBufferPtr;
	typedef Ptr<class HwBlendState> HwBlendStatePtr;
	typedef Ptr<class HwRasterizerState> HwRasterizerStatePtr;
	typedef Ptr<class HwDepthStencilState> HwDepthStencilStatePtr;
	//typedef Ptr<> Ptr;

	/*enum DriverFeatures
	{
		/// Support restart index of primitive for PT_*Strips* ge::PrimitiveType.
		///\since ShaderModel 4 and above
		DF_PrimitiveRestart,

		// Support per-instance vertex attributes (ge::VertexAttribDesc::divisor > 0).
		///\since ShaderModel 4 and above
		DF_InstancedArrays,

		/// Support bindless textures
		DF_TextureHandle,
	};*/

	enum CompareFunc
	{
		CF_Never, // false
		CF_Less, // <
		CF_LessEqual, // <=
		CF_Equal, // ==
		CF_NotEqual, // !=
		CF_GreaterEqual, // >=
		CF_Greater,	// >
		CF_Always, // true
	};

	/*enum PrimitiveBaseType
	{
		PBT_Unknown,
		PBT_Point,
		PBT_Line,
		PBT_Triangle,
		PBT_Patch,
	};*/

	enum PrimitiveType
	{
		PT_Points,
		PT_Lines,
		PT_LineLoop,
		PT_LineStrip,
		PT_LinesAdjacency,
		PT_LineStripAdjacency,
		PT_Triangles,
		PT_TriangleStrip,
		PT_TrianglesAdjacency, 
		PT_TriangleStripAdjacency,

		// SM5

		PT_Patches2, //!<\since ShaderModel 5 and above
		PT_Patches3, //!<\since ShaderModel 5 and above
		PT_Patches4, //!<\since ShaderModel 5 and above
		PT_Patches5, //!<\since ShaderModel 5 and above
		PT_Patches6, //!<\since ShaderModel 5 and above
		PT_Patches7, //!<\since ShaderModel 5 and above
		PT_Patches8, //!<\since ShaderModel 5 and above
	};

	enum MappingMode : uint8
	{
		MM_None = AM_None, //!< No access.
		MM_ReadOnly = AM_Read, //!< Read only.
		MM_ReadWrite = AM_ReadWrite, //!< Read and write.
		MM_WriteDiscard = AM_Write | 0x4, //!< Overwrite all mapped data.
		MM_WriteNoOverwrite = AM_Write | 0x8, //!< Write to part of mapped data.
	};

	struct HwVertexBuffer
	{
		HwBufferPtr buffer;
		uint offset = 0;
		uint stride = 0;
	};

	enum IndexFormat : uint8
	{
		IF_UShort = 2,
		IF_UInt = 4,
	};

	struct HwDrawOp
	{
		uint numElements;
		uint numInstances;
		uint baseVertex;
		uint baseInstance; //!<\note not supported in OpenGL es 3.x
	};

	struct HwDrawIndexedOp
	{
		uint numElements;
		uint numInstances;
		uint baseIndex;
		int baseVertex;
		uint baseInstance; //!<\note not supported in OpenGL es 3.x
	};

	//----------------------------------------------------------------------------//
	// HwObject
	//----------------------------------------------------------------------------//

	class HwObject : public RefCounted
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	enum BufferUsage : uint
	{
		BU_Default, //!< Default usage. Can be used for output buffer and rarely updateable data on cpu-side.
		BU_Dynamic, //!< Often updateable data on cpu-side. Cannot be used for output buffer.
		BU_Staging, //!< Data in local memory for effective transfering of data between cpu and gpu. Cannot be used for input and output buffers.
	};

	enum BufferFlag : uint
	{
		BF_Vertex,
		BF_Index,
		BF_IndirectDrawCommands,
		BF_Output, //!< This buffer can be attached to output stage (TransformFeedback, Compute). Buffer usage must be ge::BU_Default. 
	};

	class HwBuffer : public HwObject
	{
	public:
		BufferUsage GetUsage(void) { return m_usage; }
		uint GetFlags(void) { return m_flags; }
		uint GetSize(void) { return m_size; }
		virtual uint8* Map(MappingMode _mode) = 0;
		virtual uint8* Map(MappingMode _mode, uint _offset, uint _size) = 0;
		virtual void Unmap(void) = 0;
		virtual void Write(const void* _src, uint _offset, uint _size) = 0;
		virtual void Read(void* _dst, uint _offset, uint _size) = 0;
		virtual void CopyFrom(HwBuffer* _src, uint _srcOffset, uint _dstOffset, uint _size) = 0;

	protected:
		BufferUsage m_usage = BU_Default;
		uint m_size = 0;
		uint m_flags = 0;
	};

	//----------------------------------------------------------------------------//
	// HwVertexFormat
	//----------------------------------------------------------------------------//

	/// Vertex attribute pseudonym.
	enum VertexAttrib : uint8
	{
		VA_Position = 0,

		VA_Normal,
		VA_Tangent,

		VA_BoneIndices,
		VA_BoneWeights,

		VA_Color,

		// up to 2 texture custom coordinates
		VA_TexCoord0,
		VA_TexCoord1,
		//VA_TexCoord2,
		//VA_TexCoord3,

		// up to 8 object-specific attributes
		VA_Generic0,
		VA_Generic1,
		VA_Generic2,
		VA_Generic3,
		VA_Generic4,
		VA_Generic5,
		VA_Generic6,
		VA_Generic7,
	};

	/// Format of vertex attribute.
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
		MAX_VERTEX_ATTRIBS = 16,
		MAX_VERTEX_STREAMS = 8,
	};

	/// Vertex attribute description.
	struct VertexAttribDesc
	{
		VertexAttribFormat format = VAF_None; //!< The format.
		uint8 stream = 0; //!< Index of Source buffer. \see ge::HwVertexArray::SetBuffer
		uint8 offset = 0; //!< Relative offset in vertex. Must be aligned 4 bytes.
		uint8 divisor = 0; //!< Per-instance data divisor. If equal to 0, this element is per-vertex data.
	};

	/// Vertex format description.
	struct VertexFormatDesc
	{
		VertexAttribDesc attribs[MAX_VERTEX_ATTRIBS];

		VertexFormatDesc& Attrib(VertexAttrib _attrib, VertexAttribFormat _format, uint _offset, uint8 _stream = 0, uint8 _divisor = 0)
		{
			ASSERT(_attrib < MAX_VERTEX_ATTRIBS);
			ASSERT(_stream < MAX_VERTEX_STREAMS);
			attribs[_attrib].format = _format;
			attribs[_attrib].stream = _stream;
			attribs[_attrib].offset = (uint8)_offset;
			attribs[_attrib].divisor = _divisor;
			return *this;
		}
		const VertexAttribDesc& operator [] (uint _attrib) const { ASSERT(_attrib < MAX_VERTEX_ATTRIBS); return attribs[_attrib]; }
		VertexAttribDesc& operator [] (uint _attrib) { ASSERT(_attrib < MAX_VERTEX_ATTRIBS); return attribs[_attrib]; }
	};

	/// Format of vertex.
	class HwVertexFormat : public HwObject
	{
	public:
		HwVertexFormat(const VertexFormatDesc& _desc) : m_desc(_desc) { }
		const VertexFormatDesc& GetDesc(void) { return m_desc; }

	protected:
		VertexFormatDesc m_desc;
	};

	//----------------------------------------------------------------------------//
	// HwVertexArray
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// HwTexture
	//----------------------------------------------------------------------------//

	enum TextureType
	{
		TT_Buffer,
		TT_2D,
		TT_2DArray,
		TT_2DMultisample,
		//TT_2DMultisampleArray,
		TT_3D,
		TT_Cube,
	};

	enum TextureFormat : uint8
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

		TF_RGB10A2,
		TF_RG11B10F,
		TF_D24S8,
		TF_R5G6B5,

		TF_RGTC1,
		TF_RGTC2,
		TF_DXT1,
		TF_DXT1A,
		TF_DXT3,
		TF_DXT5,
	};

	enum TextureFilter : uint8
	{
		TF_None = 0,
		TF_Linear,
		TF_Bilinear,
		TF_Trilinear,
	};

	class HwTexture : public HwObject
	{
	public:
		
		virtual uint8* Map(MappingMode _mode, uint _lod, uint _index) = 0;
		virtual void Unmap(void) = 0;
		virtual void Write(const void* _data, uint _lod, uint _index) = 0;
		virtual void Read(void* _data, uint _lod, uint _index) = 0;
		
		/// Get texture buffer view. \return nullptr if type is not ge::TT_Buffer.
		virtual HwBuffer* GetBufferView(void) = 0;

		virtual void Resolve(HwTexture* _dst, uint _dstLayer) = 0;

	protected:

		TextureFormat m_format;
		uint m_width;
		uint m_height;
		uint m_depth;
		//uint m_arraySize;
	};

	//----------------------------------------------------------------------------//
	// HwSampler
	//----------------------------------------------------------------------------//

	enum TextureWrap
	{
		TW_Repeat,
		TW_Clamp,
	};

	struct SamplerDesc
	{
		TextureFilter minFilter = TF_Trilinear;
		TextureFilter magFilter = TF_Linear;
		TextureWrap wrap = TW_Repeat;
		TextureWrap wrapZ = TW_Repeat;
		CompareFunc cmpFunc = CF_Never;
		float lodBias = 0; // ???
		bool fixedAnisotropy = false;
		bool fixedFilter = false;
	};

	class HwSampler : public HwObject
	{
	public:
		HwSampler(const SamplerDesc& _desc) : m_desc(_desc) { }

		const SamplerDesc& GetDesc(void) { return m_desc; }
		virtual void SetFilter(TextureFilter _min, TextureFilter _mag) = 0;
		virtual void SetAnisotropy(uint _value) = 0;

	protected:
		SamplerDesc m_desc;
	};

	//----------------------------------------------------------------------------//
	// HwFrameBuffer
	//----------------------------------------------------------------------------//

	class HwFrameBuffer : public HwObject // remove
	{
	public:

		virtual void SetSize(uint _width, uint _height) = 0;
		virtual void SetMsaa(uint _count) = 0;
		virtual void SetTexture(uint _slot, HwTexture* _tex, int _layer = -1) = 0;
		virtual void SetDepthStencil(HwTexture* _tex, int _layer = -1) = 0;
		virtual bool Begin(void) = 0;
		virtual void End(void) = 0;

	protected:
		uint m_width = 0;
		uint m_height = 0;
		uint m_msaa = 0;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	enum ShaderType
	{
		ST_Vertex,
		ST_Pixel,
		ST_Geometry,
		ST_Hull, //!<\since ShaderModel 5 and above 
		ST_Domain, //!<\since ShaderModel 5 and above 
		ST_Compute, //!<\since ShaderModel 5 and above 
	};

	class HwShaderStage : public HwObject
	{
	public:

		void GetByteCode(Array<uint8>& _dst);

	protected:
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class HwShaderProgram : public HwObject
	{
	public:

		virtual HwShaderStage* GetStage(ShaderType _type) = 0;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class HwQuery : public HwObject
	{
		
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class HwBlendState : public HwObject
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class HwRasterizerState : public HwObject
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class HwDepthStencilState : public HwObject
	{

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////


	enum HwPipelineStateType
	{
		HPST_Graphics,
		HPST_Compute,
	};


	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct HwPipelineStateDesc
	{
		HwShaderProgramPtr program;
		HwVertexFormatPtr vertexFormat;
		HwBlendStatePtr blend;
		//Hw
	};

	class HwPipelineState : public HwObject
	{
	public:

		// HwShaderProgram
		//		HwVertexFormat (output)
		// HwVertexFormat (input)
		// HwBlendState
		// HwRasterizerState
		// HwDepthStencilState
		// PrimitiveType (?)
		// HwSampler[] (?)
		// msaa

	protected:
	};


	class HwQueryBuffer
	{
		// num queries
	};


	class RenderContext
	{
		// [BUFFERS]
		virtual void CopyBuffer(HwBuffer* _src, uint _srcOffset, HwBuffer* _dst, uint _dstOffset, uint _size) = 0;

		// [TEXTURES]

		virtual void ResolveMultisampleTexture(HwTexture* _srcMultisample, HwTexture* _dst) = 0;
		virtual	void CopyTexture(...) = 0;




		virtual void SetTexture(uint _slot, HwTexture* _texture) = 0;
		virtual void SetTextures(uint _firstSlot, uint _count, HwTexturePtr* _texture) = 0;

		virtual void SetSampler(uint _slot, HwSampler* _sampler) = 0;
		virtual void SetSamplers(uint _firstSlot, uint _count, HwSamplerPtr* _sampler) = 0;

		virtual void SetProgram() = 0;
		//virtual 

		// vertex format

		virtual void SetPrimitiveType() = 0;
		virtual void Draw() = 0;
		virtual void DrawIndexed() = 0;
		virtual void DrawIndirect() = 0;


		//virtual void SetVertexArray)(HwVertexArray* _array) = 0;
		virtual void SetVertexBuffer(HwBuffer* _buffer, uint _offset = 0, uint _stride = 0) = 0;
		virtual void SetVertexBuffers(uint _firstSlot, uint _count, const HwVertexBuffer* _buffers) = 0;
		virtual void SetIndexBuffer(HwBuffer* _buffer, IndexFormat _format, uint _offset = 0) = 0;



		virtual void SetViewports() = 0;
		virtual void SetScissorRects() = 0;

		virtual void SetStencilRef(uint8 _ref) = 0;

		virtual void SetOutputBuffers(uint _firstSlot, uint _count, HwBufferPtr* _buffers) = 0;

		virtual void SetUniformBuffer(uint _slot, HwBuffer* _buffer, uint _offset = 0) = 0;

		//virtual void SetVert(HwVertexArray* _vertices, HwBuffer* _indexBuffer) = 0;

		//virtual void Dispatch(uint _x, uint _y, uint _z) = 0;
		//virtual void ResourceBarrier(uint _count, HwBarrier** _barriers) = 0;

		virtual void SetRenderTargets() = 0;
		virtual void ClearFrameBuffers() = 0;

		virtual void BeginTransformFeedback() = 0;
		virtual void EndTransformFeedback() = 0;

		virtual void BeginQuery(HwQueryBuffer* _group, uint _index) = 0;
		virtual void EndQuery(void) = 0;
		virtual void ResolveQueryData(HwQueryBuffer* _group, uint _start, uint _count) = 0;

		virtual void BeginConditionalRender(HwQueryBuffer* _group, uint _index) = 0;
		virtual void EndConditionalRender(void) = 0;

	};


	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	enum DriverType
	{
		DT_Unknown,
		DT_OpenGL,
		DT_OpenGL_ES,
		DT_Direct3D11,
		//DT_Direct3D12,
		//DT_Vulkan,
	};

	enum ShaderModel
	{
		SM_Unknown = 0,
		SM4 = 4,
		SM5,
	};

	class RenderDevice : public Singleton<RenderDevice>
	{
	public:




		virtual HwBufferPtr CreateBuffer() = 0;
		virtual HwVertexFormatPtr CreateVertexFormat() = 0;
		//virtual HwVertexArrayPtr CreateVertexArray() = 0;
		virtual HwTexturePtr CreateTexture() = 0;
		//virtual HwFrameBufferPtr CreateFrameBuffer() = 0;

		virtual RenderContext* CreateRenderContext(void) = 0;
		//[command list queue]

		virtual HwShaderStagePtr CompileShader(ShaderType _type, const char* _src, const char* _entry, HwVertexFormat* _vf = nullptr) = 0;
		virtual HwShaderStagePtr CreateShader(const void* _bytecode) = 0;
		virtual HwShaderProgramPtr CreateProgram(HwShaderStage* _vs, HwShaderStage* _ps, HwShaderStage* _gs, HwShaderStage* _hs, HwShaderStage* _ds, HwShaderStage* _cs) = 0;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}

