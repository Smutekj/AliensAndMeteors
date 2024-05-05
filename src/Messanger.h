#pragma once

#include <queue>
#include <memory>
#include <unordered_map>
#include <variant>

#include "core.h"

struct Message
{

protected:
    Message(int system_id) : system_id(system_id) {}

public:
    int getId() const
    {
        return system_id;
    }

private:
    int system_id;

    // virtual void
};

struct BulletMessage : Message
{

    BulletMessage() : Message(1) {}

    enum class Type
    {
        SHOOT_BULLET,
        REMOVE_BULLET,
        CHANGE_BULLET
    };

    int bullet_type_ind = 0;
    sf::Vector2f target = {-1, -1};
    sf::Vector2f direction = {0, 0};
};

struct SpawnEnemyMessage : Message
{

    SpawnEnemyMessage() : Message(0) {}

    int enemy_type_id = 0;
    sf::Vector2f pos = {-1, -1};
    sf::Vector2f vel = {0, 0};
};

struct DestroyEnemyMessage : public Message{

    DestroyEnemyMessage() : Message(0) {}

    int entity_ind = -1;
    int destruction_type_id = 0;
};


enum class Behaviours{
    
};

struct ChangeEnemyBehaviour : public Message{

    ChangeEnemyBehaviour() : Message(1) {}


    std::vector<int> entity_inds;
    std::vector<Behaviours> new_ais;
};

// using  std::variant<SpawnEnemyMessage, BulletMessage>  MessageType; 

// std::unordered_map<int, std::queue<std::unique_ptr<MessageType>>> to_be_sent;

enum class SystemType : int{
    BOIDS = 0,
    PHYSICS = 1,
    BULLETS =2,
    N_SYSTEMS
};

template <int ID = -1>
class Messenger 
{
    // std::unordered_map<SystemType,  >

    void insertMessage(std::unique_ptr<Message>& message){
        // to_be_sent.at(message->getId()).push(std::move(message));
    }

};


class BoidsMessenger : public Messenger<0>{
    


};