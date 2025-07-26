#pragma once

#include <queue>
#include <unordered_map>
#include <tuple>
#include <memory>
#include <functional>
#include <typeindex>

#include "GameEvents.h"

template <class T>
class PostBox;

using SubscriptionId = int;

template <class MessageT>
using Callback = std::function<void(const std::deque<MessageT> &)>;
template <class T>
using SubscribersT = std::unordered_map<SubscriptionId, Callback<T>>;

class MessageHolderI
{
public:
    virtual void distribute() = 0;
};

template <class MessageType>
class MessageHolder : public MessageHolderI
{
public:
    void sendNow(MessageType message)
    {
        assert(false); //! TODO will implement
        // for (auto [id, call_back] : subscribers)
        // {
        //     call_back(messages);
        // }
    }
    void send(MessageType message)
    {
        messages.push_back(message);
    }

    int subscribe(Callback<MessageType> subscriber)
    {
        int new_id = findNewFreeId();
        assert(subscribers.count(new_id) == 0);
        subscribers[new_id] = subscriber;
        return new_id;
    }

    void unsubscribe(int id)
    {
        subscribers.erase(id);
    }

    virtual void distribute()
    {
        for (auto [id, call_back] : subscribers)
        {
            call_back(messages);
        }
        messages.clear();
    };

private:
    inline int findNewFreeId()
    {
        while (subscribers.count(next_id) != 0)
        {
            next_id++;
        }
        return next_id;
    }

    int next_id = 0;
    SubscribersT<MessageType> subscribers;
    std::deque<MessageType> messages;
};

class PostOffice
{

public:
    void distributeMessages();

    template <class MessageDataT>
    void send(MessageDataT message);

    template <class MessageDataT>
    void registerEvent();

    template <class ...MessageData>
    void registerEvents();

private:
    template <class MessageData>
    friend class PostBox;

    template <class MessageData>
    bool isRegistered() const;

    template <class MessageData>
    MessageHolder<MessageData> &getHolder();

    template <class MessageData>
    int subscribeTo(Callback<MessageData> &subscriber);

    template <class MessageDataT>
    void unsubscribe(int id);

    // template <class MessageType>
    // void distributeToSubscribers(std::deque<MessageType> &messages);

    std::unordered_map<std::type_index, std::unique_ptr<MessageHolderI>> m_holders;
};

// using PostOfficeA = PostOffice<EntityDiedEvent, EntityCreatedEvent, ObjectiveFinishedEvent>;

#include "PostOffice.inl"
// #pragma once

// #include <queue>
// #include <unordered_map>
// #include <tuple>
// #include <memory>
// #include <functional>

// #include "GameEvents.h"

// template <class T> class PostBox;

// class MessageHolderI
// {
//     // virtual void /
// };

// template <class MessageType>
// class MessageHolder : public MessageHolderI
// {
//     std::function<void(const std::deque<MessageType>&)> subscribers;
//     std::deque<MessageType> messages;
// };

// class PostOffice
// {

//     template <class MessageT>
//     using Callback = std::function<void(const std::deque<MessageT>&)>;
//     template <class T>
//     using SubscribersT = std::unordered_map<int, Callback<T>>;

//     // using MessageTypes = std::tuple<std::deque<EntityDiedEvent>, std::deque<EntityCreatedEvent>, std::deque<ObjectiveFinishedEvent>>;
//     // using SubscriberTypes = std::tuple<SubscribersT<EntityDiedEvent>, SubscribersT<EntityCreatedEvent>, SubscribersT<ObjectiveFinishedEvent>>;

//     using MessageTypes = std::tuple<std::deque<RegisteredTypes>...>;
//     using SubscriberTypes = std::tuple<SubscribersT<RegisteredTypes>...>;

//     #include <type_traits>

//     template <typename T, typename Tuple>
//     struct has_type;

//     template <typename T, typename... Us>
//     struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...>
//     {
//     };

// public:
//     void distributeMessages();

//     template <class MessageDataT>
//     void send(MessageDataT message);

//     template <class MessageDataT>
//     void registerEvent();

// private:
//     template <class MessageData>
//     friend class PostBox;

//     template <class MessageData>
//     int subscribeTo(PostOffice::Callback<MessageData>& subscriber);

//     template <class MessageDataT>
//     void unsubscribe(int id);

//     template <class MessageType>
//     void distributeToSubscribers(std::deque<MessageType> &messages);

//     template <class MessageDataT>
//     int findNewFreeId();

//     int next_id = 0;

//     MessageTypes m_message_queues;
//     SubscriberTypes m_subscribers;
// };

// using PostOfficeA = PostOffice<EntityDiedEvent, EntityCreatedEvent, ObjectiveFinishedEvent>;

// #include "PostOffice.inl"