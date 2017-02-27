#pragma once

#include "Math.hpp"
#include "Object.hpp"
#include "Window.hpp"

namespace ge
{
	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	/*
	концепция многопоточного рендеринга:
	1. Рендеринг.
		Приложение заполняет списки команд и выполняет их. 
		Создавать списки можно паралельно. 
		Один список можно заполнять только в одном потоке.
		Списки привязываются к группе при создании. Все списки в одной группе выполняются последовательно.
		После заполнения списка, он выполняется (асинхронно, в потоке рендеринга) и потом очищается для нового заполнения.
		Список можно заново заполнять сразу после выполнения (без задержки и ожидания выполнения). 
	2. Изменение ресурсов (текстуры, буфера)
		Работать с ресурсом можно из любого потока.
		Для синхронизации используется GpuFence.

	3. синхронизация:

	*/
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#define gRenderSystem ge::RenderSystem::Get()

	enum : uint
	{
		MAX_RENDER_THREADS = 6
	};

	typedef Ptr<class Texture> TexturePtr;

	enum RenderSystemType : uint8
	{
		RST_Unknown = 0,
		RST_OpenGL,
		RST_OpenGLES,
		RST_Direct3D11,
		//RST_Direct3D12,
		//RST_Vulkan,
	};

	enum ShaderModel : uint8
	{
		SM_Unknown = 0,
		SM_4_1 = 41,
		SM_5_0 = 50,
		SM_5_1 = 51,
	};

	enum RenderSystemFlags : uint
	{
		RSF_AnyShaderModel = 0x1, ///!< Create with available shader model, if requested type is not supported.
		RSF_AnyType = 0x2, ///!< Create available render system, if requested type is not available.
		RSF_sRGB = 0x4, ///!< Support sRGB color space.
		RSF_DebugOutput = 0x8, ///!< Create debug context.
		RSF_GL_CoreProfile = 0x10, ///!< Create core profile context (for OpenGL render system only).
#ifdef DEBUG_RC
		RDF_Default = RSF_AnyType | RSF_AnyShaderModel | RSF_sRGB | RSF_DebugOutput, ///!< Default flags.
#else
		RSF_Default = RSF_AnyType | RSF_AnyShaderModel | RSF_sRGB, ///!< Default flags.
#endif
	};

	enum GpuDataFormat : uint8
	{
		GDF_Unknown = 0,

		GDF_UByte1N,
		GDF_UByte2N,
		GDF_UByte4N,
		GDF_UByte4,
		GDF_Byte4N,

		GDF_UShort2,
		GDF_UShort2N,
		GDF_UShort4,
		GDF_UShort4N,

		GDF_Int1,
		GDF_Int2,
		GDF_Int3,
		GDF_Int4,

		GDF_Half1,
		GDF_Half2,
		GDF_Half4,

		GDF_Float1,
		GDF_Float2,
		GDF_Float3,
		GDF_Float4,

		GDF_Float3x4,
		GDF_Float4x4,

		GDF_RGB10A2,
		GDF_RGB10A2UI,
		GDF_RG11B10F,
		GDF_R5G6B5,
		GDF_D24S8,

		GDF_RGTC1,
		GDF_RGTC2,
		GDF_DXT1,
		GDF_DXT1A,
		GDF_DXT3,
		GDF_DXT5,
	};

	enum TextureFormat : uint8
	{
		TF_Unknown = 0,

		TF_R8 = GDF_UByte1N,
		TF_RG8 = GDF_UByte2N,
		TF_RGBA8 = GDF_UByte4N,
		TF_RGBA8SN = GDF_UByte4SN,

		TF_RG16 = GDF_UShort2N,
		TF_RGBA16 = GDF_UShort4N,

		TF_RG16I = GDF_UShort2,
		TF_RGBA16I = GDF_UShort4,

		TF_R16F = GDF_Half1,
		TF_RG16F = GDF_Half2,
		TF_RGBA16F = GDF_Half4,

		TF_R32F = GDF_Float1,
		TF_RG32F = GDF_Float2,
		TF_RGBA32F = GDF_Float4,

		TF_RGB10A2 = GDF_RGB10A2,
		TF_RGB10A2UI = GDF_RGB10A2UI,
		TF_RG11B10F = GDF_RG11B10F,
		TF_R5G6B5 = GDF_R5G6B5,
		TF_D24S8 = GDF_D24S8,

		// compressed

		TF_RGTC1 = GDF_RGTC1,
		TF_RGTC2 = GDF_RGTC2,
		TF_DXT1 = GDF_DXT1,
		TF_DXT1A = GDF_DXT1A,
		TF_DXT3 = GDF_DXT3,
		TF_DXT5 = GDF_DXT5,

		//TODO: more compressed formats for mobile devices
	};

	//----------------------------------------------------------------------------//
	// GpuObject
	//----------------------------------------------------------------------------//

	enum GpuCpuAccess : uint
	{
		GCA_None = 0,
		GCA_Read = AM_Read,
		GCA_Write = AM_Write,
		GCA_ReadWrite = AM_ReadWrite,

		GCA_Mask = AM_ReadWrite,
	};

	/// Usage of gpu object. One of them is included in object flags.
	enum GpuObjectUsage : uint8
	{
		GOU_Default = 0x0, //!< Default usage. Object is rarely accessible on cpu-side. Object can be attached to input stage. 
		GOU_Immutable = 0x10, //!< Immutable object. Uses for objects of state.	For other objects it is equivalence ge::GOU_Default.
		GOU_Dynamic = 0x20, ///!< Often updateable object on cpu-side. Object cannot be attached to output stage.
		GOU_Staging = 0x40,	///!< Object places data in local memory for effective transfering of data between cpu and gpu. Object cannot be attached to an input and an output stage.

		GOU_Mask = 0xf0,
	};

	/// Flags of gpu object. Can be combined in object flags.
	enum GpuObjectFlags
	{
		GOF_Input = 0x0100,	//!< Object can be attached to input stage. This flag is mounted automatically by an object, depending on ge::GpuObjectUsage.
		GOF_Output = 0x0200, //!< Object can be attached to output stage.
		GOF_UnorderedAccess = 0x0400, //!< Object can be accessed in computation shader.

		GOF_Mask = 0xff00,
	};

	class GpuObject : public RefCounted
	{
	public:

		uint GetFlags(void) { return m_flags; }
		bool HasFlags(uint _flags) { return (m_flags & _flags) != 0; }
		GpuCpuAccess GetCpuAccess(void) { return (GpuCpuAccess)(m_flags & GCA_Mask); }
		GpuObjectUsage GetUsage(void) { return (GpuObjectUsage)(m_flags & GOU_Mask); }

	protected:
		uint m_flags = 0;
	};

	//----------------------------------------------------------------------------//
	// HardwareBuffer
	//----------------------------------------------------------------------------//

	enum HardwareBufferUsage : uint8
	{
		HBU_Default = 0, ///!< Default usage. Can be used for output buffer and rarely updateable data on cpu-side.
		HBU_Dynamic = 0x1, ///!< Often updateable data on cpu-side. Cannot be used for output buffer.
		HBU_Staging = 0x2, ///!< Data in local memory for effective transfering of data between cpu and gpu. Cannot be used for input and output buffers.
	};

	enum HardwareBufferFlags : uint
	{
		HBF_VertexBuffer,
		HBF_IndexBuffer,
		HBF_Uniform,
		HBF_TransformFeedback,
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

	//----------------------------------------------------------------------------//
	// RenderCommandList
	//----------------------------------------------------------------------------//

	/// List of render commands for deferred execution. Can be used asynchronously.
	class RenderCommandList
	{
	public:

		/// Begin command list.
		virtual void Begin(void) = 0;
		/// End command list.
		virtual void End(void) = 0;
		/// Wait execution ending.
		virtual bool Wait(void) = 0;

		//virtual bool Flush(void) = 0;
		//Present(Window*);

	protected:
	};

	//----------------------------------------------------------------------------//
	// RenderSystem
	//----------------------------------------------------------------------------//

	class RenderSystem : public Singleton<RenderSystem>
	{
	public:

		RenderSystem(void);
		~RenderSystem(void);

		static bool Create(RenderSystemType _type, ShaderModel _shaderModel, uint _flags, uint _numThreads);

		virtual WindowPtr CreateWindow(void* _externalWindow = nullptr) = 0;

		//CreateCommandList(uint _group) = 0;
		//virtual void BeginFrame(void) = 0;
		//virtual void EndFrame(void) = 0;

		/// Add command list to queue
		virtual void Execute(RenderCommandList* _commands) = 0;
		virtual void SwapBuffers(Window* _window) = 0;

	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	////////////////////////////////////////////////////////////////////////////////

	class RenderWindow;

	typedef Ptr<class HardwareBuffer> HardwareBufferPtr;
	typedef Ptr<class Texture> TexturePtr;


	//----------------------------------------------------------------------------//
	// VertexFormat
	//----------------------------------------------------------------------------//

	// Vertex attribute pseudonym.
	enum VertexAttrib : uint8
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
	};

	// Vertex attribute format.
	enum AttribFormat : uint8
	{
		AF_None = GDF_Unknown,
		AF_Half2 = GDF_Half2,
		AF_Half4 = GDF_Half4,
		AF_Float = GDF_Float1,
		AF_Float2 = GDF_Float2,
		AF_Float3 = GDF_Float3,
		AF_Float4 = GDF_Float4,
		AF_UByte4 = GDF_UByte4,
		AF_UByte4N = GDF_UByte4N,
		AF_Byte4N = GDF_Byte4N,
		AF_UShort2 = GDF_UShort2,
		AF_UShort2N = GDF_UShort2N,
		AF_UShort4 = GDF_UShort4,
		AF_UShort4N = GDF_UShort4N,
	};

	enum : uint
	{
		MAX_VERTEX_ATTRIBS = 16,
		MAX_VERTEX_STREAMS = 8,
	};

	class VertexFormat : public GpuObject
	{
	public:

		/// Vertex attribute description.
		struct Attrib
		{
			AttribFormat format = AF_None;
			uint8 stream = 0;
			uint16 offset = 0;
		};

		/// Vertex format description.
		struct Desc
		{
			Attrib attribs[MAX_VERTEX_ATTRIBS];

			Desc& operator () (VertexAttrib _attrib, AttribFormat _format, uint _offset, uint8 _stream = 0)
			{
				ASSERT(_attrib < MAX_VERTEX_ATTRIBS);
				ASSERT(_stream < MAX_VERTEX_STREAMS);
				attribs[_attrib].format = _format;
				attribs[_attrib].stream = _stream;
				attribs[_attrib].offset = (uint16)_offset;
				return *this;
			}
			const Attrib& operator [] (uint _attrib) const { ASSERT(_attrib < MAX_VERTEX_ATTRIBS); return attribs[_attrib]; }
			Attrib& operator [] (uint _attrib) { ASSERT(_attrib < MAX_VERTEX_ATTRIBS); return attribs[_attrib]; }
		};

		VertexFormat(const Desc& _desc) : m_desc(_desc) { }

	protected:
		Desc m_desc;
	};

	//----------------------------------------------------------------------------//
	// VertexBuffer
	//----------------------------------------------------------------------------//
	
	struct VertexBuffer
	{
		HardwareBufferPtr buffer;
		uint offset;
		uint stride;
	};

	//----------------------------------------------------------------------------//
	// VertexArray
	//----------------------------------------------------------------------------//

	class VertexArray : public GpuObject
	{
	public:
		virtual VertexFormat* GetFormat(void) = 0;
		uint GetNumBuffers(void) { return m_numBuffers; }
		virtual bool GetBuffer(uint _slot, VertexBuffer& _buffer) = 0;
		HardwareBuffer* GetBuffer(uint _slot, uint* _offset = nullptr, uint* _stride = nullptr)
		{
			VertexBuffer _buffer;
			if (!GetBuffer(_slot, _buffer))
				return nullptr;
			if (_offset)
				*_offset = _buffer.offset;
			if (_stride)
				*_stride = _buffer.stride;
			return _buffer.buffer;
		}

	protected:
		uint m_numBuffers = 0;
	};

	//----------------------------------------------------------------------------//
	// Texture
	//----------------------------------------------------------------------------//


	enum : uint
	{
		MAX_TEXTURE_LOD = 32,
	};

	enum TextureFlags : uint
	{
		TF_Asset = 0x00010000,
		TF_Streamable = TF_Asset | 0x00020000,
	};

	class Texture : public GpuObject
	{

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class TextureManager : public Singleton<TextureManager>
	{

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	typedef Ptr<class ShaderSource>	ShaderSourcePtr;
	typedef Ptr<class ShaderStage>	ShaderStagePtr;
	typedef Ptr<class ShaderProgram> ShaderProgramPtr;

	enum ShaderType
	{
		ST_Vertex,
		ST_Pixel,
		ST_Geometry,
		ST_Hull, //!<\since ShaderModel 5 and above 
		ST_Domain, //!<\since ShaderModel 5 and above 
		ST_Compute, //!<\since ShaderModel 5 and above 
	};

	class ShaderOptions
	{

	};

	class ShaderSource : public Object
	{
	public:

		virtual ShaderStagePtr Compile(ShaderOptions& _options, const String& _entry, uint _flags = 0) = 0;

		void Reload(void);

	protected:

		Mutex m_mutex;
		String m_filename;
		time_t m_filetime;
		String m_rawText;
		String m_text;
	};

	class ShaderStage : public GpuObject
	{
	public:

		

	protected:
	};

	class ShaderProgram : public GpuObject
	{

	};

	class ShaderManager : public Singleton<ShaderManager>
	{

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class GpuFence : public GpuObject
	{
	public:

		enum : uint64
		{
			MAX_TIMEOUT = (uint64)-1,
		};

		virtual void Begin(bool _flushCommands = true) = 0;

		/// Wait GPU commands completion with blocking cpu thread.
		virtual bool CpuWait(uint64 _ns = MAX_TIMEOUT) = 0;
		/// Wait GPU commands completion without blocking cpu thread. Use it for synchronization of objects modification between threads.
		virtual void GpuWait(uint64 _ns = MAX_TIMEOUT) = 0;

	protected:
	};

}
