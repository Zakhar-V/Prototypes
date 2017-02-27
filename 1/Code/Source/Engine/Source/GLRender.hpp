#pragma once

#include "Render.hpp"
#include "Thread.hpp"
#include "Log.hpp"

#ifdef USE_GL

#include <SDL.h>
#include "glLoad.h"
#include "PlatformIncludes.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

//#define _NO_CHECK_ERRORS

#define gGLRenderSystem GLRenderSystem::Get<GLRenderSystem>()

	extern uint GLVersion;
	extern uint GLRealVersion;

	class GLBuffer;
	typedef Ptr<GLBuffer> GLBufferPtr;

	//----------------------------------------------------------------------------//
	// Debug
	//----------------------------------------------------------------------------//

#ifdef DEBUG_RC

	// GL debug output
	void GLDebugMsg(int _type, int _prio, const char* _msg, ...);
	// gDEBugger
	void GLDebugPoint(const char* _title);

#define R_DEBUG_INFO(msg, ...) GLDebugMsg(GL_DEBUG_TYPE_OTHER_ARB, GL_DEBUG_SEVERITY_LOW_ARB, msg, ##__VA_ARGS__)
#define R_DEBUG_WARNING(msg, ...) GLDebugMsg(GL_DEBUG_TYPE_ERROR_ARB, GL_DEBUG_SEVERITY_MEDIUM_ARB, msg, ##__VA_ARGS__)
#define R_DEBUG_ERROR(msg, ...) GLDebugMsg(GL_DEBUG_TYPE_ERROR_ARB, GL_DEBUG_SEVERITY_HIGH_ARB, msg, ##__VA_ARGS__)
//#define R_DEBUG_UB(msg, ...) GLDebugMsg(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB, GL_DEBUG_SEVERITY_HIGH_ARB, msg, ##__VA_ARGS__)
//#define R_DEBUG_PERF(msg, ...) GLDebugMsg(GL_DEBUG_TYPE_PERFORMANCE_ARB, GL_DEBUG_SEVERITY_LOW_ARB, msg, ##__VA_ARGS__)
#define R_DEBUG_NODE(...) ge::LogNode _logNode_(false, __FUNCTION__, __FUNCTION__ "(" ##__VA_ARGS__")"); /*GLDebugMarker(__FUNCTION__ "(" ##__VA_ARGS__")")*/
#else
#define R_DEBUG_INFO(msg, ...)
#define R_DEBUG_WARNING(msg, ...)
#define R_DEBUG_ERROR(msg, ...)
//#define R_DEBUG_UB(msg, ...)
//#define R_DEBUG_PERF(msg, ...)
#define R_DEBUG_NODE(...)
#endif

#define CHECK_RENDER_THREAD(...) R_DEBUG_NODE(); if(!GLRenderContext::IsRenderThread()) {R_DEBUG_ERROR(__FUNCTION__ " can be used only in render thread"); return ##__VA_ARGS__; }
#if defined(_DEBUG) && defined(DEBUG_RC)
#define CHECK_RENDER_THREAD_D(...) CHECK_RENDER_THREAD(##__VA_ARGS__)
#else
#define CHECK_RENDER_THREAD_D(...)
#endif

	//----------------------------------------------------------------------------//
	// GLBuffer
	//----------------------------------------------------------------------------//

	class GLBuffer : public HardwareBuffer
	{
	public:
		GLBuffer(HardwareBufferUsage _usage, AccessMode _cpuAccess, uint _size, const void* _data);
		~GLBuffer(void);

		uint8* Map(MappingMode _mode) override;
		uint8* Map(MappingMode _mode, uint _offset, uint _size) override;
		void Unmap(void) override;
		void Write(const void* _src, uint _offset, uint _size) override;
		void Read(void* _dst, uint _offset, uint _size) override;
		void CopyFrom(HardwareBuffer* _src, uint _srcOffset, uint _dstOffset, uint _size) override;

		uint Handle(void) { return m_handle; }
		void _Bind(void);

	protected:

		struct MappedData21
		{
			uint8* ptr = nullptr;
			uint8* storage = nullptr;
			uint8* tempBuffer = nullptr;
			MappingMode mode = MM_None;
			uint offset = 0;
			uint size = 0;
		};

		MappedData21* m_mappedData;
		bool m_isMapped;
		uint m_handle;
	};

	//----------------------------------------------------------------------------//
	// GLVertexFormat
	//----------------------------------------------------------------------------//

	struct GLVertexAttribDesc
	{
		uint type;
		uint8 components;
		uint8 size;
		bool normalized;
		bool integer;

		uint8 index;
		uint8 streamIndex;
		uint16 offset;
	};

	struct GLVertexStreamDesc
	{
		uint8 index;
		uint8 numAttribs;
		uint16 stride;
		GLVertexAttribDesc attribs[MAX_VERTEX_ATTRIBS];
	};

	class GLVertexFormat : public VertexFormat
	{
	public:
		GLVertexFormat(const VertexFormatDesc& _desc, uint32 _hash, uint _index);
		~GLVertexFormat(void);

		const GLVertexStreamDesc* GetStreams(void) { return m_streams; }
		uint GetNumStreams(void) { return m_numStreams; }
		uint GetStreamIndex(uint _stream) { return _stream < m_numStreams ? m_streamIndices[_stream] : MAX_VERTEX_STREAMS; }

		static void CreateDefaults(void);
		static VertexFormat* Create(const VertexFormatDesc& _desc);
		static void ReleaseResources(void);
		static GLVertexFormat* GetFormat(uint _index);

	protected:

		uint32 m_hash;
		uint m_index;
		uint m_numStreams;
		GLVertexStreamDesc m_streams[MAX_VERTEX_STREAMS];
		uint8 m_streamIndices[MAX_VERTEX_STREAMS];

		static Mutex s_mutex;
		static HashMap<uint32, uint> s_formatsTable;
		static Array<GLVertexFormat*> s_formats;
	};

	//----------------------------------------------------------------------------//
	// GLVertexArray
	//----------------------------------------------------------------------------//

	struct GLVertexStream
	{
		Ptr<GLBuffer> buffer;
		uint offset = 0;
		uint stride = 0;
		bool updated = true; // for OpenGL 3.3
		bool enabled = false; // for OpenGL >= 3.3
	};

	class GLVertexArray : public VertexArray
	{
	public:
		GLVertexArray(GLVertexFormat* _format);
		~GLVertexArray(void);

		void SetBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset = 0, uint _stride = 0) override;
		VertexFormat* GetFormat(void) override { return m_format; }

		// for GLRenderSystem
		
		void Bind(uint _baseVertex);
		void BindIndexBuffer(GLBuffer* _indexBuffer);
		static void ReleaseResources(void);

	protected:
		friend class GLRenderSystem;

		GLVertexFormat* m_format;
		bool m_updated;
		uint m_handle;
		uint m_numStreams;
		GLVertexStream m_streams[MAX_VERTEX_STREAMS];
		Ref<GLBuffer> m_lastIndexBuffer; // for OpenGL >= 3.3
		uint m_baseVertex; // for OpenGL 2.1
		uint m_attribSet; // for OpenGL 2.1
		uint m_bufferIds[MAX_VERTEX_STREAMS]; // for GL_ARB_vertex_attrib_binding (OpenGL >= 4.3)
		ptrdiff_t m_bufferOffsets[MAX_VERTEX_STREAMS]; // for GL_ARB_vertex_attrib_binding (OpenGL >= 4.3)
		int m_bufferStrides[MAX_VERTEX_STREAMS]; // for GL_ARB_vertex_attrib_binding (OpenGL >= 4.3)

		// for render thread

		static CriticalSection s_mutex;
		static Array<uint> s_toDelete; // for deletion in render thread

		static GLVertexArray* s_current;
		static GLBuffer* s_currentIndexBuffer; // for OpenGL 2.1
		static uint s_attribSet; // for OpenGL 2.1
	};

	//----------------------------------------------------------------------------//
	// GLShaderCompiler
	//----------------------------------------------------------------------------//

	class GLShaderCompiler : public Singleton <GLShaderCompiler>
	{

	};

	//----------------------------------------------------------------------------//
	// GLTexture
	//----------------------------------------------------------------------------//

	class GLTexture : public TextureObject
	{
	public:

	protected:
		uint m_handle;
	};

	class GLRenderTarget : public RenderTarget
	{
	public:

	protected:
		uint m_handle;
	};

	//----------------------------------------------------------------------------//
	// GLRenderState
	//----------------------------------------------------------------------------//

	struct GLRenderState
	{
		static Ptr<GLVertexArray> vertexArray;
		static Ptr<GLBuffer> indexBuffer;
		static IndexType indexType;
		static uint indexOffset;
		static bool primitveRestartEnabled; // for OpenGL >= 3.3
		static uint primitveRestartIndex; // for OpenGL 3.3
		static PrimitiveType primitiveType;
		static uint patchSize; // for OpenGL >= 4.3
	};

	//----------------------------------------------------------------------------//
	// GLRenderContext
	//----------------------------------------------------------------------------//

#define GLState GLRenderContext::CurrentContext()

	struct GLRenderContext
	{
		bool Create(ShaderModel _shaderModel, uint _flags);
		bool Create(const GLRenderContext& _sharedContext);
		void Destroy(void);

		void MakeCurrent(SDL_Window* _window = nullptr) const;

		void SetupHints(void);
		static void SetupHints(uint _version, bool _coreProfile, bool _debugContext, bool _sRGB);

		SDL_Window* window = nullptr;
		SDL_GLContext context = nullptr;
		uint version = 0; // 2.1, 3.3, 4.3, 4.5
		bool coreProfile = false;
		bool debugContext = false;
		bool sRGB = false;

		static GLRenderContext* CurrentContext(void);
		static bool IsRenderThread(void);

		GLBuffer* currentBuffer = nullptr;
		GLBuffer* currentReadBuffer = nullptr;
		GLBuffer* currentWriteBuffer = nullptr;
	};

	//----------------------------------------------------------------------------//
	// GLWindow
	//----------------------------------------------------------------------------//

	class GLWindow : public Window
	{
	public:
		GLWindow(SDL_Window* _handle);
		~GLWindow(void);

		void SwapBuffers(bool _vsync = true) override;
	};

	//----------------------------------------------------------------------------//
	// GLRenderSystem
	//----------------------------------------------------------------------------//

	class GLRenderSystem : public RenderSystem
	{
	public:
		GLRenderSystem(void);
		~GLRenderSystem(void);
		bool Init(ShaderModel _shaderModel, uint _flags);

		void DebugPoint(const char* _title) override;

		bool RegisterThread(void) override { return true; }
		void UnregisterThread(void) override { }

		void BeginFrame(void) override;
		void EndFrame(void) override;

		WindowPtr CreateWindow(void* _externalWindow) override;
		
		VertexFormat* CreateVertexFormat(const VertexFormatDesc& _desc) override;
		HardwareBufferPtr CreateBuffer(HardwareBufferUsage _usage, AccessMode _cpuAccess, uint _size, const void* _data = nullptr) override;
		VertexArrayPtr CreateVertexArray(VertexFormat* _format) override;

		void SetGeometry(VertexArray* _vertices, HardwareBuffer* _indices, IndexType _indexType = IF_UShort, uint _indexOffset = 0) override;


		void Draw(PrimitiveType _type, uint _baseVertex, uint _numElements) override;
		void DrawInstanced(PrimitiveType _type, uint _baseVertex, uint _numElements, uint _numInstances = 1) override;
		void DrawIndexed(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements) override;
		void DrawIndexedInstanced(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements, uint _numInstances = 1) override;

		void _RestoreContext(void); // for GLWindow::~GLWindow (temp)

	protected:

		//bool _PrepareDrawing(PrimitiveType _type, uint _baseVertex);
		void _Draw(PrimitiveType _type, uint _baseVertex, uint _baseIndex, uint _numElements, uint _numInstances, bool _indexed);
		void _ApplyChanges(void);

		GLRenderContext m_mainContext;
		GLRenderContext m_bgContexts[4];

		Ptr<GLVertexArray> m_emptyVertexArray;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}

#endif
