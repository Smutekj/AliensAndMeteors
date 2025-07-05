#include "Meteor.h"

#include "../GameWorld.h"

#include "../DrawLayer.h"
#include "../Utils/RandomTools.h"

Meteor::Meteor(GameWorld *world, TextureHolder &textures, Collisions::CollisionSystem *collider,  PlayerEntity *player)
 : GameObject(world, textures, ObjectType::Meteor)
{
    m_collision_shape = std::make_unique<Polygon>();
    m_rigid_body = std::make_unique<RigidBody>();
    initializeRandomMeteor();
}

void Meteor::update(float dt)
{
    truncate(m_vel, max_vel);
    m_pos += m_vel * dt;
}
void Meteor::onCreation()
{
}
void Meteor::onDestruction()
{
    const auto& new_meteor = m_world->addObject2<Meteor>();
    // new_meteor.setPosition(randomPosInBox({0,0}, {Geometry::Box[0], Geometry::Box[1]}));
}

void Meteor::onCollisionWith(GameObject &obj, CollisionData &c_data)
{
}

void Meteor::draw(LayersHolder& layers)
{

    auto points = m_collision_shape->getPointsInWorld();
    auto& target = layers.getCanvas("Unit");
    if(!target.hasShader("Meteor"))
    {
        target.addShader("Meteor", "basictex.vert", "Meteor.frag");
    }
    VertexArray m_verts;
    m_verts.resize(3*points.size());
    
    Color c = {1,0,0,1};
    auto center = m_collision_shape->getCenter();
    auto n_points = points.size();
    for(int i = 0; i < n_points; ++i)
    {
        auto tex_coord = ((points[i] - center)/m_collision_shape->getScale().x + 1.)/2.;
        auto tex_coord_next = ((points[(i+1)%n_points] - center)/m_collision_shape->getScale().x + 1.)/2.;

        m_verts[3*i+0] = {points[i], c, tex_coord};
        m_verts[3*i+1] = {points[(i+1)%n_points], c, tex_coord_next};
        m_verts[3*i+2] = {center, c, {0.5,0.5}};
    }
    
    target.drawVertices(m_verts, "Meteor", DrawType::Dynamic, m_textures->get("Meteor"));
}

Meteor::~Meteor() {}

//! Not my code! Taken from here: https://cglab.ca/~sander/misc/ConvexGeneration/convex.html
Polygon Meteor::generateRandomConvexPolygon(int n) const
{

    // Generate two lists of random X and Y coordinates
    std::vector<float> xPool(0);
    std::vector<float> yPool(0);

    for (int i = 0; i < n; i++)
    {
        xPool.push_back(randf(-1, 1));
        yPool.push_back(randf(-1, 1));
    }

    // Sort them
    std::sort(xPool.begin(), xPool.end());
    std::sort(yPool.begin(), yPool.end());

    // Isolate the extreme points
    auto minX = xPool.at(0);
    auto maxX = xPool.at(n - 1);
    auto minY = yPool.at(0);
    auto maxY = yPool.at(n - 1);

    // Divide the interior points into two chains & Extract the vector components
    std::vector<float> xVec(0);
    std::vector<float> yVec(0);

    float lastTop = minX, lastBot = minX;

    for (int i = 1; i < n - 1; i++)
    {
        auto x = xPool.at(i);

        if (rand() % 2)
        {
            xVec.push_back(x - lastTop);
            lastTop = x;
        }
        else
        {
            xVec.push_back(lastBot - x);
            lastBot = x;
        }
    }

    xVec.push_back(maxX - lastTop);
    xVec.push_back(lastBot - maxX);

    float lastLeft = minY, lastRight = minY;

    for (int i = 1; i < n - 1; i++)
    {
        auto y = yPool.at(i);

        if (rand() % 2)
        {
            yVec.push_back(y - lastLeft);
            lastLeft = y;
        }
        else
        {
            yVec.push_back(lastRight - y);
            lastRight = y;
        }
    }

    yVec.push_back(maxY - lastLeft);
    yVec.push_back(lastRight - maxY);

    std::random_device rd;
    std::mt19937 g(rd());

    // Randomly pair up the X- and Y-components
    std::shuffle(yVec.begin(), yVec.end(), g);

    // Combine the paired up components into vectors
    std::vector<utils::Vector2f> vec;

    for (int i = 0; i < n; i++)
    {
        vec.emplace_back(xVec.at(i), yVec.at(i));
    }

    // Sort the vectors by angle
    std::sort(vec.begin(), vec.end(), [](const auto &p1, const auto &p2)
              { return std::atan2(p1.y, p1.x) < std::atan2(p2.y, p2.x); });

    // Lay them end-to-end
    float x = 0, y = 0;
    float minPolygonX = 0;
    float minPolygonY = 0;
    std::vector<utils::Vector2f> points;

    for (int i = 0; i < n; i++)
    {
        points.push_back({x, y});

        x += vec.at(i).x;
        y += vec.at(i).y;

        minPolygonX = std::min(minPolygonX, x);
        minPolygonY = std::min(minPolygonY, y);
    }

    // Move the polygon to the original min and max coordinates
    auto xShift = minX - minPolygonX;
    auto yShift = minY - minPolygonY;

    for (int i = 0; i < n; i++)
    {
        auto p = points.at(i);
        points.at(i) += utils::Vector2f{xShift, yShift};
    }
    Polygon p;
    p.points = points;

    return p;
}

void Meteor::initializeRandomMeteor()
{
    auto polygon = generateRandomConvexPolygon(12 + rand() % 3);
    auto radius = randf(5, 20);
    polygon.setScale(radius, radius);
    auto rand_pos = randomPosInBox(utils::Vector2f{0,0}, utils::Vector2f{500, 500});
    polygon.setPosition(rand_pos.x, rand_pos.y);

    m_pos = polygon.getPosition();
    m_angle = polygon.getRotation();

    m_vel = {randf(-6, 6), randf(-6, 6)};

    m_rigid_body->angle_vel = randf(-0.02, 0.02);
    m_rigid_body->mass = radius * radius;
    m_rigid_body->inertia = radius * radius * m_rigid_body->mass;

    m_collision_shape = std::make_unique<Polygon>(polygon);
}