#pragma once
#include "spinlock.h"
#include "utils.h"

namespace minico {

	class SpinlockGuard
	{
	public:
		SpinlockGuard(Spinlock& l)
			: lock_(l)
		{
			lock_.lock();
		}

		~SpinlockGuard()
		{
			lock_.unlock();
		}

		DISALLOW_COPY_MOVE_AND_ASSIGN(SpinlockGuard);

	private:
		Spinlock& lock_;

	};

}
