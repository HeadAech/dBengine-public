#include "Timer.h"
#include <dBengine/EngineDebug/EngineDebug.h>

/// <summary>
/// Constructor.
/// </summary>
/// <param name="m_TimeoutMs"> Time to timeout in ms</param>
/// <param name="m_OneShot"> Is OneShot [timeout once / loop]</param>
/// <param name="SignalOnTimeout"> Connected signal which will be called upon timeout </param>
Timer::Timer(float TimeoutMs, std::string SignalMessage, bool OneShot) {
    this->name = "Timer";
    this->icon = ICON_FA_CLOCK_O;
    this->TimeoutMs = TimeoutMs;
    StartTime = std::chrono::high_resolution_clock::now();
    ElapsedTime = std::chrono::high_resolution_clock::now();
    this->OneShot = OneShot;
    this->SignalMessage = SignalMessage;
}

void Timer::Reset() {
    StartTime = std::chrono::high_resolution_clock::now();
    ElapsedTime = StartTime;
    ElapsedMs = 0.0f;
    HasTimedOut = false;
}

void Timer::Start() {
    HasStarted = true;
}

/// <summary>
/// use externally to check the time.
/// </summary>
/// <returns></returns>
float Timer::GetElapsedMs() const { 
    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float, std::milli>(currentTime - StartTime).count();
}

/// <summary>
/// Updates time, emits signal when updated
/// </summary>
/// <param name="deltaTime">delta time elapsed IN SECONDS!! [it is converted to ms inside]</param>
void Timer::Update(float deltaTime) { 
    if (!HasStarted || (OneShot && HasTimedOut)) {
        return;
    }

    ElapsedMs += deltaTime * 1000; //ms conversion.
    
    //EngineDebug::GetInstance().PrintInfo(std::to_string(ElapsedMs) + "\n");
    if (ElapsedMs >= TimeoutMs) {
        HasTimedOut = true;
        if (!OneShot) {
            Reset();
        }
        //EngineDebug::GetInstance().PrintInfo("TIMEOUT");
        Signals::Timer_OnTimeout.emit(SignalMessage);
        return; 
    }
    return;
}
