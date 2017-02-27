#include "ThreadPool.hpp"
#include "Time.hpp"
#include "Log.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// ThreadPool
	//----------------------------------------------------------------------------//

	namespace
	{
		THREAD_LOCAL int tls_threadIndex = -1;
	}

	ThreadPool::InitParams ThreadPool::s_initParams;

	//----------------------------------------------------------------------------//
	ThreadPoolPtr ThreadPool::Create(void)
	{
		if (s_instance)
			return s_instance;

		LOG_NODE("ThreadPool::Create");
		ThreadPoolPtr _pool = new ThreadPool();
		if (!_pool->_Init())
		{
			_pool = nullptr;
		}
		return _pool;
	}
	//----------------------------------------------------------------------------//
	ThreadPool::ThreadPool(void)
	{

	}
	//----------------------------------------------------------------------------//
	ThreadPool::~ThreadPool(void)
	{

	}
	//----------------------------------------------------------------------------//
	bool ThreadPool::_Init(void)
	{
		TODO("read cpu num cores");

		if (s_initParams.numThreads == 0)
			s_initParams.numThreads = MIN_THREADS; //TODO: cpu.numCores;
		if (s_initParams.numThreads < MIN_THREADS)
			s_initParams.numThreads = MIN_THREADS;
		if (s_initParams.numThreads > MAX_THREADS)
			s_initParams.numThreads = MAX_THREADS;

		if (s_initParams.numThreadsWithRc < MIN_THREADS)
			s_initParams.numThreadsWithRc = MIN_THREADS;
		if (s_initParams.numThreadsWithRc > MAX_THREADS_WITH_RC)
			s_initParams.numThreadsWithRc = MAX_THREADS_WITH_RC;
		if (s_initParams.numThreadsWithRc > s_initParams.numThreads)
			s_initParams.numThreadsWithRc = s_initParams.numThreads;

		for (uint i = 0; i < s_initParams.numThreads; ++i)
		{
			
		}
		return true;
	}
	//----------------------------------------------------------------------------//
	void ThreadPool::_ExecInEachThread(Callback _callback, void* _param)
	{
		if (_callback)
		{
			// init
			m_cmdDone = 0;
			m_callbackParam = _param;
			m_callback = _callback;
			// wait
			while (m_cmdDone < (int)m_numThreads)
				Thread::Sleep(1);
			// reset
			m_callback = nullptr;
			m_cmdDone = 0;
		}
	}
	//----------------------------------------------------------------------------//
	void ThreadPool::AddTask(ThreadTask* _task, uint _flags, uint _thread)
	{
		if (_task)
		{
			uint _first = (_flags & TTF_RequestRenderContext) ? 0 : (m_numThreads - m_numThreadsWithRc);
			uint _last = (_flags & TTF_RequestRenderContext) ? m_numThreadsWithRc : m_numThreads;
			if (_flags & TTF_DisableMoving)
			{
				uint _numTasks = 0, _index = 0;
				for (uint i = 0; i < _last; ++i)
				{
					
				}
			}
			else
			{

			}
		}
	}
	//----------------------------------------------------------------------------//
	void ThreadPool::RemoveTask(ThreadTask* _task)
	{
		if (_task)
		{

		}
	}
	//----------------------------------------------------------------------------//
	void ThreadPool::_BackgroundThread(uint _index)
	{
		uint _bit = 1 << _index;
		uint _publicIndex = _index + 1;
		bool _hasRc = _index < m_numThreadsWithRc;

		Array<ThreadTaskPtr> _commonTasks;

		// wait starting all threads
		++m_cmdDone;
		while (m_cmdDone < (int)m_numThreads)
			Thread::Sleep(1);

		uint _frame = 0;
		double _frameTime, _avgFrameTime = 0;
		Item& _ctx = m_threads[_index];

		while (!m_stopThreads)
		{
			// execution in each thread
			if (m_callback)
			{
				m_callback(_index, (void*)m_callbackParam);
				++m_cmdDone;
				while (m_cmdDone < (int)m_numThreads)
					Thread::Sleep(1);
			}

			// add tasks
			{
				SCOPE_LOCK(_ctx.mutex);
				if (!_ctx.tasksToAdd.empty())
				{
					_ctx.tasks.reserve(_ctx.tasks.size() + _ctx.tasksToAdd.size());
					while (!_ctx.tasksToAdd.empty())
					{
						ThreadTaskPtr _task = _ctx.tasksToAdd.back();
						++_task->m_numThreads;
						_ctx.tasks.push_back(_task);
						_ctx.tasksToAdd.pop_back();
					}
				}
			}

			// execute tasks
			for (size_t i = 0, s = _ctx.tasks.size(); i < s; ++i)
			{
				for (size_t i = 0; i < _ctx.tasks.size();)
				{
					ThreadTaskPtr _task = _ctx.tasks[i];
					if (_task->m_needRemove)
					{
						_ctx.tasks[i] = _ctx.tasks.back();
						_ctx.tasks.pop_back();
						--_task->m_numThreads;
					}
					else
					{
						++i;
						_task->Tick(_publicIndex);
					}
				}
			}

			// execute common tasks
			{
				// get common tasks
				{
					SCOPE_LOCK(m_tasksMutex);
					_commonTasks = m_tasks;
				}

				for (size_t i = 0, s = _commonTasks.size(); i < s; ++i)
				{
					ThreadTask* _task = _commonTasks[i];
					if (_task->m_requiredRc && !_hasRc)
						continue;

					if (++_task->m_numThreads < (int)_task->m_numThreads)
					{
						_task->Tick(_publicIndex);
					}
					--_task->m_numThreads;
				}
			}

		} // main loop

		// cleanup
		for (size_t i = 0, s = _ctx.tasks.size(); i < s; ++i)
			_ctx.tasks[i]->m_numThreads--;
		_ctx.tasksToAdd.clear();
		_ctx.tasks.clear();

		// wait stopping all threads
		++m_cmdDone;
		while (m_cmdDone < (int)m_numThreads)
			Thread::Sleep(1);
	}
	//----------------------------------------------------------------------------//
	void ThreadPool::_ThreadEntry(uint _index)
	{
		s_instance->_BackgroundThread(_index);

		/*if (_index < MAX_THREADS_WITH_RC)
		{
		gRenderSystem->RegisterThread();
		}


		if (_index < MAX_THREADS_WITH_RC)
		{
		gRenderSystem->UnregisterThread();
		}*/
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
