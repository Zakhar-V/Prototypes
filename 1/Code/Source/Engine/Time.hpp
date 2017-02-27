#pragma once

#include "Object.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

	class Timer;
	typedef Ptr<Timer> TimerPtr;

#define gTimer ge::Timer::Get()

	//----------------------------------------------------------------------------//
	// Timer
	//----------------------------------------------------------------------------//

	class ENGINE_API Timer : public Object, public Singleton<Timer>
	{
	public:

		/// Create instance of Timer.
		static TimerPtr Create(void);

		/// Get time counter.
		uint64 Counter(void);
		/// Get timer frequency.
		uint64 Freq(void);
		/// Get current time in microseconds.
		double Microseconds(void);
		/// Get current time in milliseconds.
		double Milliseconds(void);
		/// Get current time in seconds.
		double Seconds(void);

	protected:

		Timer(void);
		~Timer(void);
		bool _Init(void);
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
