
#ifndef LIMITEDSPACE_COMPONENT_H
#define LIMITEDSPACE_COMPONENT_H

#include <types.h>
#include <raylib.h>

#include <Entity.h>

enum class EffectType : u8
{
    EXPLOSION = 0,
    SMOKE     = 1,
    SPARKS    = 2
};

enum class EnemyType : u8
{
    BASIC  = 0,
    SHIELD = 1,
    TANKY  = 2,
    SPEEDY = 3,
    BOSS   = 4
};

enum class ProjectileType : u8
{
    LASER  = 0,
    SHELL  = 1,
    ROCKET = 2,
    HOMING = 3,
};

enum class PickupType : u8
{
    COINS          = 0,
    HEALTH         = 1,
    SHIELD         = 2,
    SHELL          = 3,
    ROCKET         = 4,
    HOMING         = 5,
    SHOT_UPGRADE   = 6,
    ENGINE_UPGRADE = 7
};

namespace Component
{
    struct Pickup
    {
        PickupType type { PickupType::COINS };
        u32 pickup_amount { 1 };

        f32 size { 0.0f };
    };

    struct CircleCollider
    {
        f32 radius { 0.0f };
    };

    struct Effect
    {
        EffectType type;
        f32 lifetime { 1.0f };
    };

    struct Projectile
    {
        Color color { GRAY };
        u32 damage { 0 };
        Entity owner;
        ProjectileType type;
    };

    struct Star {
        bool growing { false };
        static inline f32 max_scale { 0.25f };
        static inline f32 min_scale { 0.0f };
    };

    struct Health {
        bool show_shield_bar { false };
        bool show_health_bar { false };
        s32 shield { 0 };
        s32 health { 100 };
        s32 max_shield { 0 };
        s32 max_health { 100 };
    };

    struct Player {
        u32 score { 0 };
        f32 shoot_delay { 0.0f };
        bool outside_circle { false };
        f32 warning_timer { 0.0f };

        ProjectileType projectile_type { ProjectileType::LASER };
        u32 multi_shot_amount = 1;

        u32 shell_amount { 15 };
        u32 rocket_amount { 3 };
        u32 homing_amount { 0 };
    };

    struct Enemy {
        f32 rotation_speed { 2.0f };
        f32 shoot_delay { 0.0f };
        f32 shoot_delay_max { 1.0f };

        ProjectileType projectile_type { ProjectileType::LASER };
        u32 multi_shot_amount = 1;
    };

    struct Sprite {
        Texture2D* texture { nullptr };
        u32 offset_x { 0 };
        u32 offset_y { 0 };
        Color tint { WHITE };
    };

    struct Physics
    {
        f32 thrust { 0 };
        Vector2 acc { 0, 0 };
        Vector2 vel { 0, 0 };
    };

    struct Transform
    {
        f32 rotation { 0 };
        Vector2 pos { 0,  0 };
        f32 scale { 1.0f };
        Vector2 size { 32.0f, 32.0f };
    };
}

#endif //LIMITEDSPACE_COMPONENT_H
