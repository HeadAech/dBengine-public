//
// Created by Hubert Klonowski on 14/03/2025.
//
#pragma once
#ifndef SIGNAL_H
#define SIGNAL_H
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

/// <summary>
/// Class for implementing a signal-slot mechanism (Godot Engine's approach).
/// </summary>
/// <typeparam name="...Args"></typeparam>
template <typename... Args>
class Signal {


    public:

    using Slot = std::function<void(Args...)>;

    /// <summary>
    /// Connects a slot to the signal using a UUID.
    /// </summary>
    /// <param name="UUID"></param>
    /// <param name="slot"></param>
    /// <param name="targetMessage">Who has to react to emitted signal</param>
    void connect(std::string UUID, const Slot &slot, std::string targetMessage = "") { 
        slots.emplace(UUID, SlotInfo{slot, targetMessage});
    }

    /// <summary>
    /// Emits the signal, calling all connected slots with the provided arguments.
    /// </summary>
    /// <param name="...args">Arguments</param>
    void emit(Args... args, std::string targetMessage = "") {
        for (const auto &[uuid, slot_info]: slots) {
            if (slot_info.slot && (slot_info.targetMessage == targetMessage)){
                slot_info.slot(args...);
            }
        }
    }

    /// <summary>
    /// Disconnects a slot from the signal using its UUID.
    /// </summary>
    /// <param name="UUID"></param>
    void disconnect(std::string UUID) { 
        slots.erase(UUID);
    }

private:
    //it is easier this way trust me.
    struct SlotInfo {
        Slot slot;
        std::string targetMessage;
    };
    std::unordered_map<std::string, SlotInfo> slots;
};


#endif //SIGNAL_H
