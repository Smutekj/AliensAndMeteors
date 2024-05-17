#include "BoidSystem.h"
#include "Game.h"
#include "PolygonObstacleManager.h"
#include "Behaviours.h"
#include "BVH.h"

#include <iostream>

BoidSystem::BoidSystem(float max_dist)
{
    textures.load(Textures::EnemyShip, "../Resources/EnemyShip.png");
    textures.load(Textures::EnemyBomber, "../Resources/EnemyBomber.png");

    enemy_type2texture[Boid::EnemyType::BASIC] = Textures::EnemyShip;
    enemy_type2texture[Boid::EnemyType::BOMBER] = Textures::EnemyBomber;
    enemy_type2texture[Boid::EnemyType::LASERBOI] = Textures::EnemyShip;
    // enemy_type2texture[Boids::EnemyType::BASIC] = Textures::EnemyShip;

    groups = std::make_shared<GroupManager>(entity2boid_data);

    sf::Vector2f box_size = {Geometry::BOX[0], Geometry::BOX[1]};
    const sf::Vector2i n_cells = {static_cast<int>(box_size.x / max_dist) + 1,
                                  static_cast<int>(box_size.y / max_dist) + 1};

    p_grid = std::make_unique<SearchGrid>(n_cells, sf::Vector2f{max_dist, max_dist});
    grid2boid_inds.resize(n_cells.x * n_cells.y);

    for (int i = 0; i < 5000; ++i)
    {
        free_entities.insert(i);
    }

    force_multipliers[BoidSystem::Multiplier::ALIGN] = 1.0f;
    force_multipliers[BoidSystem::Multiplier::SCATTER] = 1.0f;
    force_multipliers[BoidSystem::Multiplier::AVOID] = 1.0f;
    force_multipliers[BoidSystem::Multiplier::SEEK] = 5.0f;
    force_multipliers[BoidSystem::Multiplier::VELOCITY] = 2.0;

    for (auto &[multiplier_type, value] : force_multipliers)
    {
        force_ranges[multiplier_type] = 10.f;
    }
}

void BoidSystem::avoidMeteors()
{
    auto avoid_multiplier = force_multipliers[Multiplier::AVOID];

    int compvec_ind = 0;
    for (int i = 0; i < boids.size(); ++i)
    {
        auto &boid = boids.at(i);
        auto r = boid.r;
        auto &v = boid.vel;
        auto state = entity2boid_data.at(boid.entity_ind).state;

        auto nearest_meteors = polygons->getNearestMeteors(boid.r, boid.radius * 5.0f);
        // auto wtf = polygons->collision_tree.rayCast(boid.r, boid.target_pos)
        sf::Vector2f avoid_force = {0,0};
        for (auto *meteor : nearest_meteors)
        {
            auto mvt = meteor->getMVTOfSphere(r, boid.radius);
            if (norm2(mvt) > 0.0001f)
            {
                entity2boid_data.at(boid.entity_ind).health -= 1;
                v -= 2.f * dot(mvt, v) * mvt;
                break;
            }
            auto r_meteor = meteor->getPosition();
            auto radius_meteor = meteor->radius;
            auto dr_to_target =  boid.target_pos - boid.r;
            dr_to_target/=norm(dr_to_target);
            auto dr_to_meteor = (r_meteor - r) / norm(r - r_meteor);
            auto dv_rel = v - meteor->vel;
            // sf::Vector2f dr_norm = {dr_to_target.y, -dr_to_target.x};
            sf::Vector2f dr_norm = {dr_to_meteor.y, -dr_to_meteor.x};
            dr_norm /= norm(dr_norm);

            auto dist_to_meteor = dist(r, r_meteor);
            if (dist_to_meteor < radius_meteor * 2.f)
            {
                const auto angle = angle_calculator.angleBetween(dr_to_meteor, dr_to_target);
                const auto angle_b = std::asin(radius_meteor / dist_to_meteor) * 180.f / M_PIf;
                const float sign = 2 * (angle < 0) - 1;

                if (std::abs(angle) < 110)
                {
                    avoid_force += sign * dr_norm * radius_meteor*radius_meteor/ (dist_to_meteor + radius_meteor);
                    avoid_force *= avoid_multiplier ;

                }
            }
        }
        truncate(avoid_force, 5.f);
        boid.acc += avoid_force;    
    }
}

void BoidSystem::applyForces(int boid_ind)
{

    const auto scatter_multiplier = force_multipliers[Multiplier::SCATTER];
    const auto align_multiplier = force_multipliers[Multiplier::ALIGN];
    const auto seek_multiplier = force_multipliers[Multiplier::SEEK];

    sf::Vector2f repulsion_force(0, 0);
    sf::Vector2f push_force(0, 0);
    sf::Vector2f scatter_force(0, 0);
    sf::Vector2f cohesion_force(0, 0);
    float n_neighbours = 0;
    float n_neighbours_group = 0;
    sf::Vector2f dr_nearest_neighbours(0, 0);
    sf::Vector2f average_neighbour_position(0, 0);

    sf::Vector2f align_direction = {0, 0};
    int align_neighbours_count = 0;

    auto range_align = std::pow(force_ranges[Multiplier::ALIGN], 2);
    auto range_scatter = std::pow(force_ranges[Multiplier::SCATTER], 2);

    auto &boid = boids.at(boid_ind);
    auto v = boid.vel;
    if (boid.state == MoveState::STANDING)
    {
    }

    for (auto ind_j : boid2neighbour_inds.at(boid_ind))
    {
        // if(ind_j == boid_ind){continue;}
        const auto &neighbour_boid = boids.at(ind_j);
        const auto dr = neighbour_boid.r - boid.r;
        const auto dist2 = norm2(dr);

        if (dist2 < range_align)
        {
            align_direction += neighbour_boid.vel;
            align_neighbours_count++;
        }

        if (dist2 < range_scatter)
        {
            scatter_force -= scatter_multiplier * dr / dist2;
            dr_nearest_neighbours += dr / dist2;
            n_neighbours++;
        }
        if (dist2 < range_scatter * 2.f && neighbour_boid.group_ind == boid.group_ind && boid.group_ind != -1)
        {
            average_neighbour_position += dr;
            n_neighbours_group++;
        }
    }

    dr_nearest_neighbours /= n_neighbours;

    if (n_neighbours > 0 && norm2(dr_nearest_neighbours) >= 0.00001f)
    {
        // scatter_force += -scatter_multiplier * dr_nearest_neighbours/norm(dr_nearest_neighbours) - v;
    }

    average_neighbour_position /= n_neighbours_group;
    if (n_neighbours_group > 0)
    {
        cohesion_force = 2.f * average_neighbour_position - v;
    }

    sf::Vector2f align_force = {0, 0};
    if (align_neighbours_count > 0 && norm2(align_direction) >= 0.001f)
    {
        align_force = align_multiplier * align_direction / norm(align_direction) - v;
    }

    auto dr_to_target = entity2boid_data.at(boid.entity_ind).target_position - boid.r;
    sf::Vector2f seek_force = {0, 0};
    if (norm(dr_to_target) > 3.f)
    {
        seek_force = seek_multiplier * max_vel * dr_to_target / norm(dr_to_target) - v;
    }

    boid.acc += (scatter_force + align_force + seek_force + 0.00001f * cohesion_force);
    truncate(boid.acc, max_acc);
}

void BoidSystem::update(float dt)
{
    std::chrono::high_resolution_clock clock;
    auto t_start = clock.now();

    // p_ns_->update(boids);
    for (int boid_ind = 0; boid_ind < boids.size(); ++boid_ind)
    {
        auto &boid = boids[boid_ind];
        auto entity_ind = boid.entity_ind;
        entity2boid_data.at(entity_ind).r = boid.r;
        entity2p_ai.at(entity_ind)->update();

        boid.target_pos = entity2boid_data.at(entity_ind).target_position;

        const auto old_grid_ind = boid2grid.at(boid_ind).grid_ind;
        const auto curr_grid_ind = p_grid->coordToCell(boid.r);
        if (curr_grid_ind != old_grid_ind)
        {
            removeFromGrid(boid_ind);
            insertToGrid(curr_grid_ind, boid_ind);
        }
    }

    auto delta_t = std::chrono::duration_cast<std::chrono::microseconds>(clock.now() - t_start);
    std::cout << "moving grid took: " << delta_t.count() << " us\n";
    t_start = clock.now();

    //! fill neighbour list
    fillNeighbourList();

    delta_t = std::chrono::duration_cast<std::chrono::microseconds>(clock.now() - t_start);
    std::cout << "finding_neighbours took: " << delta_t.count() << " us\n";

    for (int boid_ind = 0; boid_ind < boids.size(); ++boid_ind)
    {
        applyForces(boid_ind);
        boid2neighbour_inds.at(boid_ind).clear();
    }

    avoidMeteors();

    for (auto &boid : boids)
    {
        if (entity2boid_data.at(boid.entity_ind).health <= 0)
        {
            to_destroy.push(boid.entity_ind);
        }
        boid.vel += boid.acc + entity2boid_data.at(boid.entity_ind).acc;
        truncate(boid.vel, entity2boid_data.at(boid.entity_ind).max_vel);
        truncate(entity2impulses.at(boid.entity_ind), max_impulse_vel);

        if (norm2(boid.vel) > 0.00001f)
        {
            auto desired_orient = dir2angle(boid.vel);
            boid.orientation = dir2angle(boid.vel);
        }

        boid.vel += entity2impulses.at(boid.entity_ind);

        entity2impulses.at(boid.entity_ind) *= 0.f;
        boid.r += boid.vel * dt;
        entity2boid_data.at(boid.entity_ind).r = boid.r;
        entity2boid_data.at(boid.entity_ind).vel = boid.vel;

        entity2boid_data.at(boid.entity_ind).acc *= 0.f;
        boid.acc *= 0.f;
    }
}

void BoidSystem::addBoid(sf::Vector2f at)
{
    auto ent_ind = *free_entities.begin();
    auto new_boid_ind = boids.size();
    entity2boid_data.at(ent_ind) = {at, at};
    // auto ai = std::make_unique<FollowPlayerSomeDistance>(
    //             ent_ind,
    //             player,
    //              &entity2boid_data.at(ent_ind));
    auto ai = std::make_unique<FollowAndShootAI>(
        ent_ind,
        player,
        &entity2boid_data.at(ent_ind),
        p_bs);
    entity2p_ai.at(ent_ind) = std::move(ai);

    entity2boids.at(ent_ind) = new_boid_ind;
    free_entities.erase(ent_ind);

    Boid new_boid;
    new_boid.entity_ind = ent_ind;
    new_boid.r = at;
    new_boid.target_pos = at;
    new_boid.type = Boid::EnemyType::BASIC;
    boids.push_back(new_boid);

    boid2neighbour_inds.resize(boids.size());
    boid2grid.resize(boids.size());
    insertToGrid(p_grid->coordToCell(at), new_boid_ind);
}

void BoidSystem::addBoid(sf::Vector2f at, std::unique_ptr<BoidAI> &&ai,
                     int group_ind, Boid::EnemyType type)
{

    auto ent_ind = *free_entities.begin();
    if (group_ind != -1)
    {
        groups->addToGroup(ent_ind, group_ind);
    }
    auto new_boid_ind = boids.size();
    entity2boid_data.at(ent_ind) = {at, at};
    entity2boid_data.at(ent_ind).group_ind = group_ind;

    entity2p_ai.at(ent_ind) = std::move(ai);

    entity2boids.at(ent_ind) = new_boid_ind;
    free_entities.erase(ent_ind);

    Boid new_boid;
    new_boid.entity_ind = ent_ind;
    new_boid.r = at;
    new_boid.target_pos = at;
    new_boid.group_ind = group_ind;
    new_boid.type = type;
    boids.push_back(new_boid);

    boid2neighbour_inds.resize(boids.size());
    boid2grid.resize(boids.size());
    insertToGrid(p_grid->coordToCell(at), new_boid_ind);
}

void BoidSystem::removeFromGrid(int boid_ind)
{
    auto grid_ind = boid2grid.at(boid_ind).grid_ind;
    auto ind_in_grid = boid2grid.at(boid_ind).ind_in_grid;

    auto &grid_data = grid2boid_inds.at(grid_ind);

    auto moved_boid_ind = grid_data.back();
    grid_data.at(ind_in_grid) = moved_boid_ind;
    boid2grid.at(moved_boid_ind).ind_in_grid = ind_in_grid;
    grid_data.pop_back();

    grid_ind = 0;
    for (auto &gd : grid2boid_inds)
    {
        int ind_in_grid = 0;
        for (auto &boid : gd)
        {
            assert(boid2grid.at(boid).grid_ind == grid_ind);
            assert(boid2grid.at(boid).ind_in_grid == ind_in_grid);
            ind_in_grid++;
        }
        grid_ind++;
    }
}

void BoidSystem::insertToGrid(int grid_ind, int boid_ind)
{
    auto &grid_data = grid2boid_inds.at(grid_ind);

    boid2grid.at(boid_ind) = {grid_ind, static_cast<int>(grid_data.size())};

    grid_data.push_back(boid_ind);

    // assert(mappingsAreOK());
}

void BoidSystem::removeBoid(int entity_ind)
{
    const auto boid_ind = entity2boids.at(entity_ind);
    if (boid_ind == -1)
    {
        return;
    }
    entity2boids.at(boids.at(boid_ind).entity_ind) = -1;
    // entity2p_ai.at(boids.at(boid_ind).entity_ind).reset();

    removeFromGrid(boid_ind);

    auto moved_boid_grid_ind = boid2grid.back().grid_ind;

    if (boids.size() - 1 != boid_ind)
    {
        removeFromGrid(boids.size() - 1);
        insertToGrid(moved_boid_grid_ind, boid_ind);
    }

    boids.at(boid_ind) = boids.back();
    entity2boids.at(boids.back().entity_ind) = boid_ind;
    boids.pop_back();
    // boid2grid.at(boid_ind) = boid2grid.back(); //! I don't fucking know...
    boid2grid.pop_back();

    assert(mappingsAreOK());
}

void BoidSystem::removeBoids(const std::vector<int> &ent_inds)
{
    for (auto ind : ent_inds)
    {
        removeBoid(ind);
        assert(free_entities.count(ind) == 0);
        free_entities.insert(ind);
        // entity2boids.at(ind) = -1;
    }
}

void BoidSystem::setBehaviourOf(int entity_ind, std::unique_ptr<BoidAI> behaviour)
{
    behaviour->data = &(entity2boid_data.at(entity_ind));
    // if (entity2p_ai.at(entity_ind).get())
    // {
    //     entity2p_ai.at(entity_ind).reset();
    // }
    entity2p_ai.at(entity_ind) = std::move(behaviour);
}

void BoidSystem::addGroupOfBoids(int n_boids, sf::Vector2f center, float radius)
{

    int new_group_ind = groups->createGroup();
    auto ent_ind = *free_entities.begin();
    auto leader_ai = std::make_unique<LeaderAI>(
        ent_ind,
        player,
        &entity2boid_data.at(ent_ind),
        groups.get(),
        this,
        p_bs);

    for (int i = 1; i < n_boids; ++i)
    {
        ent_ind = *free_entities.begin();
        auto ai = std::make_unique<UnderlingAI>(
            ent_ind,
            player,
            &entity2boid_data.at(ent_ind),
            groups.get(),
            this,
            leader_ai.get());
        float rand_angle = randf(0, 2.f * M_PI);
        float rand_radius = randf(0, radius);
        sf::Vector2f new_position = center + angle2dir(rand_angle) * rand_radius;
        auto *wtf = &entity2boid_data.at(ent_ind);
        entity2boid_data.at(ent_ind).r = new_position;

        addBoid(new_position, std::move(ai), new_group_ind);
    }

    addBoid(center, std::move(leader_ai), new_group_ind);
    boids.back().radius *= 3.f;
}

void BoidSystem::spawnBoss(sf::Vector2f pos)
{
    auto new_entity_ind = *free_entities.begin();

    auto ai = std::make_unique<BomberAI>(new_entity_ind, player,
                                       &entity2boid_data.at(new_entity_ind), p_bs);

    addBoid(pos, std::move(ai), -1, Boid::EnemyType::BOMBER);
}

void BoidSystem::spawn(Boid::EnemyType type, sf::Vector2f pos )
{
    auto new_entity_ind = *free_entities.begin();


    std::unique_ptr<BoidAI> ai;
    switch(type)
    {
        case Boid::EnemyType::BASIC:
            ai = std::make_unique<FollowAndShootAI>(new_entity_ind, player,
                                       &entity2boid_data.at(new_entity_ind), p_bs);
            break;
        case Boid::EnemyType::LASERBOI:
            ai = std::make_unique<BomberAI>(new_entity_ind, player,
                                       &entity2boid_data.at(new_entity_ind), p_bs);
            break;
        case Boid::EnemyType::BOMBER:
            ai = std::make_unique<BossAI>(new_entity_ind, player,
                                       &entity2boid_data.at(new_entity_ind), p_bs);
            break;
        default:
            throw std::runtime_error("Type not supported");
    }   
    
    addBoid(pos, std::move(ai), -1, type);
}

void BoidSystem::fillNeighbourList()
{
    for (int boid_ind = 0; boid_ind < boids.size(); ++boid_ind)
    {
        auto grid_ind = boid2grid.at(boid_ind).grid_ind;
        const auto &boid = boids.at(boid_ind);

        std::array<int, 9> nearest_cells;
        int n_nearest_cells;
        p_grid->calcNearestCells(grid_ind, nearest_cells, n_nearest_cells);
        nearest_cells[n_nearest_cells] = grid_ind;
        n_nearest_cells++;

        for (int i = 0; i < n_nearest_cells; ++i)
        {
            const auto &neighbour_inds = grid2boid_inds.at(nearest_cells.at(i));
            for (auto neighbour_ind : neighbour_inds)
            {
                const auto &neighbour_boid = boids.at(neighbour_ind);
                if (dist2(boid.r, neighbour_boid.r) < 20.f * 20.f && neighbour_ind != boid_ind)
                {
                    boid2neighbour_inds.at(boid_ind).push_back(neighbour_ind);
                }
            }
        }
    }
}