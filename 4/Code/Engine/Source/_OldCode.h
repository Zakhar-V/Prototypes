//----------------------------------------------------------------------------//
// Defs (Event)
//----------------------------------------------------------------------------//

class EventHandler;
class EventBase;
struct EventPipe;
struct EventGroup;


//----------------------------------------------------------------------------//
// EventCallback
//----------------------------------------------------------------------------//

struct EventCallback
{
	typedef void(*InvokeFunc)(void* _func, EventHandler* _receiver, EventHandler* _sender, void* _data);

	EventCallback(void) { }

	template <class R, class S, class E> EventCallback(void(*_func)(R*, S*, E)) :
		func(FuncPtr(_func)),
		invoke(&_InvokeFunc<R, S, E>)
	{
	}
	template <class R, class S> EventCallback(void(*_func)(R*, S*)) :
		func(FuncPtr(_func)),
		invoke(&_InvokeFuncNoArg<R, S>)
	{
	}
	template <class R, class S, class E> EventCallback(void(R::*_func)(S*, E)) :
		func(FuncPtr(_func)),
		invoke(&_InvokeMethod<R, S, E>)
	{
	}
	template <class R, class S> EventCallback(void(R::*_func)(S*)) :
		func(FuncPtr(_func)),
		invoke(&_InvokeMethodNoArg<R, S>)
	{
	}

	void Invoke(EventHandler* _receiver, EventHandler* _sender, void* _data)
	{
		invoke(func, _receiver, _sender, _data);
	}

	void* func = nullptr;
	InvokeFunc invoke = nullptr;

	template <class R, class S, class E> static void _InvokeFunc(void* _func, EventHandler* _receiver, EventHandler* _sender, void* _data)
	{
		FuncCast<void(*)(R*, S*, E)>(_func)(static_cast<R*>(_receiver), static_cast<S*>(_sender), *reinterpret_cast<E*>(_data));
	}
	template <class R, class S> static void _InvokeFuncNoArg(void* _func, EventHandler* _receiver, EventHandler* _sender, void* _data)
	{
		FuncCast<void(*)(R*, S*)>(_func)(static_cast<R*>(_receiver), static_cast<S*>(_sender));
	}
	template <class R, class S, class E> static void _InvokeMethod(void* _func, EventHandler* _receiver, EventHandler* _sender, void* _data)
	{
		((*static_cast<R*>(_receiver)).*FuncCast<void(R::*)(S*, E)>(_func))(static_cast<S*>(_sender), *reinterpret_cast<E*>(_data));
	}
	template <class R, class S> static void _InvokeMethodNoArg(void* _func, EventHandler* _receiver, EventHandler* _sender, void* _data)
	{
		((*static_cast<R*>(_receiver)).*FuncCast<void(R::*)(S*)>(_func))(static_cast<S*>(_sender));
	}
};

//----------------------------------------------------------------------------//
// EventPipe
//----------------------------------------------------------------------------//

struct EventPipe final
{
private:
	friend class EventHandler;
	friend class EventBase;

	EventGroup* sender = nullptr;
	EventGroup* receiver = nullptr;
	EventCallback callback;
	EventBase* event = nullptr;
	EventPipe* prevReceiver = nullptr;
	EventPipe* nextReceiver = nullptr;
	EventPipe* prevSender = nullptr;
	EventPipe* nextSender = nullptr;

	void _Link(void);
	void _Unlink(void);
};

//----------------------------------------------------------------------------//
// EventGroup
//----------------------------------------------------------------------------//

struct EventGroup final
{
private:
	friend class EventHandler;
	friend class EventBase;
	friend struct EventPipe;

	EventBase* event = nullptr;
	EventHandler* self = nullptr;
	EventPipe* receivers = nullptr;
	EventPipe* senders = nullptr;
	EventGroup* next = nullptr;
};

//----------------------------------------------------------------------------//
// EventBase
//----------------------------------------------------------------------------//

class EventBase : public NonCopyable
{
public:

	EventBase(void);
	~EventBase(void);
	void Unsubscribe(EventHandler* _receiver);
	void Unsubscribe(EventHandler* _receiver, EventHandler* _sender);

protected:
	friend class EventHandler;
	friend struct EventPipe;

	void _Subscribe(EventHandler* _sender, EventHandler* _receiver, const EventCallback& _callback);
	void _Send(EventHandler* _sender, void* _data);

	EventPipe* m_receivers;
};

//----------------------------------------------------------------------------//
// Event
//----------------------------------------------------------------------------//

template <class T> class Event : public EventBase
{
public:

	template <class R, class S> void Subscribe(EventHandler* _sender, EventHandler* _receiver, void(*_func)(R*, S*))
	{
		_Subscribe(_sender, _receiver, _func);
	}
	template <class R, class S> void Subscribe(EventHandler* _sender, EventHandler* _receiver, void(*_func)(R*, S*, T))
	{
		_Subscribe(_sender, _receiver, _func);
	}
	template <class R, class S> void Subscribe(EventHandler* _sender, EventHandler* _receiver, void(*_func)(R*, S*, const T&))
	{
		_Subscribe(_sender, _receiver, _func);
	}
	template <class R, class S> void Subscribe(EventHandler* _sender, EventHandler* _receiver, void(R::*_func)(S*))
	{
		_Subscribe(_sender, _receiver, _func);
	}
	template <class R, class S> void Subscribe(EventHandler* _sender, EventHandler* _receiver, void(R::*_func)(S*, T))
	{
		_Subscribe(_sender, _receiver, _func);
	}
	template <class R, class S> void Subscribe(EventHandler* _sender, EventHandler* _receiver, void(R::*_func)(S*, const T&))
	{
		_Subscribe(_sender, _receiver, _func);
	}
	void Send(EventHandler* _sender, const T& _data)
	{
		_Send(_sender, reinterpret_cast<void*>(const_cast<T*>(&_data)));
	}
};

//----------------------------------------------------------------------------//
// EventHandler
//----------------------------------------------------------------------------//

// memory use = num_events * sizeof(void*) * 5 + num_receivers * sizeof(void*) * 9
// x86 num_events * 20 + num_receivers * 36
// x64 num_events * 40 + num_receivers * 72
class EventHandler : public NonCopyable
{
public:

	EventHandler(void);
	~EventHandler(void);

	template <class R, class S, class E> void Subscribe(Event<E>& _e, void(*_func)(R*, S*))
	{
		_Subscribe(&_e, nullptr, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, EventHandler* _sender, void(*_func)(R*, S*))
	{
		_Subscribe(&_e, _sender, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, void(*_func)(R*, S*, E))
	{
		_Subscribe(&_e, nullptr, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, EventHandler* _sender, void(*_func)(R*, S*, E))
	{
		_Subscribe(&_e, _sender, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, void(*_func)(R*, S*, const E&))
	{
		_Subscribe(&_e, nullptr, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, EventHandler* _sender, void(*_func)(R*, S*, const E&))
	{
		_Subscribe(&_e, _sender, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, void(R::*_func)(S*))
	{
		_Subscribe(&_e, nullptr, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, EventHandler* _sender, void(R::*_func)(S*))
	{
		_Subscribe(&_e, _sender, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, void(R::*_func)(S*, E))
	{
		_Subscribe(&_e, nullptr, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, EventHandler* _sender, void(R::*_func)(S*, E))
	{
		_Subscribe(&_e, _sender, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, void(R::*_func)(S*, const E&))
	{
		_Subscribe(&_e, nullptr, _func);
	}
	template <class R, class S, class E> void Subscribe(Event<E>& _e, EventHandler* _sender, void(R::*_func)(S*, const E&))
	{
		_Subscribe(&_e, _sender, _func);
	}

	void Unsubscribe(void);
	void Unsubscribe(EventBase* _event);
	void Unsubscribe(EventBase* _event, EventHandler* _sender);
	void Unsubscribe(EventHandler* _sender);

	template <class E> void Send(Event<E>& _e, const E& _data)
	{
		_e.Send(this, _data);
	}

protected:
	friend class EventBase;
	friend struct EventPipe;

	void _Subscribe(EventBase* _event, EventHandler* _sender, const EventCallback& _callback);
	EventGroup* _GetEventGroup(EventBase* _event);
	EventGroup* _AddEventGroup(EventBase* _event);

	EventGroup* m_events;
};
//----------------------------------------------------------------------------//
// EventPipe
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void EventPipe::_Link(void)
{
	ASSERT(receiver != nullptr);
	ASSERT(event != nullptr);
	ASSERT(prevSender == nullptr);
	ASSERT(nextSender == nullptr);
	ASSERT(prevReceiver == nullptr);
	ASSERT(nextReceiver == nullptr);

	// link to receiver
	nextSender = receiver->senders;
	receiver->senders = this;
	if (nextSender)
		nextSender->prevSender = this;

	// link to sender
	if (sender)
	{
		nextReceiver = sender->receivers;
		sender->receivers = this;
	}
	else
	{
		nextReceiver = event->m_receivers;
		event->m_receivers = this;
	}

	if (nextReceiver)
		nextReceiver->prevReceiver = this;
}
//----------------------------------------------------------------------------//
void EventPipe::_Unlink(void)
{
	ASSERT(receiver != nullptr);
	ASSERT(event != nullptr);

	// unlink from receiver
	if (nextSender)
		nextSender->prevSender = prevSender;
	if (prevSender)
		prevSender->nextSender = nextSender;
	else
		receiver->senders = nextSender;
	//receiver->m_eventSenders = nextSender;

	nextSender = nullptr;
	prevSender = nullptr;

	// unlink from sender
	if (nextReceiver)
		nextReceiver->prevReceiver = prevReceiver;
	if (prevReceiver)
		prevReceiver->nextReceiver = nextReceiver;
	else if (sender)
		sender->receivers = nextReceiver;
	else
		event->m_receivers = nextReceiver;

	nextReceiver = nullptr;
	prevReceiver = nullptr;
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// EventBase
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
EventBase::EventBase(void) :
	m_receivers(nullptr)
{
}
//----------------------------------------------------------------------------//
EventBase::~EventBase(void)
{
	for (EventPipe* _p; m_receivers;)
	{
		_p = m_receivers;
		_p->_Unlink();
		delete _p;
	}
}
//----------------------------------------------------------------------------//
void EventBase::Unsubscribe(EventHandler* _receiver)
{
	if (_receiver)
		_receiver->Unsubscribe(this);
}
//----------------------------------------------------------------------------//
void EventBase::Unsubscribe(EventHandler* _receiver, EventHandler* _sender)
{
	if (_receiver)
		_receiver->Unsubscribe(this);
}
//----------------------------------------------------------------------------//
void EventBase::_Subscribe(EventHandler* _sender, EventHandler* _receiver, const EventCallback& _callback)
{
	if (_receiver)
		_receiver->_Subscribe(this, _sender, _callback);
}
//----------------------------------------------------------------------------//
void EventBase::_Send(EventHandler* _sender, void* _data)
{
	for (EventPipe *_n, *_p = m_receivers; _p;)
	{
		_n = _p->nextReceiver;
		_p->callback.Invoke(_p->receiver->self, _sender, _data);
		_p = _n;
	}

	if (_sender)
	{
		EventGroup* _g = _sender->_GetEventGroup(this);
		if (_g)
		{
			for (EventPipe *_n, *_p = _g->receivers; _p;)
			{
				_n = _p->nextReceiver;
				_p->callback.Invoke(_p->receiver->self, _sender, _data);
				_p = _n;
			}
		}
	}
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// EventHandler
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
EventHandler::EventHandler(void) :
	m_events(nullptr)
{
}
//----------------------------------------------------------------------------//
EventHandler::~EventHandler(void)
{
	for (EventGroup* _g = m_events; _g;)
	{
		for (EventPipe* _p; _g->senders;)
		{
			_p = _g->senders;
			_p->_Unlink();
			delete _p;
		}
		for (EventPipe* _p; _g->receivers;)
		{
			_p = _g->receivers;
			_p->_Unlink();
			delete _p;
		}

		EventGroup* _n = _g->next;
		delete _g;
		_g = _n;
	}
}
//----------------------------------------------------------------------------//
void EventHandler::Unsubscribe(void)
{
	for (EventGroup* _g = m_events; _g; _g = _g->next)
	{
		for (EventPipe* _p; _g->senders;)
		{
			_p = _g->senders;
			_p->_Unlink();
			delete _p;
		}
	}
}
//----------------------------------------------------------------------------//
void EventHandler::Unsubscribe(EventBase* _event)
{
	EventGroup* _g = _GetEventGroup(_event);
	if (_g)
	{
		for (EventPipe* _p; _g->senders;)
		{
			_p = _g->senders;
			_p->_Unlink();
			delete _p;
		}
	}
}
//----------------------------------------------------------------------------//
void EventHandler::Unsubscribe(EventBase* _event, EventHandler* _sender)
{
	EventGroup* _g = _GetEventGroup(_event);
	if (_g && _sender)
	{
		for (EventPipe* _p = _g->senders; _p; _p = _p->nextSender)
		{
			if (_p->sender->self == _sender)
			{
				_p->_Unlink();
				delete _p;
				return;
			}
		}
	}
}
//----------------------------------------------------------------------------//
void EventHandler::Unsubscribe(EventHandler* _sender)
{
	if (_sender)
	{
		for (EventGroup* _g = m_events; _g; _g = _g->next)
		{
			for (EventPipe* _p = _g->senders; _p; _p = _p->nextSender)
			{
				if (_p->sender->self == _sender)
				{
					_p->_Unlink();
					delete _p;
					break;
				}
			}
		}
	}
}
//----------------------------------------------------------------------------//
void EventHandler::_Subscribe(EventBase* _event, EventHandler* _sender, const EventCallback& _callback)
{
	if (!_event || !_callback.func)
		return;

	EventGroup* _g = _AddEventGroup(_event);
	if (_sender)
	{
		// find and remove shared pipe to event
		for (EventPipe* _p = _g->senders; _p; _p = _p->nextSender)
		{
			if (_p->sender->self == _sender)
			{
				_p->callback = _callback;
				return;
			}
			else if (!_p->sender)
			{
				_p->_Unlink();
				delete _p;
				break;
			}
		}

		// create unique pipe to specified sender
		EventPipe* _p = new EventPipe;
		_p->receiver = _g;
		_p->sender = _sender->_AddEventGroup(_event);
		_p->event = _event;
		_p->callback = _callback;
		_p->_Link();
	}
	else
	{
		// find and remove all unique pipes to event
		for (EventPipe *_n, *_p = _g->senders; _p; _p = _p->nextSender)
		{
			_n = _p->nextSender;
			if (_p->sender)
			{
				_p->_Unlink();
				delete _p;
			}
			else
			{
				_p->callback = _callback;
				return;
			}
			_p = _n;
		}

		// create shared pipe to event
		EventPipe* _p = new EventPipe;
		_p->receiver = _g;
		_p->event = _event;
		_p->callback = _callback;
		_p->_Link();
	}
}
//----------------------------------------------------------------------------//
EventGroup* EventHandler::_GetEventGroup(EventBase* _event)
{
	for (EventGroup* _g = m_events; _g; _g = _g->next)
	{
		if (_g->event == _event)
			return _g;
	}
	return nullptr;
}
//----------------------------------------------------------------------------//
EventGroup* EventHandler::_AddEventGroup(EventBase* _event)
{
	EventGroup* _g = m_events;
	for (; _g; _g = _g->next)
	{
		if (_g->event == _event)
			break;
	}
	if (!_g)
	{
		_g = new EventGroup;
		_g->self = this;
		_g->event = _event;
		_g->next = m_events;
		m_events = _g;
	}
	return _g;
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//


//----------------------------------------------------------------------------//
// Defs
//----------------------------------------------------------------------------//

#define gRenderSystem Rx::RenderSystem::Get()
#define gRenderFeatures Rx::RenderSystem::Get()->GetFeatures()

typedef SharedPtr<class HardwareBuffer> HardwareBufferPtr;
typedef SharedPtr<class HardwareShaderStage> HardwareShaderStagePtr;


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

enum HardwareBufferType : uint8
{
	HBT_Vertex,
	HBT_Index,
	HBT_Uniform,
	HBT_DrawIndirect,
	HBT_Texture, //!<\note cannot be created directly. use Rx::RenderSystem::CreateTexture with Rx::TT_Buffer.

				 //BUFFER_SHADER_STORAGE,
};

enum HardwareBufferUsage : uint8
{
	HBU_Default, //!< optimized for usage on GPU. \note buffer can be read or updated on CPU (not very much effective)
	HBU_DynamicRead, //!< optimized for reading on CPU. \note buffer cannot be updated on CPU.
	HBU_DynamicWrite, //!< optimized for overwriting on CPU. Use Rx::MM_WriteDiscard for updating buffer. \note buffer cannot be attached to output stage. \note buffer cannot be read on CPU.
};

enum MappingMode : uint8
{
	MM_None = AM_None,
	MM_Read = AM_Read,
	MM_Write = AM_Write, //!< For partial write of mapped data. Not very much effective, because required read of actual data before mapping data to CPU.
	MM_ReadWrite = AM_ReadWrite,
	MM_WriteDiscard = AM_Write | 0x4, //!< For full overwrite of mapped data. Effective for buffer with Rx::HBU_DynamicWrite.
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

class RenderContext : public NonCopyable
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

	///\param[in] _elementSize specify size of each element in buffer. It's obligatory when _type is Rx::HBU_Uniform, use as hint otherwise. 
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
//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//
#define CLASSNAME(T) \
	static uint ClassIdStatic(void) { static const NameHash _clsid(ClassNameStatic()); return _clsid; } \
	static const String& ClassNameStatic(void) { static const String _clsname(#T); return _clsname; } \
	uint ClassId(void) override { return ClassIdStatic(); } \
	const String& ClassName(void) override  { return ClassNameStatic(); } \
	bool IsClass(const NameHash& _id) override { return _id.hash == ClassIdStatic() || __super::IsClass(_id); } \
	template <class X> bool IsClass(void) { return IsClass(X::ClassIdStatic()); }

static uint ClassIdStatic(void) { static const NameHash _clsid(ClassNameStatic()); return _clsid; }
static const String& ClassNameStatic(void) { static const String _clsname("BaseObject"); return _clsname; }
virtual uint ClassId(void) { return ClassIdStatic(); }
virtual const String& ClassName(void) { return ClassNameStatic(); }
virtual bool IsClass(const NameHash& _id) { return _id.hash == ClassIdStatic(); }
template <class T> bool IsClass(void) { return IsClass(T::ClassIdStatic()); }

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//
