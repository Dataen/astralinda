#include <System.h>
#include <Util.h>

#include <optional>

namespace
{
    struct ColliderResult
    {
    };

    std::optional<ColliderResult> circle_intersect(const Component::Transform& transform_A,
                          const Component::Transform& transform_B,
                          const Component::CircleCollider& collider_A,
                          const Component::CircleCollider& collider_B)
    {
        auto distance = Util::distance_between_points(transform_A.pos, transform_B.pos);
        if (distance < collider_A.radius + collider_B.radius)
        {
            return std::make_optional<ColliderResult>();
        }
        return std::nullopt;
    }

    void destroy_enemy(SystemContext& context, Entity enemy)
    {
        auto& transform = enemy.get_component<Component::Transform>();
        for (u32 i = 0; i < 50; i++)
        {
            f32 spread = 20.0f;
            EffectType type = i % 2 == 0 ? EffectType::SMOKE : EffectType::EXPLOSION;
            context.entity_manager.create_effect(context.assets.effects,
                                                 type,
                                                 transform.pos.x + Util::random_f32(-spread, spread),
                                                 transform.pos.y + Util::random_f32(-spread, spread),
                                                 Util::random_f32(1.0f, 6.0f));
        }

        if (enemy.has_component<Component::Player>())
        {
            context.sound_manager.play_game_over();
            context.game_over = true;
        }
        else
        {
            if(enemy.has_component<Component::Enemy>())
            {
                context.entity_manager.create_pickup(context.assets.pickups, static_cast<PickupType>(Util::random_u8(0, 7)), transform.pos.x, transform.pos.y);
            }
            context.player.get_component<Component::Player>().score += 10;
            context.sound_manager.play_die();
            context.entity_manager.registry.destroy(enemy);
        }
    }

    void damage_target(SystemContext& context, Entity target, u32 damage)
    {
        if (target.has_component<Component::Health>() && target.has_component<Component::Transform>())
        {
            auto& health_component = target.get_component<Component::Health>();

            if (health_component.shield > 0)
            {
                health_component.shield -= static_cast<s32>(damage);
                health_component.shield = std::max(health_component.shield, 0);
            }
            else if (health_component.health > 0)
            {
                health_component.health -= static_cast<s32>(damage);
                health_component.health = std::max(health_component.health, 0);
            }
            else
            {
                destroy_enemy(context, target);
            }
        }
    }
}

void System::update_stars(SystemContext& context, f32 dt)
{
    auto view = context.entity_manager.registry.view<Component::Transform, Component::Star>();
    for (auto entity : view)
    {
        auto [transform, star] = view.get<Component::Transform, Component::Star>(entity);
        if (star.growing)
        {
            if (transform.scale >= Component::Star::max_scale) star.growing = false;
            else transform.scale += 0.01f * dt;
        } else {
            if (transform.scale <= Component::Star::min_scale) star.growing = true;
            else transform.scale -= 0.01f * dt;
        }
    }
}

void System::update_enemies(SystemContext& context, f32 dt)
{
    auto& player_transform = context.player.get_component<Component::Transform>();
    auto enemy_view = context.entity_manager.registry.view<Component::Transform, Component::Physics, Component::Enemy>();
    for (auto entity : enemy_view)
    {
        auto [transform, physics, enemy] = enemy_view.get<Component::Transform, Component::Physics, Component::Enemy>(entity);

        // Change the thrust randomly
        static f32 thrust = physics.thrust;
        static f32 thrust_timer = 0.0f;
        thrust_timer += 1.0f * dt;
        if (thrust_timer >= 1.0f)
        {
            thrust = Util::random_f32(physics.thrust - 20.0f, physics.thrust + 20.0f);
            thrust_timer = 0.0f;
        }

        // Make enemies accelerate in the direction they are pointing
        physics.acc.x += std::cos(transform.rotation) * thrust * dt;
        physics.acc.y += std::sin(transform.rotation) * thrust * dt;

        // Pull enemies toward center if too far away.
        f32 angle_to_center = Util::get_angle_between_points(transform.pos, { 0.0f, 0.0f });
        f32 distance = Util::distance_between_points(transform.pos, { 0.0f, 0.0f });
        f32 center_pull_thrust = Util::lerp(Util::random_f32(0.0f, 0.01f), physics.thrust, distance / context.circle_radius);
        physics.acc.x += std::cos(angle_to_center) * center_pull_thrust * dt;
        physics.acc.y += std::sin(angle_to_center) * center_pull_thrust * dt;

        // Rotate enemy towards player if within a certain looking radius
        f32 target_rotation = std::atan2(player_transform.pos.y - transform.pos.y, player_transform.pos.x - transform.pos.x);
        f32 rotation_diff = target_rotation - transform.rotation;

        if (rotation_diff > PI) {
            rotation_diff -= 2 * PI;
        } else if (rotation_diff < -PI) {
            rotation_diff += 2 * PI;
        }

        // Shoot projectile when pointing towards and when in range of player
        if (enemy.shoot_delay > 0) enemy.shoot_delay -= 1.0f * dt;
        if (rotation_diff < PI / 8 && enemy.shoot_delay <= 0 && Util::distance_between_points(player_transform.pos, transform.pos) < 800)
        {
            for (u32 i = 0; i < enemy.multi_shot_amount; i++)
            {
                context.sound_manager.play_shoot();
                context.entity_manager.create_projectile(
                        context.assets.projectiles,
                        enemy.projectile_type,
                        Entity(entity, &context.entity_manager.registry),
                        transform.pos.x,
                        transform.pos.y,
                        transform.rotation);
            }
            enemy.shoot_delay = enemy.shoot_delay_max;
        }

        // Gradually update the enemy's rotation
        transform.rotation += rotation_diff * enemy.rotation_speed * dt;

        // Ensure the enemy's rotation is within the range of 0 to 2π
        if (transform.rotation < 0) {
            transform.rotation += 2 * PI;
        } else if (transform.rotation >= 2 * PI) {
            transform.rotation -= 2 * PI;
        }
    }
}

void System::update_physics(SystemContext& context, f32 dt)
{
    auto view = context.entity_manager.registry.view<Component::Transform, Component::Physics>();
    for (auto entity : view)
    {
        auto [transform, physics] = view.get<Component::Transform, Component::Physics>(entity);

        // Decay acceleration
        physics.acc.x -= physics.acc.x * (1.0f - 0.001f) * dt;
        physics.acc.y -= physics.acc.y * (1.0f - 0.001f) * dt;

        // Update velocity based on acceleration
        physics.vel.x += physics.acc.x * physics.thrust * dt;
        physics.vel.y += physics.acc.y * physics.thrust * dt;

        // Decay velocity
        physics.vel.x -= physics.vel.x * (1.0f - 0.001f) * dt;
        physics.vel.y -= physics.vel.y * (1.0f - 0.001f) * dt;

        // Update position based on velocity
        transform.pos.x += physics.vel.x * dt;
        transform.pos.y += physics.vel.y * dt;

        // Cap velocity to a maximium
        const float max_velocity = 200.0f;
        float velocity_magnitude = sqrtf( physics.vel.x * physics.vel.x + physics.vel.y * physics.vel.y);
        if (velocity_magnitude > max_velocity)
        {
            physics.vel.x = (physics.vel.x / velocity_magnitude) * max_velocity;
            physics.vel.y = (physics.vel.y / velocity_magnitude) * max_velocity;
        }
    }
}

void System::update_projectile_collisions(SystemContext& context, f32 dt)
{
    auto view = context.entity_manager.registry.view<Component::Transform, Component::CircleCollider, Component::Projectile>();
    for (auto projectile_entity : view)
    {
        auto [projectile_transform, projectile_collider, projectile] = view.get<Component::Transform, Component::CircleCollider, Component::Projectile>(projectile_entity);

        auto other_view = context.entity_manager.registry.view<Component::Transform, Component::CircleCollider>();

        for (auto entity : other_view)
        {
            auto game_entity = Entity(entity, &context.entity_manager.registry);

            if (projectile_entity == game_entity)
            {
                // Skip checking with itself
                continue;
            }
            else if (projectile.owner == game_entity ||
            (projectile.owner.has_component<Component::Enemy>() &&  game_entity.has_component<Component::Enemy>()))
            {
                // Skip if hit with projectile's owner
                // Or if enemy hit enemy
                continue;
            }
            else if (game_entity.has_component<Component::Projectile>())
            {
                // Skip if both are projectiles and they either have same owner
                // or both owners are enemies
                auto game_entity_projectile = view.get<Component::Projectile>(game_entity);
                if (game_entity_projectile.owner == projectile.owner ||
                (game_entity_projectile.owner.has_component<Component::Enemy>() && projectile.owner.has_component<Component::Enemy>())) {
                    continue;
                }
            } else if (game_entity.has_component<Component::Pickup>())
            {
                // Skip hitting pickups
                continue;
            }

            auto [transform, collider] = view.get<Component::Transform, Component::CircleCollider>(game_entity);

            auto result = circle_intersect(projectile_transform, transform, projectile_collider, collider);
            if (result.has_value()) {
                Vector2 effect_spawn_point {
                        projectile_transform.pos.x,
                        projectile_transform.pos.y
                };

                context.sound_manager.play_hit(projectile.type);

                context.entity_manager.create_effect(context.assets.effects, EffectType::EXPLOSION, effect_spawn_point.x, effect_spawn_point.y);
                context.entity_manager.registry.destroy(projectile_entity);

                // Can the "collided with entity" die?
                damage_target(context, game_entity, projectile.damage);

                break;
            }
        }
    }
}

void System::update_projectiles(SystemContext& context, f32 dt)
{
    auto& player_transform = context.player.get_component<Component::Transform>();

    auto view = context.entity_manager.registry.view<Component::Transform, Component::Physics, Component::Projectile>();
    for (auto projectile_entity : view)
    {
        auto [transform, physics, projectile] = view.get<Component::Transform, Component::Physics, Component::Projectile>(projectile_entity);

        if (projectile.type == ProjectileType::HOMING)
        {
            f32 rotation_diff = 0.0f;

            if (projectile.owner.has_component<Component::Player>())
            {
                f32 shortest_distance = 99999.0;
                entt::entity enemy = entt::null;
                auto view_enemies = context.entity_manager.registry.view<Component::Transform, Component::Enemy>();
                for (auto enemy_entity : view_enemies)
                {
                    auto [enemy_transform, enemy_component] = view_enemies.get<Component::Transform, Component::Enemy>(enemy_entity);
                    f32 distance = Util::distance_between_points(enemy_transform.pos, transform.pos);
                    if (distance < shortest_distance)
                    {
                        shortest_distance = distance;
                        enemy = enemy_entity;
                    }
                }

                if (enemy != entt::null)
                {
                    Entity enemy_game_entity = Entity(enemy, &context.entity_manager.registry);
                    auto enemy_transform = enemy_game_entity.get_component<Component::Transform>();
                    f32 target_rotation = std::atan2(enemy_transform.pos.y - transform.pos.y, enemy_transform.pos.x - transform.pos.x);
                    rotation_diff = target_rotation - transform.rotation;
                }

            } else {
                f32 target_rotation = std::atan2(player_transform.pos.y - transform.pos.y, player_transform.pos.x - transform.pos.x);
                rotation_diff = target_rotation - transform.rotation;
            }

            if (rotation_diff > PI) {
                rotation_diff -= 2 * PI;
            } else if (rotation_diff < -PI) {
                rotation_diff += 2 * PI;
            }

            // Gradually update the rotation
            transform.rotation += rotation_diff * 3.0f * dt;

            // Ensure the enemy's rotation is within the range of 0 to 2π
            if (transform.rotation < 0) {
                transform.rotation += 2 * PI;
            } else if (transform.rotation >= 2 * PI) {
                transform.rotation -= 2 * PI;
            }
        }

        physics.acc.x += std::cos(transform.rotation) * physics.thrust * dt;
        physics.acc.y += std::sin(transform.rotation) * physics.thrust * dt;

        float dx = transform.pos.x - player_transform.pos.x;
        float dy = transform.pos.y - player_transform.pos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance >= context.death_distance)
        {
            context.entity_manager.registry.destroy(projectile_entity);
        }
    }
}

void System::update_effects(SystemContext& context, f32 dt)
{
    auto view = context.entity_manager.registry.view<Component::Transform, Component::Effect, Component::Sprite>();
    for (auto entity : view)
    {
        auto [transform, effect, sprite] = view.get<Component::Transform, Component::Effect, Component::Sprite>(entity);

        effect.lifetime -= 5.0f * dt;

        if (effect.lifetime <= 0.0f) {
            context.entity_manager.registry.destroy(entity);
            continue;
        }

        auto total_sprites_in_animation = sprite.texture->width / transform.size.x;
        auto current_sprite = std::floor((1.0f - effect.lifetime) / 1.0f * total_sprites_in_animation);

        sprite.offset_x = current_sprite * transform.size.x;
        sprite.offset_y = static_cast<u32>(effect.type) * transform.size.y;

    }
}

void System::update_player_enemy_collisions(SystemContext& context, f32 dt)
{
    auto& player_transform = context.player.get_component<Component::Transform>();
    auto& player_physics = context.player.get_component<Component::Physics>();
    auto& player_collider = context.player.get_component<Component::CircleCollider>();

    auto view = context.entity_manager.registry.view<Component::Transform, Component::Physics, Component::CircleCollider, Component::Enemy, Component::Health>();
    for (auto enemy_entity : view)
    {
        auto [transform, physics, collider, enemy, health] = view.get<Component::Transform, Component::Physics, Component::CircleCollider, Component::Enemy, Component::Health>(enemy_entity);

        auto result = circle_intersect(player_transform, transform, player_collider, collider);

        if (result.has_value())
        {
            damage_target(context, context.player, health.max_health + health.shield);
            destroy_enemy(context, Entity(enemy_entity, &context.entity_manager.registry));
        }
    }
}

void System::update_health_circle_radius(SystemContext& context, f32 dt)
{
    static f32 damage_counter = 0.0f;
    damage_counter += 5.0f * dt;
    damage_counter = std::min(damage_counter, 1.0f);

    auto view = context.entity_manager.registry.view<Component::Transform, Component::Health>();
    for (auto entity : view)
    {
        auto [transform, health] = view.get<Component::Transform, Component::Health>(entity);

        auto game_entity = Entity(entity, &context.entity_manager.registry);

        f32 dx = transform.pos.x;
        f32 dy = transform.pos.y;
        f32 distance = std::sqrt(dx * dx + dy * dy);

        bool outside_circle = distance >= context.circle_radius;

        if (outside_circle)
        {
            if (damage_counter >= 1.0f && !game_entity.has_component<Component::Projectile>())
            {
                damage_target(context, game_entity, 1);
                damage_counter = 0.0f;
            }
        }

        if (game_entity.has_component<Component::Player>())
        {


            auto& player_component = game_entity.get_component<Component::Player>();
            player_component.outside_circle = outside_circle;
            if (outside_circle)
            {
                player_component.warning_timer += 1.0f * dt;
                if (player_component.warning_timer >= 1.0f)
                {
                    player_component.warning_timer = 0;
                }
            }
        }
    }
}

void System::update_pickups(SystemContext& context, f32 dt)
{
    auto view = context.entity_manager.registry.view<Component::Transform, Component::Pickup>();
    for (auto entity : view)
    {
        auto [transform, pickup] = view.get<Component::Transform, Component::Pickup>(entity);
        if (pickup.size < 1.0f)
        {
            pickup.size += 0.25f * dt;
            transform.scale = pickup.size;
            transform.rotation += 2.0f * dt;
        }
    }
}

void System::update_player_pickup_collisions(SystemContext& context, f32 dt)
{
    auto& player_transform = context.player.get_component<Component::Transform>();
    auto& player_physics = context.player.get_component<Component::Physics>();
    auto& player_collider = context.player.get_component<Component::CircleCollider>();
    auto& player_component = context.player.get_component<Component::Player>();
    auto& player_health = context.player.get_component<Component::Health>();

    auto view = context.entity_manager.registry.view<Component::Transform, Component::CircleCollider, Component::Pickup>();
    for (auto entity : view)
    {
        auto [transform, collider, pickup] = view.get<Component::Transform, Component::CircleCollider, Component::Pickup>(entity);

        auto result = circle_intersect(player_transform, transform, player_collider, collider);
        if (result.has_value())
        {
            context.entity_manager.registry.destroy(entity);

            switch(pickup.type)
            {
                case PickupType::COINS:
                {
                    player_component.score += pickup.pickup_amount;
                    break;
                }
                case PickupType::HEALTH:
                {
                    player_health.health += pickup.pickup_amount;
                    player_health.health = std::min (player_health.health, player_health.max_health);
                    break;
                }
                case PickupType::SHIELD:
                {
                    player_health.shield += pickup.pickup_amount;
                    player_health.shield = std::min (player_health.shield, player_health.max_shield);
                    break;
                }
                case PickupType::SHELL:
                {
                    player_component.shell_amount += pickup.pickup_amount;
                    break;
                }
                case PickupType::ROCKET:
                {
                    player_component.rocket_amount += pickup.pickup_amount;
                    break;
                }
                case PickupType::HOMING:
                {
                    player_component.homing_amount += pickup.pickup_amount;
                    break;
                }
                case PickupType::SHOT_UPGRADE:
                {
                    player_component.multi_shot_amount += 1;
                    break;
                }
                case PickupType::ENGINE_UPGRADE:
                {
                    player_physics.thrust += 25.0f;
                    break;
                }
            }
        }
    }
}
