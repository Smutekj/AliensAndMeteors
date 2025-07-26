#pragma once


// template <class ...RegisteredTypes>
// inline void PostOffice<RegisteredTypes...>::distributeMessages()
// {
//     std::apply([this](auto&...messages)
//     { (distributeToSubscribers(messages),...); }, m_message_queues);
// }

// template <class ...RegisteredTypes>
// template <class MessageType>
// inline void PostOffice<RegisteredTypes...>::distributeToSubscribers(std::deque<MessageType> &messages)
// {
//     auto &subs = std::get<SubscribersT<MessageType>>(m_subscribers);
//     for (auto &[id, post_box] : subs)
//     {
//         // if(post_box.expired())
//         // {
//             // subs.erase(id);
//         // }else{
//             post_box(messages);
//         // }
//     }
//     messages.clear();
// }

// template <class ...RegisteredTypes>
// template <class MessageDataT>
// inline void PostOffice<RegisteredTypes...>::send(MessageDataT message)
// {
//     std::get<std::deque<MessageDataT>>(m_message_queues).push_back(message);
// }

// template <class ...RegisteredTypes>
// template <class MessageData>
// inline int PostOffice<RegisteredTypes...>::subscribeTo(PostOffice<RegisteredTypes...>::Callback<MessageData>& subscriber)
// {
//     int next_id = findNewFreeId<MessageData>();
//     std::get<SubscribersT<MessageData>>(m_subscribers)[next_id] = subscriber;

//     return next_id;
// }

// template <class...RegisteredTypes>
// template <class MessageDataT>
// inline void PostOffice<RegisteredTypes...>::unsubscribe(int id)
// {
//     std::get<SubscribersT<MessageDataT>>(m_subscribers).erase(id);
// }

// template <class...RegisteredTypes>
// template <class MessageDataT>
//  inline int PostOffice<RegisteredTypes...>::findNewFreeId()
// {
//     auto &subs = std::get<SubscribersT<MessageDataT>>(m_subscribers);
//     while (subs.count(next_id) != 0)
//     {
//         next_id++;
//     }
//     return next_id;
// }

// template <class...RegisteredTypes>
// template <class MessageDataT>
// void PostOffice<RegisteredTypes...>::registerEvent()
// {

// }

#pragma once

inline void PostOffice::distributeMessages()
{
    for (auto &[type_id, holder] : m_holders)
    {
        holder->distribute();
    }
}


template <class MessageDataT>
inline void PostOffice::send(MessageDataT message)
{
    getHolder<MessageDataT>().send(message);
}

template <class MessageDataT>
inline int PostOffice::subscribeTo(Callback<MessageDataT> &subscriber)
{
    assert(isRegistered<MessageDataT>());
    auto &holder = getHolder<MessageDataT>();
    return holder.subscribe(subscriber);
}

template <class MessageDataT>
bool PostOffice::isRegistered() const
{
    return m_holders.count(std::type_index(typeid(MessageDataT))) > 0;
}

template <class MessageDataT>
MessageHolder<MessageDataT> &PostOffice::getHolder()
{
    assert(isRegistered<MessageDataT>());
    return static_cast<MessageHolder<MessageDataT> &>(*m_holders.at(std::type_index(typeid(MessageDataT))));
}
template <class MessageDataT>
inline void PostOffice::unsubscribe(int id)
{
    getHolder<MessageDataT>().unsubscribe(id);
}


template <class MessageDataT>
void PostOffice::registerEvent()
{
    m_holders[std::type_index(typeid(MessageDataT))] = std::make_unique<MessageHolder<MessageDataT>>();
}

template <class...MessageDataT>
void PostOffice::registerEvents()
{
    (registerEvent<MessageDataT>(), ...);
}