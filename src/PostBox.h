#pragma once

#include "Vector2.h"

class PostOffice;

class PostBoxI
{
public:
    PostBoxI() {}
    virtual ~PostBoxI(){};

protected:
    int id = -1;
};

template <class MessageData>
class PostBox : public PostBoxI
{
public:
    PostBox(PostOffice &post_office,
            std::function<void(const std::deque<MessageData> &)> on_receival);

    virtual ~PostBox() override;

private:
    friend class PostOffice;

    PostOffice *p_post_office;
    std::function<void(const std::deque<MessageData> &)> on_receival;
};

#include "PostBox.inl"