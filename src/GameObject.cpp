#include "GameObject.h"

#include "Polygon.h"

GameObject::GameObject(GameWorld *world, TextureHolder &textures, ObjectType type, PlayerEntity *player)
    : m_world(world), m_textures(&textures), m_type(type)
{
}

void GameObject::updateAll(float dt)
{

    if(m_parent)
    {
        m_pos = m_parent->getPosition();
        m_angle = m_parent->getAngle(); 
        m_vel = m_parent->m_vel; 
    }

    update(dt);
}

bool GameObject::isRoot() const{
    return m_parent == nullptr;
}

void GameObject::move(utils::Vector2f by)
{
    m_pos += by;
    if (m_collision_shape)
    {
        m_collision_shape->setPosition(m_pos);
    }
}

bool GameObject::collides() const
{
    return m_collision_shape != nullptr;
}

const utils::Vector2f &GameObject::getPosition() const
{
    if(m_parent)
    {
        // return m_pos + m_parent->m_pos;
    }
    return m_pos;
}

float GameObject::getAngle() const
{
    return m_angle;
}

Polygon &GameObject::getCollisionShape()
{
    return *m_collision_shape;
}

bool GameObject::doesPhysics() const
{
    return m_rigid_body != nullptr;
}

RigidBody &GameObject::getRigidBody()
{
    return *m_rigid_body;
}


int GameObject::getId() const
{
    return m_id;
}

ObjectType GameObject::getType() const
{
    return m_type;
}

void GameObject::removeCollider()
{
    if (m_collision_shape)
    {
        m_collision_shape = nullptr;
    }
}

void GameObject::setDestructionCallback(std::function<void(int, ObjectType)> callback)
{
    m_on_destruction_callback = callback;
}

void GameObject::addChild(GameObject* child)
{
    m_children.push_back(child);
    child->m_parent = this;
}

void GameObject::removeChild(GameObject* child)
{
    m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
}

bool GameObject::isParentOf(GameObject* child) const
{
    //! walk through queried object parents, if we find ourselves we are a parent of the child
    GameObject* curr = child->m_parent;
    while(curr)
    {
        if(curr == this) 
        {
            return true;
        }
        curr = curr->m_parent;
    }
    return false;
}

void GameObject::kill()
{
    m_is_dead = true;
}

bool GameObject::isDead() const
{
    return m_is_dead;
}

void GameObject::setPosition(utils::Vector2f new_position)
{
    m_pos = new_position;
    if (m_collision_shape)
    {
        m_collision_shape->setPosition(new_position);
    }
}

void GameObject::setSize(utils::Vector2f size)
{
    if (m_collision_shape)
    {
        m_collision_shape->setScale(size);
    }
    m_size = size;
}

const utils::Vector2f& GameObject::getSize() const
{
    return m_size;
}

void GameObject::setAngle(float angle)
{
    m_angle = angle;
    if (m_collision_shape)
    {
        m_collision_shape->setRotation(angle);
    }
}