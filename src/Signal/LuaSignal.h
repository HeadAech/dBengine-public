#pragma once
#ifndef LUA_SIGNAL_H
#define LUA_SIGNAL_H

#include <string>
#include <unordered_map>
#include <vector>
#include <sol.hpp>

class LuaSignal {
public:
    void connect(const std::string &uuid, sol::function slot) {
        slots[uuid] = slot;
    }

    void emit(sol::variadic_args args) {
        std::vector<std::string> to_remove;
        for (const auto &[uuid, slot]: slots) {
            if (slot.valid()) {
                slot.call(args);
            } else {
                to_remove.push_back(uuid);
            }
        }
        for (const auto &uuid: to_remove) {
            slots.erase(uuid);
        }

    }

    void disconnect(const std::string &uuid) { 
        slots.erase(uuid); 
    }

private:

    std::unordered_map<std::string, sol::function> slots;
};


static std::unordered_map<std::string, LuaSignal> global_signals;

#endif // LUA_SIGNAL_H
