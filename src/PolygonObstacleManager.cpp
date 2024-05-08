#include "PolygonObstacleManager.h"

PolygonObstacleManager::PolygonObstacleManager(int n_meteors)
{
    for (int i = 0; i < n_meteors; ++i)
    {
        auto polygon = generateRandomConvexPolygon(12 + rand() % 3);
        auto radius = randf(5, 10);
        polygon.setScale({radius, radius});
        polygon.setPosition(randomPosInBox());
        polygon.vel = {randf(-6, 6), randf(-6, 6)};
        polygon.angle_vel = randf(-0.02, 0.02);
        polygon.mass = radius * radius;
        polygon.inertia = std::pow(polygon.radius, 2) * polygon.mass;

        addMeteor(polygon);
    }
}

std::vector<Polygon *> PolygonObstacleManager::getNearestMeteors(sf::Vector2f r, float radius)
{
    std::vector<Polygon *> nearest_meteors;
    auto entity_inds = collision_tree.findIntersectingLeaves({r - sf::Vector2f{radius, radius},
                                                              r + sf::Vector2f{radius, radius}});

    for (auto ind : entity_inds)
    {
        nearest_meteors.push_back(&meteors.at(ind));
    }
    return nearest_meteors;
}

std::vector<int> PolygonObstacleManager::getNearestMeteorInds(sf::Vector2f lower_left, sf::Vector2f upper_right)
{
    sf::Vector2f ll = lower_left;
    sf::Vector2f ur = upper_right;

    ll.x = std::min(lower_left.x, upper_right.x);
    ll.y = std::min(lower_left.y, upper_right.y);

    ur.x = std::max(lower_left.x, upper_right.x);
    ur.y = std::max(lower_left.y, upper_right.y);

    std::vector<Polygon *> nearest_meteors;
    auto entity_inds = collision_tree.findIntersectingLeaves({ll, ur});

    return entity_inds;
}

Polygon PolygonObstacleManager::createRandomMeteor()
{
    auto polygon = generateRandomConvexPolygon(12 + rand() % 3);
    auto radius = randf(5, 10);
    polygon.setScale({radius, radius});
    polygon.setPosition(randomPosInBox());
    polygon.vel = {randf(-6, 6), randf(-6, 6)};
    polygon.angle_vel = randf(-0.02, 0.02);
    polygon.mass = radius * radius;
    polygon.inertia = std::pow(polygon.radius, 2) * polygon.mass;
    return std::move(polygon);
}

void PolygonObstacleManager::addRandomMeteorAt(sf::Vector2f position)
{
    auto new_meteor = createRandomMeteor();
    new_meteor.setPosition(position);
    auto new_entity_ind = meteors.addObject(new_meteor);
    collision_tree.addRect(new_meteor.getBoundingRect(), new_entity_ind);

    sf::ConvexShape wtf(new_meteor.points.size());

    for (int i = 0; i < new_meteor.points.size(); ++i)
    {
        wtf.setPoint(i, new_meteor.points.at(i));
    }

    wtf.setPosition(new_meteor.getPosition());
    wtf.setRotation(new_meteor.getRotation());
    wtf.setScale(new_meteor.getScale());
    wtf.setFillColor(sf::Color(rand() % 256, rand() % 256, 0, 128));

    auto new_entity_ind2 = drawables.addObject(wtf);
    assert(new_entity_ind == new_entity_ind2);
}

void PolygonObstacleManager::addMeteor(Polygon &new_meteor)
{
    auto new_entity_ind = meteors.addObject(new_meteor);
    collision_tree.addRect(new_meteor.getBoundingRect(), new_entity_ind);

    sf::ConvexShape wtf(new_meteor.points.size());

    for (int i = 0; i < new_meteor.points.size(); ++i)
    {
        wtf.setPoint(i, new_meteor.points.at(i));
    }

    wtf.setPosition(new_meteor.getPosition());
    wtf.setRotation(new_meteor.getRotation());
    wtf.setScale(new_meteor.getScale());
    wtf.setFillColor(sf::Color(rand() % 256, 255, 0, 255));

    auto new_entity_ind2 = drawables.addObject(wtf);
    assert(new_entity_ind == new_entity_ind2);
}

void PolygonObstacleManager::destroyMeteor(int entity_ind)
{
    meteors.remove(entity_ind);
    drawables.remove(entity_ind);
    collision_tree.removeObject(entity_ind);
}

void PolygonObstacleManager::update(float dt)
{



    for (auto k : meteors.active_inds)
    {
        auto &meteor = meteors.at(k);
        meteor.update(dt);
        if (meteor.getPosition().x < 0)
        {
            meteor.vel.x = std::abs(meteor.vel.x);
        }
        if (meteor.getPosition().y < 0)
        {
            meteor.vel.y = std::abs(meteor.vel.y);
        }
        if (meteor.getPosition().x >= Geometry::BOX[0])
        {
            meteor.vel.x = -std::abs(meteor.vel.x);
        }
        if (meteor.getPosition().y >= Geometry::BOX[1])
        {
            meteor.vel.y = -std::abs(meteor.vel.y);
        }
        truncate(meteor.vel, 5.0f);

        drawables.at(k).setPosition(meteor.getPosition());
        drawables.at(k).setRotation(meteor.getRotation());
        drawables.at(k).setScale(meteor.getScale());
    }

    struct pair_hash
    {
        inline std::size_t operator()(const std::pair<int, int> &v) const
        {
            return v.first * 31 + v.second;
        }
    };
    std::unordered_set<std::pair<int, int>, pair_hash> collided;

    collision_tree.clear();

    for (auto i : meteors.active_inds)
    {
        collision_tree.addRect(meteors.at(i).getBoundingRect(), i);
    }

    auto tic = std::chrono::high_resolution_clock::now();
    for (auto i : meteors.active_inds)
    {

        auto nearest_inds = collision_tree.findIntersectingLeaves(meteors.at(i).getBoundingRect());
        for (int meteor_ind : nearest_inds)
        {
            if (i == meteor_ind)
            {
                continue;
            }
            std::pair<int, int> collision_inds = {std::min(i, meteor_ind), std::max(i, meteor_ind)};
            if (collided.count(collision_inds) == 0)
            {
                collidePolygons(meteors.at(meteor_ind), meteors.at(i));
                collided.insert(collision_inds);
            }
        }
    }
    auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - tic);
    std::cout << "collisions took: " << delta_time.count() << " ms"
              << "\n";
}

void PolygonObstacleManager::draw(sf::RenderTarget &window)
{
    auto tic = std::chrono::high_resolution_clock::now();
    for (auto i : meteors.active_inds)
    {
        window.draw(drawables.at(i));
    }
    auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - tic);
    std::cout << "drawing took: " << delta_time.count() << " ms"
              << "\n";
}