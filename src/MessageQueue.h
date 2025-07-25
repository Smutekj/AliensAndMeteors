#pragma once

#include "GameObject.h"

#include <iostream>
#include <queue>

enum class MessageType
{
    EntityDied,
    EntityCreated,
    ObjectiveFinished,
    ObjectiveFailed,
    ChangedGameState,
};

enum class DeathCause
{
    TooMuchAlcohol,
    CrashedIntoObject,
    KilledByPlayer,
    Timeout,
};

struct EntityDiedEvent 
{
    // EntityType type;
    int id;
    utils::Vector2f where;
    DeathCause cause;
};

struct EntityCreatedEvent
{
    // EntityType type;
    int id;
    utils::Vector2f where;
};

struct ObjectiveFinishedEvent
{
    int id;
};

using MessageTypes = std::tuple<std::queue<EntityDiedEvent>, std::queue<EntityCreatedEvent>, std::queue<ObjectiveFinishedEvent>> ;

#include <type_traits>

template <typename T, typename Tuple>
struct has_type;

template <typename T, typename... Us>
struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

template <class MessageType>
class Subscriber
{
    virtual void receive(MessageType msg) = 0;
};

using SubscriberTypes = std::tuple<Subscriber<EntityDiedEvent>, Subscriber<EntityCreatedEvent>, Subscriber<ObjectiveFinishedEvent>> ;

class TestSubscriber : public Subscriber<EntityDiedEvent>
{
    virtual void receive(EntityDiedEvent msg) override
    {
        std::cout << "Received Entity Died Msg" << std::endl;
    }
};

class MessageQueue
{
    public:

    template <class MessageData>
    void send(MessageData message_data)
    {
        static_assert(has_type<std::queue<MessageData>, MessageTypes>::value);

        std::get<std::queue<MessageData>>(m_messages).push(message_data);
    
        
    };

    template <class MessageData>
    void subscribeTo(Subscriber<MessageData>& subscriber)
    {

    }

    template <class MessageData>
    void unsubscribe(Subscriber<MessageData>& subscriber)
    {
        std::get<Subscriber<MessageData>>|(m_subscribers)
    }

    template<class MessageType>
    void distributeToSubscribers(MessageType msg)
    {
        std::get<Subscriber<MessageType>>(m_subscribers).receive(msg);
    }

    void distributeMessages()
    {
        std::apply([](auto& messages){
            while(!messages.empty())
            {
                auto message = messages.front();
                distributeToSubscribers(message);
                messages.pop();
            }
        }, m_messages);
    }

    private:
    MessageTypes m_messages;

    SubscriberTypes m_subscribers;
    

};


