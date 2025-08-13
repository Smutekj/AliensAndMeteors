#pragma once

#include "System.h"
#include "../Entities/Player.h"
#include "../PostBox.h"
#include "../Entities/Factories.h"


class GameWorld;

class AISystem : public SystemI
{
public:
    AISystem(GameWorld &world,
             ContiguousColony<ShootPlayerAIComponent, int> &comps,
             ContiguousColony<LaserAIComponent, int>& l_comps);

    virtual void preUpdate(float dt, EntityRegistryT&) override;
    virtual void update(float dt) override;
    virtual void postUpdate(float dt, EntityRegistryT&) override{}

private:
    void initializeShooterAI();
    void initializeLaserShooterAI();

    void updateShooterAI(ShootPlayerAIComponent &comp, int id);
    void updateLaserAI(LaserAIComponent &comp, int id);

    void changeState(ShootPlayerAIComponent &comp, int id, ShooterAIState target_state);
    void changeState(LaserAIComponent &comp, int id, ShooterAIState target_state);

private:
    ProjectileFactory m_bullet_factory;
    LaserFactory m_laser_factory;

    std::unique_ptr<PostBox<DamageReceivedEvent>> m_dmg_postbox;

    std::unordered_map<ShooterAIState, std::function<void(ShootPlayerAIComponent &, int)>> m_change_state_callbacks;
    std::unordered_map<ShooterAIState, std::function<void(LaserAIComponent &, int)>> m_change_state_callbacks_laser;
    ContiguousColony<ShootPlayerAIComponent, int> &m_shooters;
    ContiguousColony<LaserAIComponent, int> &m_laser_shooters;

    GameWorld &m_world;
    PlayerEntity *p_player;
};
