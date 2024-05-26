// #include "Behaviours.h"


//     LeaderAI::LeaderAI(int entity_ind, Player *player, EntityData *data, GroupManager *p_groups, BoidSystem *p_boids, BulletSystem* p_bs)
//         : p_groups(p_groups), p_boids(p_boids), p_bs(p_bs), BoidAI(entity_ind, player, data)
//     {
//         return_position = data->r;
//     }

// void LeaderAI::update()
// {
//     auto dist2_to_player = dist2(player->pos, data->r);
//     if (dist2_to_player < vision_radius * vision_radius && !spotted_player )
//     {
//         spotted_player = true;
//     }
//     if (spotted_player && !released_underlings)
//     {
//         auto &group = p_groups->getGroup(data->group_ind);
//         for (auto entity : group)
//         {
//             if (entity == entity_ind)
//             {
//                 continue;
//             }
//             auto new_ai = std::make_unique<ChasePlayer>(
//                 entity,
//                 player,
//                 data
//                 );

//             p_boids->setBehaviourOf(entity, std::move(new_ai));
//         }
//         p_groups->destroyGroup(data->group_ind);

//         spotted_player = false;
//         released_underlings = true;
//         data->target_position = data->r + 2.f * (data->r - player->pos);
//     }
// }

//     UnderlingAI::UnderlingAI(int entity_ind, Player *player, EntityData *data, GroupManager *p_groups, BoidSystem *p_boids, LeaderAI* leader)
//         : p_groups(p_groups), p_boids(p_boids), p_leader(leader), BoidAI(entity_ind, player, data)
//     {
//         return_position = data->r;
//     }

// void UnderlingAI::update()
//     {
//         auto dist2_to_player = dist2(player->pos, data->r);
//         if (dist2_to_player < vision_radius * vision_radius)
//         {
//             p_leader->spotted_player = true;
//         }

//         // auto dist2_to_leader = dist2(data->r, p_leader->data->r);
//         // if(dist2_to_leader > cluster_size*cluster_size){
//         //     data->target_position = p_leader->data->r;
//         // }
//     }