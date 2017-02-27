#pragma once

#include "GraphicsDefs.hpp"
#include "Thread.hpp"
#include "Math.hpp"

#include "GL/glLoad.h"
#include <SDL.h>

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Driver thread marker
	//----------------------------------------------------------------------------//

	bool IsDriverThread(void); // in GLGraphics.cpp

#define VERIFY_DRIVER_THREAD() ASSERT(IsDriverThread(), "This function should be called from driver thread only")

	//----------------------------------------------------------------------------//
	// Utils
	//----------------------------------------------------------------------------//

#define GL_FUNCDEF(R, N, ...) typedef R (APIENTRY*N##_pfn)(##__VA_ARGS__); extern N##_pfn N
#define GL_FUNCINIT(N) N##_pfn N = nullptr
#define GL_FUNCLOAD(N) if(!(N = reinterpret_cast<N##_pfn>(SDL_GL_GetProcAddress(#N)))) ++ _numFails

	//----------------------------------------------------------------------------//
	// gDEBugger
	//----------------------------------------------------------------------------//

	GL_FUNCDEF(void, glStringMarkerGREMEDY, int len, const void *string); // GL_GREMEDY_string_marker
	GL_FUNCDEF(void, glFrameTerminatorGREMEDY, void); // GL_GREMEDY_frame_terminator
	
	bool LoadGremedyExtensions(void);
	void GLDebugString(const char* _str);
	void GLDebugBreak(void);

	//----------------------------------------------------------------------------//
	// GL_NV_command_list
	//----------------------------------------------------------------------------//

	// it's needed ?

#define GL_TERMINATE_SEQUENCE_COMMAND_NV                      0x0000
#define GL_NOP_COMMAND_NV                                     0x0001
#define GL_DRAW_ELEMENTS_COMMAND_NV                           0x0002
#define GL_DRAW_ARRAYS_COMMAND_NV                             0x0003
#define GL_DRAW_ELEMENTS_STRIP_COMMAND_NV                     0x0004
#define GL_DRAW_ARRAYS_STRIP_COMMAND_NV                       0x0005
#define GL_DRAW_ELEMENTS_INSTANCED_COMMAND_NV                 0x0006
#define GL_DRAW_ARRAYS_INSTANCED_COMMAND_NV                   0x0007
#define GL_ELEMENT_ADDRESS_COMMAND_NV                         0x0008
#define GL_ATTRIBUTE_ADDRESS_COMMAND_NV                       0x0009
#define GL_UNIFORM_ADDRESS_COMMAND_NV                         0x000a
#define GL_BLEND_COLOR_COMMAND_NV                             0x000b
#define GL_STENCIL_REF_COMMAND_NV                             0x000c
#define GL_LINE_WIDTH_COMMAND_NV                              0x000d
#define GL_POLYGON_OFFSET_COMMAND_NV                          0x000e
#define GL_ALPHA_REF_COMMAND_NV                               0x000f
#define GL_VIEWPORT_COMMAND_NV                                0x0010
#define GL_SCISSOR_COMMAND_NV                                 0x0011
#define GL_FRONT_FACE_COMMAND_NV                              0x0012

	struct GLTerminateSequenceCommandNV
	{
		GLuint  header;
	};

	struct GLNOPCommandNV
	{
		GLuint  header;
	};

	struct GLDrawElementsCommandNV
	{
		GLuint  header;
		GLuint  count;
		GLuint  firstIndex;
		GLuint  baseVertex;
	};

	struct GLDrawArraysCommandNV
	{
		GLuint  header;
		GLuint  count;
		GLuint  first;
	};

	struct GLDrawElementsInstancedCommandNV
	{
		GLuint  header;
		GLuint  mode;
		GLuint  count;
		GLuint  instanceCount;
		GLuint  firstIndex;
		GLuint  baseVertex;
		GLuint  baseInstance;
	};

	struct GLDrawArraysInstancedCommandNV
	{
		GLuint  header;
		GLuint  mode;
		GLuint  count;
		GLuint  instanceCount;
		GLuint  first;
		GLuint  baseInstance;
	};

	struct GLElementAddressCommandNV
	{
		GLuint  header;
		union
		{
			struct
			{
				GLuint  addressLo;
				GLuint  addressHi;
			};
			GLuint64  address;
		};
		GLuint  typeSizeInByte;
	};

	struct GLAttributeAddressCommandNV
	{
		GLuint  header;
		GLuint  index;
		union
		{
			struct
			{
				GLuint  addressLo;
				GLuint  addressHi;
			};
			GLuint64  address;
		};
	};

	struct GLUniformAddressCommandNV
	{
		GLuint    header;
		GLushort  index;
		GLushort  stage;
		union
		{
			struct
			{
				GLuint  addressLo;
				GLuint  addressHi;
			};
			GLuint64  address;
		};
	};

	struct GLBlendColorCommandNV
	{
		GLuint  header;
		GLfloat red;
		GLfloat green;
		GLfloat blue;
		GLfloat alpha;
	};

	struct GLStencilRefCommandNV
	{
		GLuint  header;
		GLuint  frontStencilRef;
		GLuint  backStencilRef;
	};

	struct GLLineWidthCommandNV
	{
		GLuint  header;
		GLfloat lineWidth;
	};

	struct GLPolygonOffsetCommandNV
	{
		GLuint  header;
		GLfloat scale;
		GLfloat bias;
	};

	struct GLAlphaRefCommandNV
	{
		GLuint  header;
		GLfloat alphaRef;
	};

	struct GLViewportCommandNV
	{
		GLuint  header;
		GLuint  x;
		GLuint  y;
		GLuint  width;
		GLuint  height;
	}; // only ViewportIndex 0

	struct GLScissorCommandNV
	{
		GLuint  header;
		GLuint  x;
		GLuint  y;
		GLuint  width;
		GLuint  height;
	}; // only ViewportIndex 0

	struct GLFrontFaceCommandNV
	{
		GLuint  header;
		GLuint  frontFace; // 0 for CW, 1 for CCW
	};

	GL_FUNCDEF(void, glCreateStatesNV, GLsizei n, GLuint *states);
	GL_FUNCDEF(void, glDeleteStatesNV, GLsizei n, const GLuint *states);
	GL_FUNCDEF(GLboolean, glIsStateNV, GLuint state);
	GL_FUNCDEF(void, glStateCaptureNV, GLuint state, GLenum mode);
	GL_FUNCDEF(GLuint, glGetCommandHeaderNV, GLenum tokenID, GLuint size);
	GL_FUNCDEF(GLushort, glGetStageIndexNV, GLenum shadertype);
	GL_FUNCDEF(void, glDrawCommandsNV, GLenum primitiveMode, GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, GLuint count);
	GL_FUNCDEF(void, glDrawCommandsAddressNV, GLenum primitiveMode, const GLuint64* indirects, const GLsizei* sizes, GLuint count);
	GL_FUNCDEF(void, glDrawCommandsStatesNV, GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
	GL_FUNCDEF(void, glDrawCommandsStatesAddressNV, const GLuint64* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
	GL_FUNCDEF(void, glglCreateCommandListsNV, GLsizei n, GLuint *lists);
	GL_FUNCDEF(void, glDeleteCommandListsNV, GLsizei n, const GLuint *lists);
	GL_FUNCDEF(GLboolean, glIsCommandListNV, GLuint list);
	GL_FUNCDEF(void, glListDrawCommandsStatesClientNV, GLuint list, GLuint segment, const void** indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
	GL_FUNCDEF(void, glCommandListSegmentsNV, GLuint list, GLuint segments);
	GL_FUNCDEF(void, glCompileCommandListNV, GLuint list);
	GL_FUNCDEF(void, glCallCommandListNV, GLuint list);

	bool LoadNVCommandListExtension(void);

	//----------------------------------------------------------------------------//
	// GLCommandQueue
	//----------------------------------------------------------------------------//

#define gGLResourceQueue GLCommandQueue::RQ	
#define gGLDrawQueue GLCommandQueue::DQ

	typedef void(*GLCommandFunc)(void*);

	class GLCommandQueue final : public NonCopyable // FIFO  
	{
	public:
		enum : size_t { SIZE = 1024 * 1024 }; // 1MB

		struct Command
		{
			GLCommandFunc func;
			size_t size;
		};

		template <class T> static void Exec(void* _cmd)
		{
			reinterpret_cast<T*>(_cmd)->Exec();
		}

		static GLCommandQueue RQ;
		static GLCommandQueue DQ;

		void PushRaw(const void* _data, size_t _size);
		void Push(GLCommandFunc _func, const void* _data, uint _size);
		template <class T> void Push(const T* _cmd) { Push(&Exec<T>, _cmd, sizeof(T)); }
		template <class T> void Push(void(*_func)(T*), const T* _cmd) { Push(reinterpret_cast<GLCommandFunc>(_func), _cmd, sizeof(T)); }
		bool Pop(GLCommandFunc& _func, void* _data, bool _wait);
		uint GetSize(void);
		static void Wait(uint _timeout);

		AtomicInt in, out; // for debug

	protected:

		GLCommandQueue(void);
		~GLCommandQueue(void);

		void _Wait(size_t _size);
		void _Write(const void* _data, size_t _size);
		void _Read(void* _data, size_t _size);
		size_t _GetSize(void);

		SpinLock m_mutex;
		uint8 m_data[SIZE];
		uint8* m_readPos;
		uint8* m_writePos;
		uint8* m_end;

		static Condition s_onPush;
	};

	//----------------------------------------------------------------------------//
	// GLDebugPoint
	//----------------------------------------------------------------------------//

	struct GLDebugPoint
	{
		GLDebugPoint(const char* _func, const char* _file, int _line);
		~GLDebugPoint(void);

		GLDebugPoint* prev;
		const char* func;
		const char* file;
		int line;
	};

	//----------------------------------------------------------------------------//
	// GLDebugHelper
	//----------------------------------------------------------------------------//

#define gGLDebugHelper GLDebugHelper::Get()

#ifdef DEBUG_RC
#define VERIFY_GL_SCOPE(...) GLDebugPoint _glDebugPoint_##__VA_ARGS__(__FUNCTION__, __FILE__, __LINE__)
#else
#define VERIFY_GL_SCOPE(...)
#endif

	class GLDebugHelper final : public NonCopyable
	{
	public:

		enum OutputType
		{
			OT_None,
			OT_CheckErrors,
			OT_GL43,
			OT_ARB,
			OT_AMD,
		};

		static GLDebugHelper* Get(void) { return &s_instance; }

		DebugOutputLevel SetLevel(DebugOutputLevel _level);
		void LogErrors(const char* _func, const char* _file, int _line);
		OutputType GetType(void) { return m_type; }
		void Message(const char* _func, const char* _file, int _line, const char* _src, const char* _type, const char* _level, const char* _msg);

	protected:
		friend class GLRenderDevice;
		friend struct GLDebugPoint;

		GLDebugHelper(void);
		~GLDebugHelper(void);
		void _Init(bool _debugContext);
		void _Message(const char* _func, const char* _file, int _line, const char* _src, const char* _type, const char* _level, const char* _msg);
		static void __stdcall _DebugProcARB(uint _source, uint _type, uint _id, uint _severity, int _length, const char* _message, const void* _ud);
		static void __stdcall _DebugProcAMD(uint _id, uint _category, uint _severity, int _length, const char* _message, void* _ud);

		OutputType m_type;
		DebugOutputLevel m_level;

		static GLDebugPoint* s_top;
		static GLDebugHelper s_instance;
	};

	//----------------------------------------------------------------------------//
	// GLGpuInfo
	//----------------------------------------------------------------------------//

#define gGLGpuInfo GLGpuInfo::Get()

	//TODO: generic solution (win32, linux, macos, ...), (nvidia, ati/amd, intel, ...)
	class GLGpuInfo final : public NonCopyable
	{
	public:

		static GLGpuInfo* Get(void) { return &s_instance; }

		uint GetTotalVideoMemory(void) { return m_totalVRam; }
		uint GetAvailableVideoMemory(void);

	protected:
		friend class GLRenderDevice;

		void _Init(void);

		bool m_GL_NVX_gpu_memory_info = false;
		uint m_totalVRam = 0; // kb

		static GLGpuInfo s_instance;
	};

	//----------------------------------------------------------------------------//
	// GLBufferHandle
	//----------------------------------------------------------------------------//

	struct GLBufferHandle : public NonCopyable
	{
		uint glusage = GL_STATIC_DRAW;
		uint glhandle = 0;
		uint size = 0;

		struct Cmd_Create
		{
			GLBufferHandle* handle;
			uint size;
			const void* data;
			AtomicBool* done;
		};

		struct Cmd_Destroy
		{
			GLBufferHandle* handle;

			Cmd_Destroy(GLBufferHandle* _handle) : handle(_handle) { }
			void Exec(void) { delete handle; }
		};

		struct Cmd_Map
		{
			GLBufferHandle* handle;
			MappingMode mode;
			uint offset;
			uint size;
			Atomic<uint8*>* ptr;
		};

		struct Cmd_Unmap
		{
			GLBufferHandle* handle;
		};

		struct Cmd_Copy
		{
			GLBufferHandle* dst;
			GLBufferHandle* src;
			uint srcOffset;
			uint dstOffset;
			uint size;
		};

		~GLBufferHandle(void);
		void Destroy(void);
		
		void Create(uint _size, const void* _data);
		void Create(GLCommandQueue& _queue, HwBufferUsage _usage, uint _size, const void* _data);
		static void Create(Cmd_Create* _cmd);

		uint8* Map(MappingMode _mode, uint _offset, uint _size);
		uint8* Map(GLCommandQueue& _queue, MappingMode _mode, uint _offset, uint _size);
		static void Map(Cmd_Map* _cmd);

		void Unmap(void);
		void Unmap(GLCommandQueue& _queue);
		static void Unmap(Cmd_Unmap* _cmd);

		void Copy(GLBufferHandle* _src, uint _srcOffset, uint _dstOffset, uint _size);
		void Copy(GLCommandQueue& _queue, GLBufferHandle* _src, uint _srcOffset, uint _dstOffset, uint _size);
		static void Copy(Cmd_Copy* _cmd);
	};



	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
