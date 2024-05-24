#pragma once

#include <queue>
#include <set>
#include <unordered_map>

#include "Utils/Grid.h"
#include "Utils/GayVector.h"
#include "Utils/RandomTools.h"

#include "Geometry.h"
#include "Polygon.h"
#include "Player.h"
#include "BulletSystem.h"
#include "BehaviourBase.h"


struct Boid
{
    sf::Vector2f r;
    sf::Vector2f vel;
    sf::Vector2f acc;

    MoveState state = MoveState::STANDING;

    float radius = 2.f;
    float orientation = 0.f;
    sf::Vector2f target_pos;
    int group_ind = -1;
    int entity_ind = -1;

    enum class EnemyType
    {
        BOMBER,
        BASIC,
        LASERBOI
    };

    EnemyType type = EnemyType::BASIC;
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

    ObjectPool<Boid> m_boids;

    std::unique_ptr<SearchGrid> p_grid;
    std::vector<std::vector<int>> grid2boid_inds;

    const float max_vel = 50.f;
    const float max_acc = 0.5f;
    const float max_impulse_vel = 40.f;

    TextureHolder textures;
    std::unordered_map<Boid::EnemyType, Textures::ID> enemy_type2texture;

public:
    std::queue<int> to_destroy;
    PolygonObstacleManager *polygons;
    Player *player;
    BulletSystem *p_bs;

    std::set<int> free_entities;
    std::array<int, 5000> entity2boids;
    std::array<sf::Vector2f, 5000> entity2impulses;

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

    void setBoidVel(int entity_ind, const sf::Vector2f &new_vel)
    {
        boids.at(entity2boids.at(entity_ind)).vel = new_vel;
    }

    void addBoidImpulse(int entity_ind, const sf::Vector2f &impulse)
    {
        entity2impulses.at(entity_ind) += impulse;
    }

    void removeBoid(int boid_ind);
    void removeBoids(const std::vector<int> &boid_ind);
    void addBoid(sf::Vector2f at, std::unique_ptr<BoidAI> &&ai,
                 int group_ind = -1, Boid::EnemyType type = Boid::EnemyType::BASIC);
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

    std::vector<int> getBoidsIn(AABB &rect)
    {
        std::vector<int> selection;
        int boid_ind = 0;

        int min_x = rect.r_min.x / p_grid->cell_size_.x;
        int max_x = rect.r_max.x / p_grid->cell_size_.x;
        int min_y = rect.r_min.y / p_grid->cell_size_.y;
        int max_y = rect.r_max.y / p_grid->cell_size_.y;

        auto n_total_cells = p_grid->getNCells();
        for (int ix = min_x; ix <= max_x; ++ix)
        {
            for (int iy = min_y; iy <= max_y; ++iy)
            {
                auto grid_ind = iy * p_grid->n_cells_.x + ix;
                if (grid_ind < 0 || grid_ind >= n_total_cells)
                {
                    continue;
                }
                for (auto boid_ind : grid2boid_inds.at(grid_ind))
                {
                    selection.push_back(boids.at(boid_ind).entity_ind);
                }
            }
        }
        assert(gridIsOk());
        return selection;
    }

    bool gridIsOk() const
    {
        std::set<int> set;
        auto n_total_cells = p_grid->getNCells();
        for (int grid_ind = 0; grid_ind < n_total_cells; ++grid_ind)
        {
            for (auto boid_ind : grid2boid_inds.at(grid_ind))
            {
                if (set.count(boid_ind) > 0)
                {
                    return false;
                }
                set.insert(boid_ind);
            }
        }
        return true;
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
            if (ind == -1)
            {
                continue;
            }
            boids.at(ind).target_pos = new_target;
            auto dr_to_target = boids.at(ind).target_pos - boids.at(ind).r;
            // boids.at(ind).vel = 0.01f*dr_to_target/norm(dr_to_target);
            boids.at(ind).state = MoveState::MOVING;
        }
    }

    void spawnBoss(sf::Vector2f pos);

    std::vector<Boid> &getBoids()
    {
        return boids;
    }

    void draw(sf::RenderTarget &target)
    {
        sf::VertexArray boid_vertices;
        boid_vertices.setPrimitiveType(sf::Quads);
        boid_vertices.resize(4 * boids.size());
        // sf::Color color = sf::Color::Green;

        for (int boid_ind = 0; boid_ind < boids.size(); ++boid_ind)
        {
            const auto &boid = boids[boid_ind];

            sf::Transform a;
            a.rotate(boid.orientation);
            a.scale({boid.radius, boid.radius});

            auto texture_type = enemy_type2texture.at(boid.type);
            float ship_size_x = textures.get(texture_type).getSize().x;
            float ship_size_y = textures.get(texture_type).getSize().y;
            boid_vertices.resize(4);
            boid_vertices[0 * boid_ind * 4 + 0] = {boid.r + a.transformPoint({-0.5, -0.5}), {0, 0}};
            boid_vertices[0 * boid_ind * 4 + 1] = {boid.r + a.transformPoint({0.5, -0.5}), {ship_size_x, 0}};
            boid_vertices[0 * boid_ind * 4 + 2] = {boid.r + a.transformPoint({0.5, 0.5}), {ship_size_x, ship_size_y}};
            boid_vertices[0 * boid_ind * 4 + 3] = {boid.r + a.transformPoint({-0.5, 0.5}), {0, ship_size_y}};

            target.draw(boid_vertices, &textures.get(texture_type));
            //! will fix once I make a texture atlas
        }
    }

    void spawn(Boid::EnemyType type, sf::Vector2f pos);

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

    void fillNeighbourList();
};
