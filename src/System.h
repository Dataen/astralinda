#ifndef LIMITEDSPACE_SYSTEM_H
#define LIMITEDSPACE_SYSTEM_H

#include <Component.h>
#include <Entity.h>
#include <EntityManager.h>
#include <Assets.h>
#include "SoundManager.h"

struct SystemContext
{
    Entity player;
    EntityManager& entity_manager;
    SoundManager& sound_manager;
    Assets& assets;
    f32 circle_radius;
    f32 death_distance;
    bool& game_over;
};

namespace System
{
    void update_pickups(SystemContext& context, f32 dt);
    void update_stars(SystemContext& context, f32 dt);
    void update_enemies(SystemContext& context, f32 dt);
    void update_physics(SystemContext& context, f32 dt);
    void update_projectiles(SystemContext& context, f32 dt);
    void update_effects(SystemContext& context, f32 dt);
    void update_player_enemy_collisions(SystemContext& context, f32 dt);
    void update_player_pickup_collisions(SystemContext& context, f32 dt);
    void update_projectile_collisions(SystemContext& context, f32 dt);
    void update_health_circle_radius(SystemContext& context, f32 dt);
}

#endif //LIMITEDSPACE_SYSTEM_H
