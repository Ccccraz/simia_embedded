#ifndef EVENT_CENTER_H
#define EVENT_CENTER_H
#include <Arduino.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "singleton.h"

namespace simia
{
enum CMD : uint8_t
{
    Start = 0x00,
    Stop = 0x01,
    Reverse = 0x02,
    SetSpeed = 0x03
};

class EventCenter final : public Singleton<EventCenter>
{
    friend class Singleton<EventCenter>;

  protected:
    EventCenter() = default;
    ~EventCenter() = default;

  private:
    using Task_t = void (*)(std::shared_ptr<void>);
    std::unordered_map<CMD, std::vector<std::shared_ptr<Task_t>>> _event_map{};

  public:
    auto regsiter(const CMD cmd, const Task_t &task) -> void;
    auto remove(const CMD cmd, const Task_t &task) -> void;
    auto trigger(const CMD cmd) -> void;
    template <typename T> auto trigger(const CMD cmd, T msg = nullptr) -> void;
    auto clean() -> void;
};

inline auto EventCenter::regsiter(const CMD cmd, const Task_t &task) -> void
{
    if (_event_map.find(cmd) == _event_map.end())
    {
        std::vector<std::shared_ptr<Task_t>> tmp{std::make_shared<Task_t>(task)};
        _event_map.insert(std::make_pair(cmd, tmp));
    }
    else
    {
        _event_map.at(cmd).push_back(std::make_shared<Task_t>(task));
    }
}

inline auto EventCenter::remove(const CMD cmd, const Task_t &task) -> void
{
    if (_event_map.find(cmd) == _event_map.end())
    {
        return;
    }
    else
    {
        auto sub = _event_map.at(cmd);
        for (auto it = sub.begin(); it < sub.end(); it++)
        {
            if (**it == task)
            {
                sub.erase(it);
            }
        }
    }
}

inline auto EventCenter::trigger(const CMD cmd) -> void
{
    if (_event_map.find(cmd) == _event_map.end())
    {
        return;
    }
    else
    {
        for (auto &&i : _event_map.at(cmd))
        {

            (*i)(std::make_shared<nullptr_t>(nullptr));
        }
    }
}

template <typename T> inline auto EventCenter::trigger(const CMD cmd, T msg) -> void
{
    if (_event_map.find(cmd) == _event_map.end())
    {
        return;
    }
    else
    {
        for (auto &&i : _event_map.at(cmd))
        {

            (*i)(std::make_shared<T>(msg));
        }
    }
}

inline auto EventCenter::clean() -> void
{
    _event_map.clear();
}

} // namespace simia

#endif