#pragma once

#include <functional>

enum class TimedEventType
{
    Fixed,
    Infinite
};

struct TimedEvent
{
    TimedEvent(float m_delay, std::function<void()> callback, TimedEventType type);
    TimedEvent(float m_delay, std::function<void()> callback,int repeats_count);

    void update(float dt);

    bool isInfinite() const;

    float getTimeLeft() const;
    int getRepeatsLeft() const;
    int getTotalRepeats() const;

private:
    TimedEventType m_timer_type;

    float m_event_delay;
    float m_firing_time;

    int m_repeats_left = 1;
    int m_total_repeats_count = 1;

    std::function<void()> m_callback = []() {};
};
