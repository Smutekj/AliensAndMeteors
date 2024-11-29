#pragma once

#include "../GameObject.h"


class Meteor : public GameObject
{

public:
    explicit Meteor(GameWorld *world, TextureHolder &textures);
    virtual ~Meteor() override;

    virtual void update(float dt) override;
    virtual void onCreation() override;
    virtual void onDestruction() override;
    virtual void draw(LayersHolder &target) override;
    virtual void onCollisionWith(GameObject &obj, CollisionData &c_data) override;

private:
    void initializeRandomMeteor();

    Polygon generateRandomConvexPolygon(int n) const;

    const float max_vel = 40.f;
    const float max_impulse_vel = 10.f;
};
