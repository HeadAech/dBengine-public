#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <functional>
#include <mutex>

class Scheduler
{
	public:
		Scheduler();
        ~Scheduler();

		void Schedule(std::function<void()> task);
        void ExecuteTasks();

	private:
		std::queue<std::function<void()>> m_Tasks;
		std::mutex m_TasksMutex;
};

#endif // SCHEDULER_H
