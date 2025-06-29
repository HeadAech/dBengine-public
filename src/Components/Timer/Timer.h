#ifndef TIMER_H
#define TIMER_H

#pragma once
#include "Component/Component.h"
#include <Signal/Signal.h>
#include <Signal/Signals.h>
#include <chrono>

class Timer : public Component {
public:
    Timer(float TimeoutMs, std::string SignalMessage, bool OneShot = false);
    ~Timer() = default;

    void Reset();
    void Start();
    float GetElapsedMs() const;
    void Update(float deltaTime) override;

    bool OneShot;
    bool HasStarted = false;
    bool HasTimedOut = false;
    float TimeoutMs;
    std::string SignalMessage;
    std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> ElapsedTime;
    float ElapsedMs = 0.0;

private:

};

#endif // !TIMER_H
