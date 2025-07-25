
#pragma once

#include "PostOffice.h"

template <class MessageData>
PostBox<MessageData>::PostBox(PostOffice &post_office,
                              std::function<void(const std::deque<MessageData> &)> on_receival)
    : p_post_office(&post_office), on_receival(on_receival)
{
    id = p_post_office->subscribeTo<MessageData>(this);
}

template <class MessageData>
PostBox<MessageData>::~PostBox()
{
    p_post_office->unsubscribe<MessageData>(id);
}