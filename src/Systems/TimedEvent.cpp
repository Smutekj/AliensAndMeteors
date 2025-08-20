#include "TimedEvent.h"

TimedEvent::TimedEvent(float m_delay, std::function<void(float, int)> callback, TimedEventType type)
    : m_event_delay(m_delay), m_firing_time(m_delay), m_callback(callback), m_timer_type(type)
{
}

TimedEvent::TimedEvent(float m_delay, std::function<void(float, int)> callback,
                       int repeats_count)
    : m_event_delay(m_delay), m_firing_time(m_delay), m_callback(callback),
      m_total_repeats_count(repeats_count), m_repeats_left(repeats_count), m_timer_type(TimedEventType::Fixed)
{
}

void TimedEvent::update(float dt)
{
    m_firing_time -= dt;
    if (m_firing_time <= 0.f)
    {
        if(m_callback)
        {
            int repeat_count = (m_total_repeats_count - m_repeats_left);
            m_callback(m_event_delay*repeat_count, repeat_count);
        }
        m_repeats_left--;
        m_firing_time = m_event_delay;
    }
}

bool TimedEvent::isInfinite() const
{
    return m_timer_type == TimedEventType::Infinite;
}

float TimedEvent::getTimeLeft() const
{
    return m_firing_time + m_event_delay * (m_repeats_left-1);
}
int TimedEvent::getRepeatsLeft() const
{
    return m_repeats_left;
}
int TimedEvent::getTotalRepeats() const
{
    return m_total_repeats_count;
}