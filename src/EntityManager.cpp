
#include <EntityManager.h>
#include <Component.h>
#include <Util.h>

#include <vector>

Entity EntityManager::create_entity()
{
    auto entity = registry.create();
    return Entity(entity, &registry);
}

Entity EntityManager::create_enemy_ship(Texture2D& texture, EnemyType type, f32 x, f32 y, f32 rotation)
{
    std::vector<Color> colors = {
        DARKGRAY,
        DARKBROWN,
        RED,
        ORANGE,
        GREEN,
        DARKBLUE,
        BROWN
    };

    ProjectileType projectile_type = ProjectileType::LASER;
    u32 multi_shot_amount = 1;
    f32 size_x = 32.0f;
    f32 size_y = 32.0f;
    f32 scale = Util::random_f32(0.8f, 1.2f);
    f32 collider_radius = 15.0f * scale;
    s32 health = 100;
    s32 shield = 0;
    u32 offset_x = 32;
    u32 offset_y = 0;
    f32 thrust = 50.0f;
    f32 rotation_speed = 2.0f;
    f32 shoot_delay_max = 1.0f;

    switch(type)
    {
        case EnemyType::BASIC:
        {
            health = 25;
            shield = 0;
            offset_x = 32 * 0;
            offset_y = 0;
            thrust = 50.0f;
            rotation_speed = Util::random_f32(1.5f, 1.5f);
            shoot_delay_max = Util::random_f32(1.5f, 2.5f);
            projectile_type = ProjectileType::SHELL;
            multi_shot_amount = 1;

            break;
        }
        case EnemyType::SHIELD:
        {
            scale = Util::random_f32(1.8f, 2.5f);
            collider_radius = 15.0f * scale;
            health = 25;
            shield = 100;
            offset_x = 32 * 4;
            offset_y = 0;
            thrust = 100.0f;
            rotation_speed = Util::random_f32(0.5f, 1.0f);
            projectile_type = ProjectileType::LASER;
            multi_shot_amount = 1;

            break;
        }
        case EnemyType::TANKY:
        {
            scale = Util::random_f32(1.8f, 2.5f);
            collider_radius = 15.0f * scale;
            health = 200;
            shield = 0;
            offset_x = 32 * 1;
            offset_y = 0;
            thrust = 25.0f;
            rotation_speed = Util::random_f32(0.25f, 0.75f);
            projectile_type = ProjectileType::ROCKET;
            multi_shot_amount = 3;

            break;
        }
        case EnemyType::SPEEDY:
        {
            scale = Util::random_f32(0.25f, 0.5f);
            collider_radius = 15.0f * scale;
            health = 10;
            shield = 1;
            offset_x = 32 * Util::random_u32(2, 3);
            offset_y = 0;
            thrust = 150.0f;
            projectile_type = ProjectileType::LASER;
            rotation_speed = Util::random_f32(2.5f, 5.0f);
            shoot_delay_max = 0.5f;
            multi_shot_amount = 3;

            break;
        }
        case EnemyType::BOSS:
            break;
    }

    auto entity = create_entity();
    entity.add_component<Component::Enemy>(rotation_speed, 0.0f, shoot_delay_max, projectile_type, multi_shot_amount);
    entity.add_component<Component::CircleCollider>(collider_radius);
    entity.add_component<Component::Health>(true, true, shield, health, shield, health);
    entity.add_component<Component::Sprite>(&texture, offset_x, offset_y, Util::pick_random_from_vector(colors));
    entity.add_component<Component::Transform>(
            rotation,
            Vector2{ x, y },
            scale,
            Vector2{ size_x, size_y });
    entity.add_component<Component::Physics>(thrust, Vector2{ 0.0f, 0.0f }, Vector2{ 0.0f, 0.0f });
    return entity;
}

Entity EntityManager::create_player(Texture2D& texture, f32 x, f32 y, f32 rotation)
{
    std::vector<Color> colors = {
        WHITE
    };

    auto entity = create_entity();
    entity.add_component<Component::Player>();
    entity.add_component<Component::CircleCollider>(12.0f);
    entity.add_component<Component::Sprite>(&texture, (u32) 0, (u32) 0, Util::pick_random_from_vector(colors));
    entity.add_component<Component::Health>(true, false, 0, 100);
    entity.add_component<Component::Transform>(rotation, Vector2{ x, y }, 1.0f, Vector2{ 32.0f, 32.0f });
    entity.add_component<Component::Physics>(50.0f, Vector2{ 0.0f, 0.0f }, Vector2{ 0.0f, 0.0f });
    return entity;
}

Entity EntityManager::create_star(Texture2D& texture, f32 x, f32 y)
{
    auto rotation = Util::random_f32(0.0f, 2.0f * static_cast<f32>(M_PI));
    auto scale = Util::random_f32(Component::Star::min_scale, Component::Star::max_scale);

    auto entity = create_entity();
    entity.add_component<Component::Star>();
    entity.add_component<Component::Sprite>(&texture);
    entity.add_component<Component::Transform>(rotation, Vector2{ x, y }, scale, Vector2{ 32.0f, 32.0f });
    return entity;
}

Entity EntityManager::create_projectile(Texture2D& texture, ProjectileType type, Entity owner, f32 x, f32 y, f32 rotation)
{
    u32 offset_x = 0;
    f32 thrust = 1000.0f;
    f32 size = 16.0f;
    Color color = WHITE;
    u32 damage = 10;

    switch(type)
    {
        case ProjectileType::SHELL:
        {
            offset_x = 0 * 32;
            thrust = 500.0f;
            size = 32.0f;
            color = BROWN;
            damage = 10;
            break;
        }
        case ProjectileType::LASER:
        {
            offset_x = 1 * 32;
            thrust = 4000.0f;
            size = 16.0f;
            color = WHITE;
            damage = 5;
            break;
        }
        case ProjectileType::ROCKET:
        {
            offset_x = 2 * 32;
            thrust = 2500.0f;
            size = 32.0f;
            color = DARKBROWN;
            damage = 25;
            break;
        }
        case ProjectileType::HOMING:
        {
            offset_x = 3 * 32;
            thrust = 1000.0f;
            size = 32.0f;
            color = PINK;
            damage = 20;
            break;
        }
    }

    auto entity = create_entity();
    entity.add_component<Component::Projectile>(color, damage, owner, type);
    entity.add_component<Component::CircleCollider>(5.0f);
    entity.add_component<Component::Sprite>(&texture, offset_x, (u32) 0);
    entity.add_component<Component::Health>(true, false, 0, 1);
    entity.add_component<Component::Transform>(rotation, Vector2{ x, y }, 1.0f, Vector2{ size, size });
    entity.add_component<Component::Physics>(thrust, Vector2{ 0.0f, 0.0f }, Vector2{ 0.0f, 0.0f });
    return entity;
}

Entity EntityManager::create_effect(Texture2D& texture, EffectType type, f32 x, f32 y, f32 lifetime)
{
    Vector2 size { 32.0f, 32.0f };

    if (type == EffectType::SPARKS)
    {
        size = { Util::random_f32(5.0f, 8.0f), Util::random_f32(5.0f, 8.0f) };
    }

    auto rotation = Util::random_f32(0.0f, 2.0f * M_PI);
    auto scale = Util::random_f32(0.75f, 1.25f);

    auto entity = create_entity();
    entity.add_component<Component::Sprite>(&texture);
    entity.add_component<Component::Effect>(type, lifetime);
    entity.add_component<Component::Transform>(rotation, Vector2{ x, y }, scale, size);
    return entity;
}

Entity EntityManager::create_pickup(Texture2D& texture, PickupType type, f32 x, f32 y)
{
    u32 amount = 0;
    u32 offset_x = 0;

    switch(type)
    {
        case PickupType::COINS:
            amount = Util::random_u32(5, 10);
            offset_x = 32 * 0;
            break;
        case PickupType::HEALTH:
            amount = Util::random_u32(50, 75);
            offset_x = 32 * 1;
            break;
        case PickupType::SHIELD:
            amount = Util::random_u32(50, 75);
            offset_x = 32 * 2;
            break;
        case PickupType::SHELL:
            amount = Util::random_u32(20, 30);
            offset_x = 32 * 3;
            break;
        case PickupType::ROCKET:
            amount = Util::random_u32(15, 25);
            offset_x = 32 * 4;
            break;
        case PickupType::HOMING:
            amount = Util::random_u32(10, 15);
            offset_x = 32 * 5;
            break;
        case PickupType::SHOT_UPGRADE:
            amount = 1;
            offset_x = 32 * 6;
            break;
        case PickupType::ENGINE_UPGRADE:
            amount = 1;
            offset_x = 32 * 7;
            break;
    }

    auto entity = create_entity();
    entity.add_component<Component::Sprite>(&texture, offset_x, (u32) 0);
    entity.add_component<Component::Pickup>(type, amount);
    entity.add_component<Component::CircleCollider>(20.0f);
    entity.add_component<Component::Transform>(0.0f, Vector2{ x, y }, 1.0f, Vector2{ 32.0f, 32.0f });
    return entity;
}

