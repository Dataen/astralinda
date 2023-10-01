#pragma once

#include <Entity.h>
#include <entt/entt.hpp>
#include <raylib.h>
#include <types.h>

#include <Component.h>

class EntityManager
{
public:
    EntityManager() = default;

    Entity create_entity();

    Entity create_pickup(Texture2D& texture, PickupType type, f32 x, f32 y);
    Entity create_player(Texture2D& texture, f32 x, f32 y, f32 rotation);
    Entity create_enemy_ship(Texture2D& texture, EnemyType type, f32 x, f32 y, f32 rotation);
    Entity create_star(Texture2D& texture, f32 x, f32 y);
    Entity create_projectile(Texture2D& texture, ProjectileType type, Entity owner, f32 x, f32 y, f32 rotation);
    Entity create_effect(Texture2D& texture, EffectType type, f32 x, f32 y, f32 lifetime = 1.0f);

    entt::registry registry;
};
