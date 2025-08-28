#pragma once

#include "../GameObject.h"

class Meteor : public GameObject
{

public:
    Meteor() = default;
    Meteor(GameWorld *world, TextureHolder &textures, PlayerEntity *player = nullptr);
    Meteor(const Meteor &e) = default;
    Meteor &operator=(Meteor &e) = default;
    Meteor &operator=(Meteor &&e) = default;
    virtual ~Meteor() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

    public:
    utils::Vector2f m_impulse_vel = {0.f};
private:
    void initializeRandomMeteor();

    Polygon generateRandomConvexPolygon(int n) const;

    float max_dist_from_player = 1000;
    GameObject* p_player = nullptr;
    
    float m_max_impulse_vel = 100.f;
    float m_impulse_decay = 1.5f;
    
    float m_angle_vel = 0.;

    utils::Vector2f m_center_tex;
    utils::Vector2f m_center_offset;

    std::vector<Vertex> m_verts;

};
