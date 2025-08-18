#include "GameWorld.h"

#include <chrono>

#include "Utils/RandomTools.h"

GameWorld::GameWorld(PostOffice &messenger, TextureHolder& textures)
    : p_messenger(&messenger), m_systems(m_entities), m_textures(textures), m_collision_system(messenger, m_systems.getComponents<CollisionComponent>())
{
    m_effect_factories[EffectType::ParticleEmiter] =
        [this]()
    { return std::make_shared<StarEmitter>(this, m_textures); };
    m_effect_factories[EffectType::AnimatedSprite] =
        [this]()
    { return std::make_shared<AnimatedSprite>(this, m_textures); };
}

std::size_t GameWorld::getNActiveEntities(ObjectType type)
{
    auto &entities = m_entities.data();
    auto n_enemies = std::count_if(entities.begin(), entities.end(), [type](auto &obj)
                                   { return obj->getType() == type; });
    return n_enemies;
}

void GameWorld::addQueuedEntities2()
{
    std::apply([this](auto &...entity_queue)
               { ((addX(entity_queue)), ...); }, m_entities_to_add);
}
void GameWorld::removeQueuedEntities2()
{
    std::apply([this](auto &...entity_queue)
               { ((removeX(entity_queue)), ...); }, m_entities_to_remove);
}
template <class EntityType>
void GameWorld::updateX(ComponentBlock<EntityType> &entity_block, float dt)
{
    static_assert(std::is_base_of_v<GameObject, EntityType>);

    auto &block = entity_block.getData();
    int first_ind = block.at(0).next;
    for (int ind = first_ind; ind <= 750; ind += block.at(ind).next) // block[ind].next)
    {
        auto &entity = block.at(ind).comp;
        entity.updateAll(dt);
        entity.update(dt);
        if (block.at(ind).comp.isDead())
        {
            std::get<std::queue<EntityType>>(m_entities_to_remove).push(entity);
        }
    }
}

void GameWorld::update2(float dt)
{
    // m_collision_system.update();

    std::apply([this, dt](auto &...entity_queue)
               { ((updateX(entity_queue, dt)), ...); }, m_entities2);

    addQueuedEntities2();
    removeQueuedEntities2();
}
template <class EntityType>
void GameWorld::drawX(ComponentBlock<EntityType> &entity_block, LayersHolder &layers, View camera_view)
{
    camera_view.setSize(camera_view.getSize() * 1.2); //! draw also a little bit beyond the camera

    auto &block = entity_block.getData();
    int first_ind = block.at(0).next;
    for (int ind = first_ind; ind <= 750; ind += block[ind].next) // block[ind].next)
    {
        auto &drawable = block.at(ind).comp;

        Rectf bounding_rect = {drawable.getPosition().x, drawable.getPosition().y,
                               drawable.getSize().x, drawable.getSize().y};

        if (camera_view.intersects(bounding_rect))
        {
            drawable.draw(layers);
        }
    }
}
void GameWorld::draw2(LayersHolder &layers, View camera_view)
{
    std::apply([this, &layers, camera_view](auto &&...ents)
               { ((drawX(ents, layers, camera_view)), ...); }, m_entities2);
}

GameObject &GameWorld::addObject3(ObjectType type)
{
    auto entity_p = std::make_shared<GameObject>(this, m_textures, type);
    entity_p->m_id = m_entities.reserveIndexForInsertion();
    m_to_add.push_back(entity_p);
    return *entity_p;
}

GameObject &GameWorld::addObject(ObjectType type)
{
    std::shared_ptr<GameObject> new_object;

    switch (type)
    {
    case ObjectType::Trigger:
        new_object = std::make_shared<ReachPlace>(this, m_textures, m_player);
        break;
        // default:
        //     throw std::runtime_error("You forgot to add the new object here!");
    }

    m_to_add.push_back(new_object);
    return *m_to_add.back();
}

void GameWorld::addQueuedEntities()
{
    while (!m_to_add.empty())
    {
        auto new_object = m_to_add.front();
        auto new_id = new_object->getId(); //! the object already has id because we reserved it
        m_entities.insertAt(new_id, new_object);

        assert(new_id == m_entities.at(new_id)->getId());
        m_entities.at(new_id)->onCreation();
        if (m_systems.has<CollisionComponent>(new_id))
        {
            m_collision_system.insertObject(*m_entities.at(new_id));
        }

        if (new_object->isRoot())
        {
            m_root_entities.insertAt(new_id, new_id);
        }

        m_to_add.pop_front();
    }
}

void removeEntity(GameObject *entity,
                  GameSystems &systems,
                  EntityRegistryT &entities,
                  DynamicObjectPool2<int> &root_entities,
                  Collisions::CollisionSystem &collision_system)
{
    auto id = entity->getId();
    entity->onDestruction();

    //! destroy also the object's children and remove it from roots if necessary
    if (entity->isRoot())
    {
        root_entities.remove(id);
        //! children become roots
        for (auto p_child : entity->m_children)
        {
            p_child->m_parent = nullptr;
            auto child_id = p_child->getId();
            assert(!root_entities.contains(child_id));
            root_entities.insertAt(child_id, child_id);
        }
    }
    else
    { //! entity has a parent so it should be removed from it's children
        entity->m_parent->removeChild(entity);
    }

    if (systems.has<CollisionComponent>(id))
    {
        collision_system.removeObject(*entity);
    }
    systems.removeEntity(id);
    entities.remove(id);
}

void GameWorld::removeParent(GameObject &child)
{
    if (!child.m_parent)
    {
        return;
    }

    //! children become roots
    for (auto p_child : child.m_parent->m_children)
    {
        p_child->m_parent = nullptr;
        auto child_id = p_child->getId();
        assert(!m_root_entities.contains(child_id));
        m_root_entities.insertAt(child_id, child_id);
    }
}
void GameWorld::removeQueuedEntities()
{

    //! we destroy the objects from children to parents, this way it doesn't get fucked
    //! TODO: The entity-tree logic should not be done in GameWorld, Add it's own thing for that
    std::deque<GameObject *> to_destroy;
    std::unordered_set<int> already_destroyed; //! for
    while (!m_to_destroy.empty())
    {
        auto object = m_to_destroy.back();
        m_to_destroy.pop_back();

        assert(!already_destroyed.contains(object->getId()));
        already_destroyed.insert(object->getId());

        to_destroy.push_front(object.get());

        for (auto p_child : object->m_children)
        {
            assert(!already_destroyed.contains(p_child->getId()));
            already_destroyed.insert(p_child->getId());

            to_destroy.push_front(p_child);
        }
    }
    m_to_destroy.clear();

    for (auto object : to_destroy)
    {
        p_messenger->send(EntityDiedEvent{object->getType(), object->getId(), object->getPosition()});
        removeEntity(object, m_systems, m_entities, m_root_entities, m_collision_system);
    }
}

void GameWorld::destroyObject(int entity_id)
{
    m_to_destroy.push_back(m_entities.at(entity_id));
}

void GameWorld::update(float dt)
{

    m_systems.preUpdate(dt);
    m_collision_system.preUpdate(dt, m_entities);
    m_systems.update(dt);
    m_systems.postUpdate(dt);

    std::deque<GameObject *> to_update;
    for (auto id : m_root_entities.data())
    {
        to_update.push_back(m_entities.at(id).get());
    }
    //! we update the entities starting from parents ending with children
    while (!to_update.empty())
    {
        auto current = to_update.front();
        to_update.pop_front();
        current->updateAll(dt);
        // current->update(dt);
        if (current->isDead())
        {
            destroyObject(current->getId());
        }

        for (auto child : current->m_children)
        {
            to_update.push_back(child);
        }
    }

    addQueuedEntities();
    removeQueuedEntities();
}

void GameWorld::checkComponentsConsistency()
{
    auto &t_comps = m_systems.getComponents<TargetComponent>();
    auto &comps = t_comps.data;
    auto &ids = t_comps.data_ind2id;
    for (auto id : ids)
    {
        assert(m_entities.contains(id));
    }
}

void GameWorld::draw(LayersHolder &layers)
{
    for (auto &obj : m_entities.data())
    {
        obj->draw(layers);
    }

#ifdef DEBUG
    checkComponentsConsistency();
    // m_ts->draw(layers.getCanvas("Unit"));
    m_collision_system.draw(layers.getCanvas("Unit"));
#endif
}

VisualEffect &GameWorld::addVisualEffect(EffectType type)
{
    assert(m_effect_factories.count(type) > 0);

    auto new_effect = m_effect_factories.at(type)();
    new_effect->m_id = m_entities.reserveIndexForInsertion();
    m_to_add.push_back(new_effect);
    return *new_effect;
}
