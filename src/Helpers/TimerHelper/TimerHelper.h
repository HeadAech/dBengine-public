#ifndef TIMERHELPER_H
#define TIMERHELPER_H

#include <chrono>
#include <thread>
#include <dBengine/EngineDebug/EngineDebug.h>

/// <summary>
/// Utility class to measure execution time of functions. Prints out duration on function exit.
/// <para>Usage: <code>Timer(section name)</code>.</para>
/// </summary>
class TimerHelper
{
    public:

        /// <summary>
        /// Constructor that starts the timer.
        /// </summary>
        /// <param name="name">Name of the timer</param>
        TimerHelper(std::string_view name) 
        {   
            m_Name = name;
            m_StartTime = std::chrono::high_resolution_clock::now();
        };

        /// <summary>
        /// Destructor that stops the timer and logs the duration.
        /// </summary>
        ~TimerHelper()
        {
            m_End = std::chrono::high_resolution_clock::now();
            m_Duration = m_End - m_StartTime;

            float ms = m_Duration.count() * 1000.0f;
            EngineDebug::GetInstance().PrintDebug(std::format("Timer [{}]: {}ms", m_Name, ms));
        }


    private:
        
        std::string_view m_Name;

        std::chrono::time_point<std::chrono::steady_clock> m_StartTime, m_End;
        std::chrono::duration<float> m_Duration;
};

#endif // !TIMER_H
