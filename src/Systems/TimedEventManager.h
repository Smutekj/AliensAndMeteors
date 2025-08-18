#pragma once

#include "../Utils/ObjectPool.h"
#include "TimedEvent.h"

using TimedEventId = int;
constexpr int MAX_TIMED_EVENT_COUNT = 1000;

class TimedEventManager
{
public:

    TimedEventId addInfiniteEvent(float delay, std::function<void(float, int)> callback);
    TimedEventId addTimedEvent(float delay, std::function<void(float, int)> callback, int repeats_count = 1);

    void removeEvent(TimedEventId id);

    void update(float dt);

private:
    utils::DynamicObjectPool<TimedEvent, MAX_TIMED_EVENT_COUNT> m_events;
};