#pragma once


inline void PostOffice::distributeMessages()
{
    std::apply([this](auto&...messages)
               { (distributeToSubscribers(messages),...); }, m_message_queues);
}


template <class MessageType>
inline void PostOffice::distributeToSubscribers(std::deque<MessageType> &messages)
{
    auto &subs = std::get<SubscribersT<MessageType>>(m_subscribers);
    for (auto &[id, post_box] : subs)
    {
        // if(post_box.expired())
        // {
            // subs.erase(id);
        // }else{
            post_box(messages);
        // }
    }
    messages.clear();
}



template <class MessageDataT>
inline void PostOffice::send(MessageDataT message)
{
    std::get<std::deque<MessageDataT>>(m_message_queues).push_back(message);
}

template <class MessageData>
inline int PostOffice::subscribeTo(PostOffice::Callback<MessageData>& subscriber)
{
    int next_id = findNewFreeId<MessageData>();
    std::get<SubscribersT<MessageData>>(m_subscribers)[next_id] = subscriber;

    return next_id;
}

template <class MessageDataT>
inline void PostOffice::unsubscribe(int id)
{
    std::get<SubscribersT<MessageDataT>>(m_subscribers).erase(id);
}


template <class MessageDataT>
 inline int PostOffice::findNewFreeId()
{
    auto &subs = std::get<SubscribersT<MessageDataT>>(m_subscribers);
    while (subs.count(next_id) != 0)
    {
        next_id++;
    }
    return next_id;
}