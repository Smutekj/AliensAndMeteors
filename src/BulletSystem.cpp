#include "BulletSystem.h"
#include "PolygonObstacleManager.h"
#include "BoidSystem.h"

void BulletSystem::update()
{
    auto &boids = p_boids->getBoids();

    std::vector<int> to_destroy_lasers;
    for(auto laser_ind : lasers.active_inds){
        auto& laser = lasers.at(laser_ind);
        laser.time++;
        
        if(lasers.at(laser_ind).time > 60){
            Polygon laser_poly(4);
            laser_poly.points.at(0) = {-0.5, -0.5};
            laser_poly.points.at(1) = {0.5, -0.5};
            laser_poly.points.at(2) = {0.5, 0.5};
            laser_poly.points.at(3) = {-0.5, 0.5};
            auto end_position = laser.start_position;
            end_position.x += laser.length*std::cos(laser.angle*M_PIf/180.f);
            end_position.y += laser.length*std::sin(laser.angle*M_PIf/180.f);

            laser_poly.setPosition((laser.start_position + end_position)/2.f);
            laser_poly.setRotation(laser.angle);
            laser_poly.setScale({laser.length, laser.width});
            for (auto &boid : boids)
            {
                auto mvt = laser_poly.getMVTOfSphere(boid.r, boid.radius);
                if(norm2(mvt) > 0.001f){
                    p_boids->entity2boid_data.at(boid.entity_ind).health -= 5;
                }
            }
            to_destroy_lasers.push_back(laser_ind);
        }
        
    }
    for(auto ind : to_destroy_lasers){
        lasers.remove(ind);
    }




    std::vector<int> filled_cells;
    int boid_ind = 0;
    for (auto &boid : boids)
    {
        auto grid_ind = p_bullet_grid->coordToCell(boid.r);
        grid2entities.at(grid_ind).push_back(boid_ind);
        if (visited_grids.count(grid_ind) == 0)
        {
            visited_grids.insert(grid_ind);
        }
        boid_ind++;
    }

    std::vector<int> to_remove;
    for (int bullet_ind = 0; bullet_ind < bullets.size(); bullet_ind++)
    {
        auto &bullet = bullets.data.at(bullet_ind);
        integrateAndSteer(bullet_ind);
        bullet.lifetime++;
        if (bullet.lifetime > bullet.max_lifetime)
        {
            to_remove.push_back(bullet_ind);
            continue;
        }

        if (bullet.lifetime > 60) //! we check for collisions only after some time
        {
            auto colliding_entity = findCollidingBoid(bullet_ind);
            if (colliding_entity != -1)
            {
                p_boids->entity2boid_data.at(colliding_entity).health -= bullet.dmg;
                to_remove.push_back(bullet_ind);
                continue;
            }
        }

        collideWithMeteors(bullet_ind);

        if (bullet.player)
        {
            bullet.target = bullet.player->pos;
            if (dist(bullet.target, bullet.pos) < bullet.radius)
            {
                to_remove.push_back(bullet_ind);
                continue;
            }
        }else{
            bullet.target = bullet.pos + 5.f*bullet.vel;
        }
        

        // integrateAndSteer(bullet_ind);
        bullet.pos += bullet.vel;
    }

    std::set<int> s(to_remove.begin(), to_remove.end());
    assert(s.size() == to_remove.size());

    while (!to_remove.empty())
    {
        bullets.remove(to_remove.back());
        to_remove.pop_back();
    }

    for (auto grid_ind : visited_grids)
    {
        grid2entities.at(grid_ind).clear();
    }
    visited_grids.clear();

    assert(std::all_of(grid2entities.begin(), grid2entities.end(), [](const auto &inds)
                       { return inds.empty(); }));
}

void BulletSystem::collideWithMeteors(int bullet_ind){
    auto& bullet = bullets.data.at(bullet_ind);
    for(auto i : p_meteors->meteors.active_inds)
    {       
        auto& meteor = p_meteors->meteors.at(i);
        auto mvt = meteor.getMVTOfSphere(bullet.pos, bullet.radius);
        float vel_in_mvt_dir = dot(bullet.vel, mvt);
        if(norm2(mvt) > 0.001f && vel_in_mvt_dir < 0.f){
            bullet.vel -= 2.f*vel_in_mvt_dir*mvt;
            bullet.player = nullptr;; //! stop pursuing player
        }
    }
}


int BulletSystem::findCollidingBoid(int bullet_ind)
{

    auto &bullet = bullets.data.at(bullet_ind);
    auto &boids = p_boids->getBoids();

    auto grid_ind = p_bullet_grid->coordToCell(bullet.pos);
    std::array<int, 9> nearest_cells;
    int n_nearest_cells;
    p_bullet_grid->calcNearestCells(grid_ind, nearest_cells, n_nearest_cells);
    nearest_cells[n_nearest_cells] = grid_ind;
    n_nearest_cells++;

    bool hit = false;
    for (int n_cell = 0; n_cell < n_nearest_cells && !hit; ++n_cell)
    {
        auto neighbour_grid_ind = nearest_cells.at(n_cell);
        for (auto &boid_ind : grid2entities.at(neighbour_grid_ind))
        {
            auto &boid = boids.at(boid_ind);
            if (dist2(boid.r, bullet.pos) < boid.radius * boid.radius)
            {
                return boid.entity_ind;
            }
        }
    }
    return -1;
}

void BulletSystem::integrateAndSteer(int bullet_ind)
{
    auto &bullet = bullets.data.at(bullet_ind);
    if(!bullet.player){
        // bullet.pos += bullet.vel; 
        return;
    }
    
    if (bullet.target.x > 0.f && bullet.target.y > 0.f)
    { //! if the target is inside we steer towards him
        
        auto dr_to_target = bullet.target - bullet.pos;
        auto acc = bullet.max_speed * dr_to_target / norm(dr_to_target) - bullet.vel;

        auto angle = 180.f / M_PI * std::atan2(dr_to_target.y, dr_to_target.x);
        auto d_angle = std::min(std::abs(angle - bullet.orientation), 360 - std::abs(angle - bullet.orientation));

        sf::Vector2f dir = {std::cos(angle * M_PI / 180.f), std::sin(angle * M_PI / 180.f)};

        if (d_angle < 5.f)
        {
            bullet.orientation = angle;
        }
        else if (dot(dir, dr_to_target) < 0.f)
        {
            bullet.orientation -= 5.f;
        }
        else
        {
            bullet.orientation += 5.f;
        }

        if (bullet.orientation > 180)
        {
            bullet.orientation -= 360;
        }
        if (bullet.orientation < -180)
        {
            bullet.orientation += 360;
        }

        // bullet.vel.x = bullet.max_speed * std::cos(bullet.orientation * M_PI / 180.f);
        // bullet.vel.y = bullet.max_speed * std::sin(bullet.orientation * M_PI / 180.f);
        bullet.vel += 0.01f*acc;
    }
    truncate(bullet.vel, bullet.max_speed);
    // bullet.pos += bullet.vel;
}