#pragma once

#include "GraphicsDefs.hpp"
#include "Thread.hpp"
#include "Math.hpp"

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class GLBuffer;
	class GLVertexFormat;
	class GLRenderDevice;

#ifndef MULTIRENDER
	typedef GLBuffer HardwareBuffer;
	typedef GLVertexFormat HardwareVertexFormat;
	typedef GLTexture HardwareTexture;
	typedef GLRenderContext RenderContext;
	typedef GLRenderDevice RenderDevice;
#endif

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct GLBufferHandle;
	struct GLTextureHandle;

	//----------------------------------------------------------------------------//
	// GLBuffer
	//----------------------------------------------------------------------------//

	class GLBuffer final : public MR_BASECLASS(HardwareBuffer, RCBase)
	{
	public:
		struct Handle;

		uint8* Map(MappingMode _mode, uint _offset, uint _size) MR_OVERRIDE;
		void Unmap(void) MR_OVERRIDE;
		void CopyFrom(HardwareBuffer* _src, uint _srcOffset, uint _dstOffset, uint _size) MR_OVERRIDE;
		HwBufferUsage GetUsage(void) MR_OVERRIDE { return m_usage; }
		uint GetSize(void) MR_OVERRIDE { return m_size; }
		uint GetMemoryGroup(void) MR_OVERRIDE { return m_memoryGroup; }
		GLBufferHandle* _Handle(void) { return m_handle; }

	protected:
		friend class GLRenderDevice;
		friend class GLTexture;

		GLBuffer(HwBufferUsage _usage, uint _size, const void* _data, int _memoryGroup);
		~GLBuffer(void);
		void _Realloc(uint _newSize);

		HwBufferUsage m_usage;
		uint m_size;
		int m_memoryGroup; // TODO: remove
		GLBufferHandle* m_handle;
	};

	//----------------------------------------------------------------------------//
	// GLVertexAttrib
	//----------------------------------------------------------------------------//

	struct GLVertexAttrib : VertexAttribDesc
	{
		GLVertexAttrib& operator = (const VertexAttribDesc& _desc);

		uint8 size = 0;
		uint8 components = 0;
		bool normalized = false;
		bool integer = false;
		uint8 userIndex = 0;
		uint16 gltype = 0;
	};

	//----------------------------------------------------------------------------//
	// GLVertexStreamInfo
	//----------------------------------------------------------------------------//

	struct GLVertexStreamInfo
	{
		uint8 attribs[MAX_VERTEX_ATTRIBS];
		uint8 numAttribs = 0;
		uint16 userIndex = 0;
		uint stride = 0;
	};

	//----------------------------------------------------------------------------//
	// GLVertexStream
	//----------------------------------------------------------------------------//

	struct GLVertexStream
	{
		Ptr<GLBuffer> buffer;
		uint offset = 0;
		uint stride = 0;
	};

	//----------------------------------------------------------------------------//
	// GLVertexFormat
	//----------------------------------------------------------------------------//

	class GLVertexFormat final : public MR_BASECLASS(HardwareVertexFormat, NonCopyable)
	{
	public:


		const VertexFormatDesc& GetDesc(void) MR_OVERRIDE { return m_desc; }

		const GLVertexAttrib& GetAttrib(uint _internalIndex) { ASSERT(_internalIndex < m_numAttribs); return m_attribs[_internalIndex]; }
		uint GetAttribInternalIndex(uint _userIndex) { return _userIndex < m_numAttribs ? m_attribIndices[_userIndex] : (uint)-1; }
		uint GetNumAttribs(void) { return m_numAttribs; }

		const GLVertexStreamInfo& GetStream(uint _internalIndex) { ASSERT(_internalIndex < m_numStreams); return m_streams[_internalIndex]; }
		uint GetStreamInternalIndex(uint _userIndex) { return _userIndex < m_numStreams ? m_streamIndices[_userIndex] : (uint)-1; }
		uint GetNumStreams(void) { return m_numStreams; }

	protected:
		friend class GLRenderDevice;
		friend struct Cmd_BindVertexFormat;

		GLVertexFormat(const VertexFormatDesc& _desc);
		~GLVertexFormat(void);
		void _BindFormat(void);
		void _BindBuffer(uint _stream, GLBuffer* _buffer, uint _offset, uint _stride);

		VertexFormatDesc m_desc;
		GLVertexAttrib m_attribs[MAX_VERTEX_ATTRIBS];
		GLVertexStreamInfo m_streams[MAX_VERTEX_STREAMS];
		uint8 m_attribIndices[MAX_VERTEX_ATTRIBS];
		uint8 m_streamIndices[MAX_VERTEX_ATTRIBS];
		uint16 m_numAttribs = 0;
		uint16 m_numStreams = 0;
		uint m_maxAttribIndex = 0;
	};

	//----------------------------------------------------------------------------//
	// GLTexture
	//----------------------------------------------------------------------------//

	class GLTexture	: public HardwareTexture
	{
	public:
		bool Realloc(uint _width, uint _height, uint _depth = 0) MR_OVERRIDE;
		uint8* Map(MappingMode _mode, uint _lod) MR_OVERRIDE;
		void Unmap(void) MR_OVERRIDE;
		HardwareBuffer* GetBuffer(void) MR_OVERRIDE;
		void SetLodRange(uint _baseLod, uint _numLods = 0) MR_OVERRIDE;

	protected:
		friend class GLRenderDevice;

		GLTexture(TextureType _type, TextureUsage _usage, PixelFormat _format, uint _maxLods = 0);
		~GLTexture(void);

	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

#if 0

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	class HwDepthStencilState
	{
	public:

	protected:

		HwDepthStencilState(const DepthStencilDesc& _desc);
		~HwDepthStencilState(void);

		void _Bind(HwDepthStencilState* _current);

		DepthStencilDesc m_desc;

	};

	class HwBlendState
	{

	};

	class HwRasterizerState
	{

	};

	//----------------------------------------------------------------------------//
	// HwVertexArray 
	//----------------------------------------------------------------------------//

	class HwVertexArray : public RCBase
	{
	public:
		struct Stream
		{
			Ptr<HwBuffer> buffer;
			uint offset = 0;
			uint stride = 0;
		};

		void SetBuffer(uint _stream, HwBuffer* _buffer, uint _offset = 0, uint _stride = 0);
		HwBuffer* GetBuffer(uint _stream, uint* _offset = nullptr, uint* _stride = nullptr);

	protected:
		friend class RenderDevice;

		HwVertexArray(HwVertexFormat* _format);
		~HwVertexArray(void);

		HwVertexFormat* m_format;
		SArray<Stream> m_streams;
		uint m_handle;
		uint m_enabledAttribs;
	};

	//----------------------------------------------------------------------------//
	// HwTexture 
	//----------------------------------------------------------------------------//

	class HwTexture : public RCBase
	{
	public:
		bool Realloc(uint _width, uint _height, uint _depth = 0);
		uint8_t* Map(MappingMode _mode, uint _lod);
		void Unmap(void);
		void SetBaseLod(uint _lod);
		void SetNumLods(uint _num);
		HwBuffer* GetBuffer(void) { return m_buffer; }
		uint _GetHandle(void) { return m_handle; }

	protected:
		friend class RenderDevice;

		HwTexture(TextureType _type, TextureUsage _usage, PixelFormat _format, uint _maxLods = 0);
		~HwTexture(void);

		TextureType m_type;
		TextureUsage m_usage;
		PixelFormat m_format;
		Vec3ui m_size;
		uint8_t m_baseLod;
		uint8_t m_numLods;
		uint8_t m_createdLods;
		uint8_t m_maxLods;
		uint8_t m_mappedLod;
		uint8_t m_mappingMode;
		uint m_mappedData;
		uint m_handle;
		HwBuffer* m_buffer;
	};

	//----------------------------------------------------------------------------//
	// HwSampler 
	//----------------------------------------------------------------------------//

	class HwSampler : public RCBase
	{
	public:

		// ...

		static HwSamplerPtr Create(TextureFilter _min, TextureFilter _mag, uint _aniso, TextureWrap _wrapS, TextureWrap _wrapT, TextureWrap _wrapR, CompareFunc _func);

		void SetMaxAnisotropy(uint _value);
		void SetMinLod(uint _value);
		void SetMaxLod(uint _value);
		void SetLodBias(float _value);


	protected:

		HwSampler(TextureFilter _min, TextureFilter _mag, uint _aniso, TextureWrap _wrapS, TextureWrap _wrapT, TextureWrap _wrapR, CompareFunc _func);
		~HwSampler(void);

		CompareFunc m_depthFunc;
		TextureFilter m_minFilter;
		TextureFilter m_magFilter;
		union
		{
			TextureWrap m_wrap[3];
			struct { TextureWrap m_wrapS, m_wrapT, m_wrapR; };
		};
		int8_t m_aniso;
		uint8_t m_minLod;
		uint8_t m_maxLod;
		float m_lodBias;
		uint m_handle;
	};

	//----------------------------------------------------------------------------//
	// HwShader 
	//----------------------------------------------------------------------------//

	class HwShader : public RCBase
	{
	public:

		// ...

		HwShader(ShaderType _type);
		~HwShader(void);
		void Compile(const String& _source);
		uint16 GetActualVersion(void) { return m_version; }
		ShaderType GetType(void) { return m_type; }
		ShaderState GetState(void) { return m_state; }
		uint _GetHandle(void) { return m_handle; }

		static uint16 AddFileName(const String& _name);
		static String GetFileName(uint16 _id);

	protected:
		friend class HwProgram;

		ShaderType m_type;
		ShaderState m_state;
		uint16 m_version;
		uint m_handle;

		static HashMap<uint, uint16> s_nameTable;
		static Array<String> s_names;
		static CriticalSection s_namesMutex;
	};

	//----------------------------------------------------------------------------//
	// HwProgram
	//----------------------------------------------------------------------------//

	enum ShaderParamType
	{
		SPT_Unknown,

		// scalar
		SPT_Int,
		SPT_Float,

		// vector
		SPT_Vec2i,
		SPT_Vec3i,
		SPT_Vec4i,
		SPT_Vec2,
		SPT_Vec3,
		SPT_Vec4,

		// matrix
		SPT_Mat34,
		SPT_Mat44,

		// buffer
		SPT_Block,

		// texture
		SPT_Tex2D,
		SPT_Tex2DArray,
		SPT_Tex2DMultisample,
		SPT_Tex3D,
		SPT_TexCube,
		SPT_TexBuffer,
	};

	class HwProgram : public RCBase
	{
	public:

		// ...

		struct Param
		{
			int id = -1;
			String name;
			ShaderParamType type = SPT_Unknown;
			uint elementSize = 0;
			uint count = 0;
			uint slot = 0;

			//SArray<Param> signature;
			//uint offset = 0;
			//uint alignment = 4;
		};

		struct Stage
		{
			Ptr<HwShader> shader;
			uint16 actualVersion;
		};

		///\param[in] _vfOut specifies format of output vertex. Can be null.
		static HwProgramPtr Create(HwVertexFormat* _vfIn, HwVertexFormat* _vfOut, HwShader* _vs, HwShader* _fs, HwShader* _gs);

		bool Link(void);
		bool IsOutOfDate(void);
		int GetParamIndex(const String& _name);
		const Param& GetParam(uint _index);
		void SetParam(uint _index, ShaderParamType _type, const void* _value, uint _count);
		void BindTexture(uint _index, uint _slot);
		void BindBuffer(uint _index, uint _slot);

	protected:
		HwProgram(uint _id, HwVertexFormat* _vfIn, HwVertexFormat* _vfOut, HwShader* _vs, HwShader* _fs, HwShader* _gs);
		~HwProgram(void);


		Stage m_stages[MAX_SHADER_STAGES];
		HwVertexFormat* m_vfIn;
		HwVertexFormat* m_vfOut;

		HashMap<String, uint> m_paramNames;
		SArray<Param> m_params;

		uint m_handle;
		uint m_id;

		bool m_linked;
		bool m_valid;

		String m_log;

		static HashMap<uint, HwProgram*> s_instances;
	};
#endif

	
	//----------------------------------------------------------------------------//
	// GLRenderContext
	//----------------------------------------------------------------------------//

#define gGLRenderContext GLRenderContext::Get<GLRenderContext>()

	class GLRenderContext final : public MR_BASECLASS(RenderContext, Singleton<GLRenderContext>)
	{
	public:

		void BeginFrame() MR_OVERRIDE;
		void EndFrame(void) MR_OVERRIDE;

		void SetVertexFormat(HardwareVertexFormat* _format) MR_OVERRIDE;
		void SetVertexBuffer(uint _stream, HardwareBuffer* _buffer, uint _offset = 0, uint _stride = 0) MR_OVERRIDE;

	protected:
		friend class GLRenderDevice;
		friend struct Cmd_SwapBuffers;

		GLRenderContext(void);
		~GLRenderContext(void);
		Condition m_endFrame;

		Array<Ptr<RCBase>> m_resources;
	};

	//----------------------------------------------------------------------------//
	// GLRenderDevice
	//----------------------------------------------------------------------------//

#define gGLRenderDevice GLRenderDevice::Get<GLRenderDevice>()

	class GLRenderDevice final : public MR_BASECLASS(RenderDevice, Singleton<GLRenderDevice>)
	{
	public:

#ifndef MULTIRENDER
		ShaderModel GetShaderModel(void) { return m_shaderModel; }
		bool IsDebugContext(void) { return m_debugContext; }
#endif

		/*HwBufferPtr CreateBuffer(HwBufferUsage _usage, uint _size, const void* _data = nullptr);
		HwVertexFormat* CreateVertexFormat(VertexElement* _elements, uint _numElements);
		HwVertexArrayPtr CreateVertexArray(HwVertexFormat* _format);
		HwTexturePtr CreateTexture(TextureType _type, TextureUsage _usage, PixelFormat _format, uint _maxLods = 0);
		*/
	protected:
		friend class RenderSystem;
		friend class GLRenderContext;
		friend struct Cmd_SwapBuffers;

		GLRenderDevice(void);
		~GLRenderDevice(void);
		bool _Init(ShaderModel _shaderModel, RenderContextProfile _profile, bool _debugContext) MR_OVERRIDE;
		SDL_Window* _GetSDLWindow(void) MR_OVERRIDE;


		bool _InitDriver(void);
		bool _Init33(void);
		bool _Init43(void);
		void _DriverThread(void);
		void _SwapBuffers(void);

#ifndef MULTIRENDER
		ShaderModel m_shaderModel;
		RenderContextProfile m_profile;
		bool m_debugContext;
#endif

		class GLContext* m_context;

		Thread m_thread;
		Condition m_initialized;
		bool m_success;
		volatile bool m_runThread;

		bool m_newVsync;
		bool m_vsync;


		//////

		//HashMap<uint, HwVertexFormat*> m_vertexFormats;

	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
