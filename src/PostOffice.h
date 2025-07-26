#pragma once


#include <queue>
#include <unordered_map>
#include <tuple>
#include <memory>
#include <functional>

#include "GameEvents.h"

template <class T> class PostBox;

class PostOffice
{

    template <class MessageT>
    using Callback = std::function<void(const std::deque<MessageT>&)>;
    template <class T>
    using SubscribersT = std::unordered_map<int, Callback<T>>;

    using MessageTypes = std::tuple<std::deque<EntityDiedEvent>, std::deque<EntityCreatedEvent>, std::deque<ObjectiveFinishedEvent>>;
    using SubscriberTypes = std::tuple<SubscribersT<EntityDiedEvent>, SubscribersT<EntityCreatedEvent>, SubscribersT<ObjectiveFinishedEvent>>;


    #include <type_traits>

    template <typename T, typename Tuple>
    struct has_type;

    template <typename T, typename... Us>
    struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...>
    {
    };

public:
    void distributeMessages();

    template <class MessageDataT>
    void send(MessageDataT message);
    
private:
    template <class MessageData>
    friend class PostBox;
    
    template <class MessageData>
    int subscribeTo(PostOffice::Callback<MessageData>& subscriber);

    template <class MessageDataT>
    void unsubscribe(int id);

    template <class MessageType>
    void distributeToSubscribers(std::deque<MessageType> &messages);

    template <class MessageDataT>
    int findNewFreeId();

    int next_id = 0;

    MessageTypes m_message_queues;
    SubscriberTypes m_subscribers;
};


#include "PostOffice.inl"