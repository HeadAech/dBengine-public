#include "Scheduler.h"

Scheduler::Scheduler()
{}

Scheduler::~Scheduler()
{}

void Scheduler::Schedule(std::function<void()> task)
{
    std::lock_guard<std::mutex> lock(m_TasksMutex);
    m_Tasks.push(std::move(task));
}

void Scheduler::ExecuteTasks()
{
    std::lock_guard<std::mutex> lock(m_TasksMutex);
    while (!m_Tasks.empty()) 
    {
        auto task = std::move(m_Tasks.front());
        m_Tasks.pop();
        task();
    }
}
