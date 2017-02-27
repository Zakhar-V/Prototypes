#include "Object.hpp"

namespace ge
{

	//----------------------------------------------------------------------------//
	// Object
	//----------------------------------------------------------------------------//

	Mutex Object::s_namesMutex;
	HashMap<uint32, String> Object::s_names;

	//----------------------------------------------------------------------------//
	void Object::SetName(const String& _name)
	{
		m_name = _name.NameHash();
		SCOPE_LOCK(s_namesMutex);
		s_names[m_name] = _name;
	}
	//----------------------------------------------------------------------------//
	const String& Object::GetName(void)
	{
		SCOPE_READ(s_namesMutex);
		auto _name = s_names.find(m_name);
		return _name != s_names.end() ? _name->second : String::Empty;
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Event
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	Event::~Event(void)
	{
		for (auto i = m_callbacks.begin(); i != m_callbacks.end();)
		{
			delete i->second;
			i = m_callbacks.erase(i);
		}
	}
	//----------------------------------------------------------------------------//
	void Event::Subscribe(Object* _receiver, EventCallback* _callback)
	{
		ASSERT(_receiver != nullptr);
		ASSERT(_callback != nullptr);
		m_callbacks[_receiver] = _callback;
	}
	//----------------------------------------------------------------------------//
	void Event::Unsubscribe(Object* _receiver)
	{
		auto _it = m_callbacks.find(_receiver);
		if (_it != m_callbacks.end())
			_it->second->SetReceiver(nullptr);
	}
	//----------------------------------------------------------------------------//
	bool Event::IsSubscribed(Object* _receiver)
	{
		auto _it = m_callbacks.find(_receiver);
		if (_it != m_callbacks.end())
			return _it->second->GetReceiver() == _receiver;
		return false;
	}
	//----------------------------------------------------------------------------//
	void Event::Send(Object* _sender)
	{
		bool _hasNullCallbacks = false;

		m_sender = _sender;
		for (auto i = m_callbacks.begin(), e = m_callbacks.end(); i != e; ++i)
		{
			EventCallback* _cb = i->second;
			if (_cb->GetReceiver())
				_cb->Invoke(this);
			else
				_hasNullCallbacks = true;
		}
		m_sender = nullptr;

		if (_hasNullCallbacks)
		{
			for (auto i = m_callbacks.begin(); i != m_callbacks.end();)
			{
				if (i->second->GetReceiver())
					++i;
				else
				{
					delete i->second;
					i = m_callbacks.erase(i);
				}
			}
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}