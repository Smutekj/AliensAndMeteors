#include "TimedEventManager.h"


TimedEventId TimedEventManager::addInfiniteEvent(float delay, std::function<void(float, int)> callback)
{
    TimedEvent event = {delay, callback, TimedEventType::Infinite};
    return m_events.addObject(event);
}

TimedEventId TimedEventManager::addTimedEvent(float delay, std::function<void(float, int)> callback, int repeats_count)
{
    TimedEvent event = {delay, callback, repeats_count};
    return m_events.addObject(event);
}

void TimedEventManager::removeEvent(TimedEventId id)
{
    m_events.remove(id);
}

void TimedEventManager::update(float dt)
{
    std::vector<TimedEventId> to_destroy;

    auto& events = m_events.getObjects();
    auto& event_ids = m_events.getEntityIds();
    for (std::size_t i = 0; i < events.size(); ++i)
    {
        auto &event = events[i];
        event.update(dt);
        if (event.getRepeatsLeft() == 0 && !event.isInfinite())
        {
            to_destroy.push_back(event_ids.at(i));
        }
    }

    for (auto event_id : to_destroy)
    {
        m_events.remove(event_id);
    }
}