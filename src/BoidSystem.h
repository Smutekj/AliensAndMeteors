#pragma once

#include "Utils/Grid.h"
#include "Utils/GayVector.h"
#include "Utils/RandomTools.h"

#include "Geometry.h"
#include "Polygon.h"
#include "Player.h"
#include "BulletSystem.h"
#include "BehaviourBase.h"

#include <queue>
#include <set>

#include "Messanger.h"

struct Boid
{
    sf::Vector2f r;
    sf::Vector2f vel;
    sf::Vector2f acc;

    MoveState state = MoveState::STANDING;

    float radius = 1.f;
    float orientation = 0.f;
    sf::Vector2f target_pos;
    int group_ind = -1;
    int entity_ind = -1;
};

struct Task
{
    EntityData *data;
    virtual bool isFinished();
};

struct MoveToPointTask : Task
{
    float radius = 3.f;
    sf::Vector2f point;
    MoveToPointTask(sf::Vector2f move_target, float radius)
        : point(move_target), radius(radius) {}
    bool isFinished()
    {
        return dist2(data->r, point) < radius * radius;
    }
};

struct ShootTask : Task
{

    int cool_down = 60;
    int frames_since_shot = 0;
    bool isFinished()
    {
        frames_since_shot++;
        return frames_since_shot >= cool_down;
    }
};

struct GridInds
{
    int grid_ind = -1;
    int ind_in_grid = -1;
};

constexpr int N_GRID_X = 20;
constexpr int N_GRID_Y = 20;

struct NeighbourData
{

    sf::Vector2f dr;
    sf::Vector2f dv;

    int first;
    int second;
};

class Player;
class PolygonObstacleManager;
class BoidAI;
class GroupManager;

class BoidSystem
{

    std::vector<Boid> boids;
    std::vector<GridInds> boid2grid;
    std::vector<std::vector<int>> boid2neighbour_inds;

    std::unique_ptr<SearchGrid> p_grid;

    std::array<std::vector<int>, N_GRID_X * N_GRID_Y> grid2boid_inds;

    const float max_vel = 25.f;

    

public:
    std::queue<int> to_destroy;
    PolygonObstacleManager *polygons;
    Player *player;
    BulletSystem *p_bs;

    std::set<int> free_entities;
    std::array<int, 5000> entity2boids;

    std::array<EntityData, 5000> entity2boid_data;

    std::array<std::unique_ptr<BoidAI>, 5000> entity2p_ai;
    std::array<std::vector<std::unique_ptr<BoidAI>>, 5000> entity2p_ais;

    std::shared_ptr<GroupManager> groups;

    BoidSystem(float max_dist = 40.f);

    enum class Multiplier
    {
        SCATTER,
        ALIGN,
        SEEK,
        VELOCITY,
        AVOID
    };
    AcosTable<1000> angle_calculator;
    std::unordered_map<Multiplier, float> force_multipliers;
    std::unordered_map<Multiplier, float> force_ranges;

    void addBoid(sf::Vector2f at);

    void update(float dt);

    void applyForces(int boid_ind_i);

    void removeFromGrid(int boid_ind);

    void insertToGrid(int grid_ind, int boid_ind);

    void removeBoid(int boid_ind);
    void removeBoids(const std::vector<int> &boid_ind);

    void addBoid(sf::Vector2f at, std::unique_ptr<BoidAI> &&ai, int group_ind = -1);

    void setBehaviourOf(int entity_ind, std::unique_ptr<BoidAI> behaviour);

    void addGroupOfBoids(int n_boids, sf::Vector2f center, float radius);

    std::vector<int> getBoidsIn(const sf::FloatRect &rect)
    {
        std::vector<int> selection;
        int boid_ind = 0;
        for (auto &boid : boids)
        {
            if (rect.contains(boid.r))
            {
                selection.push_back(boid.entity_ind);
            }
            boid_ind++;
        }
        return selection;
    }

    bool collidesWith(sf::Vector2f r, const Polygon &polygon)
    {
        const auto &points = polygon.getPointsInWorld();
        int next = 1;
        float min_dist = std::numeric_limits<float>::max();
        sf::Vector2f min_axis;
        for (int curr = 0; curr < points.size(); ++curr)
        {

            auto t1 = points[next] - points[curr]; //! line perpendicular to current polygon edge
            sf::Vector2f n1 = {t1.y, -t1.x};
            n1 /= norm(n1);
            // auto proj1 = projectOnAxis(n1, points);
            auto proj_point = dot(r - points[curr], n1);
            if (proj_point > 0)
            {
                return false; //! no collision
            }

            next++;
            if (next == points.size())
            {
                next = 0;
            }
        }
        return true;
    }

    void avoidMeteors();

    void setTargetOf(std::vector<int> inds, sf::Vector2f new_target)
    {
        for (auto entity_ind : inds)
        {
            auto ind = entity2boids.at(entity_ind);
            if(ind == -1){continue;}
            boids.at(ind).target_pos = new_target;
            auto dr_to_target = boids.at(ind).target_pos - boids.at(ind).r;
            // boids.at(ind).vel = 0.01f*dr_to_target/norm(dr_to_target);
            boids.at(ind).state = MoveState::MOVING;
        }
    }

    std::vector<Boid> &getBoids()
    {
        return boids;
    }

    // std::variant<DestroyEnemy, SpawnEnemyMessage>;

    //     void acceptMessages(std::queue<std::unique_ptr<BoidMessage>>& messages){
    //          basicPackA;
    // std::variant<LightItem, HeavyItem> basicPackB;

    // std::visit(overload{
    //     [](LightItem&, LightItem& ) { cout << "2 light items\n"; },
    //     [](LightItem&, HeavyItem& ) { cout << "light & heavy items\n"; },
    //     [](HeavyItem&, LightItem& ) { cout << "heavy & light items\n"; },
    //     [](HeavyItem&, HeavyItem& ) { cout << "2 heavy items\n"; },
    // }, basicPackA, basicPackB);

    //     }

private:
    bool mappingsAreOK()
    {
        for (int i = 0; i < boids.size(); ++i)
        {
            auto gi = boid2grid.at(i).grid_ind;
            auto iig = boid2grid.at(i).ind_in_grid;
            if (grid2boid_inds.at(gi).at(iig) != i)
            {
                return false;
            }
            if (entity2boids.at(boids.at(i).entity_ind) != i)
            {
                return false;
            }
        }

        return true;
    }
};
