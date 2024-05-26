#pragma once

#include <queue>
#include <memory>
#include <unordered_map>
#include <variant>

#include "core.h"


enum class MessageType
{
    SpawnBullet,
    DestroyMeteor,
    SpawnEnemy,
};

struct Message
{
    int sender_id;
    
};

struct SpawnBulletMessage : public Message
{
    int owner;
    sf::Vector2f dir;
    float vel;
};

struct EntityMessage : public Message
{
    int receiver;
};

struct ChangeBehaviourMessage : public Message
{

};

class Messenger
{

    std::queue<std::unique_ptr<Message>> m_messages;




    void queueMessage(std::unique_ptr<Message>& message)
    {
        m_messages.push(std::move(message));
    }
};
