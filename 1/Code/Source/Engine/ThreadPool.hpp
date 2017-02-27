#pragma once

#include "Thread.hpp"
#include "Object.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class ThreadTask;
	typedef Ptr<ThreadTask> ThreadTaskPtr;

	class ThreadPool;
	typedef Ptr<ThreadPool> ThreadPoolPtr;

#define gThreadPool ThreadPool::Get()

	enum : uint
	{
		MIN_THREADS = 2,
		MAX_THREADS = 8,
		MAX_THREADS_WITH_RC = 2,
	};

	//----------------------------------------------------------------------------//
	// ThreadTask
	//----------------------------------------------------------------------------//

	enum ThreadTaskFlags : uint
	{
		//TTF_SingleThread = 0x1, ///!< Run this task in single thread.
		TTF_RequestRenderContext = 0x2,	///!< Request access to the render system or any a render object.
		TTF_DisableMoving = 0x4, ///!< Disable moving this task	between	threads for optimization.
	};

	class ThreadTask : public Object
	{
	public:
		OBJECT(ThreadTask);

		/// Run single step. \return false if this task is inactive.
		virtual bool Tick(uint _threadIndex = 0) = 0;

	protected:

	private:
		friend class ThreadPool;

		uint m_maxThreads;
		AtomicInt m_numThreads;
		bool m_requiredRc;
		volatile bool m_needRemove;
	};

	//----------------------------------------------------------------------------//
	// ThreadPool
	//----------------------------------------------------------------------------//

	class ThreadPool : public Object, public Singleton<ThreadPool>
	{
	public:
		OBJECT(ThreadPool);

		typedef void(*Callback)(uint _index, void* _param);

		struct InitParams
		{
			uint numThreads = 0;
			uint numThreadsWithRc = 0;
		};

		InitParams& GetInitParams(void) { return s_initParams; }
		ThreadPoolPtr Create(void);

		/// Run some function in each thread.
		///\note For experts and internal usage only.
		void _ExecInEachThread(Callback _callback, void* _param);

		/// Add new task for asynchronous execution.
		///\param[in] _thread specifies index of thread if flag ge::TTF_DisableMoving is presented, number of parallel executions otherwise.
		///\note Can be used from any thread.
		///\note Can be used from task.
		void AddTask(ThreadTask* _task, uint _flags, uint _thread = 0);

		void RemoveTask(ThreadTask* _task);

		void EmulateCpu(uint _numCores)
		{

		}
		//uint GetIndex(void);

	protected:
		friend class RenderSystem;

		ThreadPool(void);
		~ThreadPool(void);
		bool _Init(void);


		/*struct Task
		{
			double timeAccum;
			double frameTime;
			double avgFrameTime;
			ThreadTaskPtr task;
			uint flags;
		}; */

		struct Item
		{
			CriticalSection mutex;
			Array<ThreadTaskPtr> tasks;
			Array<ThreadTaskPtr> tasksToAdd;
			Thread thread;
			float fps;

			//double frameTime;
			//double avgFrameTime;
			//Array<Task> tasksToAdd;
			//Array<Task> tasks;
			//uint flags;
			//uint oldMask;

			//uint numSingleThreadTasks;
			//uint numMovalbleTasks;
		};

		void _BackgroundThread(uint _index);
		static void _ThreadEntry(uint _index);

		volatile Callback m_callback;
		volatile void* m_callbackParam;


		volatile bool m_stopThreads;
		AtomicInt m_cmdDone;
		volatile bool m_pauseThreads;

		uint m_numThreads;
		uint m_numThreadsWithRc;
		Item m_threads[MAX_THREADS];


		Mutex m_tasksMutex;
		Array<ThreadTaskPtr> m_tasks;
		//Array<ThreadTaskPtr> m_tasksToAdd;

		static InitParams s_initParams;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
