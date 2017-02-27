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
