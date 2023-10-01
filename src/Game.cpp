#include <Game.h>
#include <Component.h>
#include <Util.h>
#include <System.h>

#include <rlgl.h>
#include <iostream>
#include <cmath>

#define SQRT_2 1.41421354
#define PI2 (2 * PI)

namespace
{
    std::string projectile_type_to_string(ProjectileType type)
    {
        switch(type)
        {
            case ProjectileType::LASER:  return "Laser";
            case ProjectileType::SHELL:  return "Shell";
            case ProjectileType::ROCKET: return "Rocket";
            case ProjectileType::HOMING: return "Missile";
        }
        return "???";
    }

    void render_centered_text(const char* text, f32 y, u32 font_size, const Color& color)
    {
        f32 w = GetScreenWidth();
        u32 text_width = MeasureText(text, font_size);
        DrawText(text, (w - text_width) / 2, y, font_size, color);
    }
}

Game::Game(const GameSpecification& spec)
{

    if (spec.resizable_window) SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 1024, "Astralinda");
    this->assets.screen = LoadRenderTexture(spec.width, spec.height);

    load_assets();
    setup_level(level_index);
}

void Game::setup_level(u32 level_index)
{
    if (level_index >= this->assets.levels.size())
    {
        game_won = true;
        return;
    }

    auto level = this->assets.levels.at(level_index);

    bonus_timer = static_cast<f32>(level.bonus_time_seconds);
    game_over = false;
    level_fade = 0.0f;
    entity_manager.registry.clear();
    circle_radius = static_cast<f32>(level.circle_radius);

    // PLAYER
    {
        // Keep thrust and multi-shot upgrades
        f32 thrust = 50.0f;
        u32 multi_shot = 1;
        if (level_index > 0 && player.has_component<Component::Physics>())
        {
            thrust = player.get_component<Component::Physics>().thrust;
        }
        if (level_index > 0 && player.has_component<Component::Player>())
        {
            multi_shot = player.get_component<Component::Player>().multi_shot_amount;
        }

        // Respawn player
        Vector2 pos = Util::get_polar_coordinates(PI2 * 0.75f, circle_radius - 64);
        f32 angle = Util::get_angle_between_points(pos, { 0.0f, 0.0f });
        this->player = entity_manager.create_player(this->assets.ships, pos.x, pos.y, angle);
        this->player.get_component<Component::Physics>().thrust = thrust;
        this->player.get_component<Component::Player>().multi_shot_amount = multi_shot;
    }
    // ENEMIES
    {
        Util::shuffle_vector(level.enemy_types);
        const auto angles = Util::get_evenly_spaced_angles(level.enemy_types.size());
        for (u32 i = 0; i < level.enemy_types.size(); ++i)
        {
            auto type = level.enemy_types.at(i);
            Vector2 pos = Util::get_polar_coordinates(angles.at(i), circle_radius - 100);
            f32 angle = Util::get_angle_between_points(pos, { 0.0f, 0.0f });
            entity_manager.create_enemy_ship(this->assets.ships, type, pos.x, pos.y, angle);
        }
    }
    // STARS
    for (u32 i = 0; i < 100; i++)
    {
        Vector2 pos = Util::get_polar_coordinates(
                Util::random_f32(0.0f, 2.0f * M_PI),
                Util::random_f32(0, circle_radius + 1000));
        entity_manager.create_star(this->assets.stars, pos.x, pos.y);
    }
}

void Game::load_assets()
{
    this->assets.title = LoadTexture("assets/title.png");
    this->assets.ships = LoadTexture("assets/ships.png");
    this->assets.stars = LoadTexture("assets/stars.png");
    this->assets.effects = LoadTexture("assets/effects.png");
    this->assets.projectiles = LoadTexture("assets/projectiles.png");
    this->assets.engine = LoadTexture("assets/engine.png");
    this->assets.warning = LoadTexture("assets/warning.png");
    this->assets.pickups = LoadTexture("assets/pickups.png");

    InitAudioDevice();
    this->assets.ship_death = LoadSound("assets/ship_death.ogg");
    SetSoundVolume(this->assets.ship_death, 0.25f);
    this->assets.hit_laser1 = LoadSound("assets/hit_laser1.ogg");
    SetSoundVolume(this->assets.hit_laser1, 0.25f);
    this->assets.hit_laser2 = LoadSound("assets/hit_laser2.ogg");
    SetSoundVolume(this->assets.hit_laser2, 0.25f);
    this->assets.hit1 = LoadSound("assets/hit1.ogg");
    SetSoundVolume(this->assets.hit1, 0.25f);
    this->assets.hit2 = LoadSound("assets/hit2.ogg");
    SetSoundVolume(this->assets.hit2, 0.25f);
    this->assets.hit3 = LoadSound("assets/hit3.ogg");
    SetSoundVolume(this->assets.hit3, 0.25f);
    this->assets.shoot1 = LoadSound("assets/shoot.ogg");
    SetSoundVolume(this->assets.shoot1, 0.25f);
    this->assets.game_over1 = LoadSound("assets/game_over.ogg");
    SetSoundVolume(this->assets.game_over1, 0.25f);
    this->assets.no_ammo = LoadSound("assets/no_ammo.ogg");
    SetSoundVolume(this->assets.no_ammo, 0.25f);
    this->assets.pickup = LoadSound("assets/pickup.ogg");
    SetSoundVolume(this->assets.pickup, 0.25f);

    const auto levels_optional = Loader::load_levels("assets/levels.json");
    if (!levels_optional.has_value())
    {
        std::cerr << "Unable to load 'assets/levels.json'";
        std::exit(1);
    }
    this->assets.levels = levels_optional.value();

    sound_manger = std::make_unique<SoundManager>(this->assets);
}

void Game::run()
{
    while (!WindowShouldClose())
    {
        f32 dt = GetFrameTime();

        if (game_start)
        {
            update(dt);
            render();
        }
        else
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                game_start = true;
            }

            BeginDrawing();

            DrawTexturePro(
                    assets.title,
                    {0.0f, 0.0f, 2048.0f, 2048.0f},
                    { 0.0f, 0.0f, static_cast<f32>(GetScreenWidth()), static_cast<f32>(GetScreenHeight()) },
                    { 0.0f, 0.0f },
                    0.0f,
                    WHITE);

            EndDrawing();

        }
    }
    CloseWindow();
}


void Game::update_player(f32 dt)
{
    auto& player_component = player.get_component<Component::Player>();
    auto& player_transform = player.get_component<Component::Transform>();
    auto& player_physics = player.get_component<Component::Physics>();

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        player_physics.acc.x += std::cos(player_transform.rotation) * player_physics.thrust * dt;
        player_physics.acc.y += std::sin(player_transform.rotation) * player_physics.thrust * dt;
        const f32 magnitude = player_physics.acc.x * player_physics.acc.x + player_physics.acc.y * player_physics.acc.y;

        // Generate sparks when accelerating fast
        if (magnitude > 800 && Util::random_f32(0.0f, 1.0f) <= 0.01f)
        {
            f32 spread = Util::lerp(2.0f, 8.0f, magnitude / 2500.0f);
            entity_manager.create_effect(
                    assets.effects,
                    EffectType::SPARKS,
                    player_transform.pos.x + Util::random_f32(-spread, spread),
                    player_transform.pos.y + Util::random_f32(-spread, spread),
                    Util::random_f32(1.5f, 4.0f));
        }
    }

    if (IsKeyDown(KEY_ONE))
    {
        player_component.projectile_type = ProjectileType::SHELL;
    }

    if (IsKeyDown(KEY_TWO))
    {
        player_component.projectile_type = ProjectileType::LASER;
    }

    if (IsKeyDown(KEY_THREE))
    {
        player_component.projectile_type = ProjectileType::ROCKET;
    }

    if (IsKeyDown(KEY_FOUR))
    {
        player_component.projectile_type = ProjectileType::HOMING;
    }


    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player_transform.rotation += 3.0f * dt;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player_transform.rotation -= 3.0f * dt;
    }

    if (player_component.shoot_delay > 0) player_component.shoot_delay -= 3.0f * dt;

    if (IsKeyPressed(KEY_SPACE) && player_component.shoot_delay <= 0) {
        for (u32 i = 0; i < player_component.multi_shot_amount; ++i)
        {

            if (player_component.projectile_type == ProjectileType::LASER ||
               (player_component.projectile_type == ProjectileType::SHELL && player_component.shell_amount > 0) ||
               (player_component.projectile_type == ProjectileType::ROCKET && player_component.rocket_amount > 0) ||
               (player_component.projectile_type == ProjectileType::HOMING && player_component.homing_amount > 0))
            {
                sound_manger->play_shoot();
                entity_manager.create_projectile(assets.projectiles, player_component.projectile_type, player, player_transform.pos.x, player_transform.pos.y, player_transform.rotation);
                player_component.shoot_delay = 1.0f;

                if (player_component.projectile_type == ProjectileType::SHELL)  player_component.shell_amount -= 1;
                if (player_component.projectile_type == ProjectileType::ROCKET) player_component.rocket_amount -= 1;
                if (player_component.projectile_type == ProjectileType::HOMING) player_component.homing_amount -= 1;

            } else {
                sound_manger->play_no_ammo();
            }
        }
    }

    if (player_transform.rotation < 0) {
        player_transform.rotation += 2 * PI;
    } else if (player_transform.rotation >= 2 * PI) {
        player_transform.rotation -= 2 * PI;
    }
}

void Game::update(f32 dt)
{
    if (IsKeyPressed(KEY_L))
    {
        setup_level(++level_index);
    }

    // Check if level finished
    auto view = entity_manager.registry.view<Component::Enemy>();
    u32 enemy_count = static_cast<u32>(std::distance(view.begin(), view.end()));
    if (enemy_count == 0 || game_over)
    {
        if (level_fade == 0.0f && bonus_timer > 0.0f)
        {
            auto& player_component = player.get_component<Component::Player>();
            player_component.score += this->assets.levels[level_index].bonus_score;
        }

        level_fade += 2.0f * dt;
        level_fade = std::min(level_fade, 1.0f);

        if (level_fade >= 0.95f && IsKeyPressed(KEY_ENTER))
        {
            if (game_over)
            {
                level_index = 0;
                setup_level(level_index);
            } else {
                setup_level(++level_index);
            }
        }
        return;
    }

    // Pause

    if (IsKeyPressed(KEY_P)) {
        pause = !pause;
    }
    if (pause) return;

    // Update game entities

    SystemContext context {
        player,
        entity_manager,
        *sound_manger,
        assets,
        circle_radius,
        static_cast<f32>(GetScreenWidth() * 2 * SQRT_2),
        game_over
    };

    // Make circle smaller
    circle_radius -= 3.0f * dt;
    circle_radius = std::max(circle_radius, 100.0f);

    // Increase bonus timer
    bonus_timer -= 1.0f * dt;
    bonus_timer = std::max(bonus_timer, 0.0f);

    update_player(dt);
    System::update_stars(context, dt);
    System::update_projectiles(context, dt);
    System::update_enemies(context, dt);
    System::update_physics(context, dt);
    System::update_effects(context, dt);
    System::update_projectile_collisions(context, dt);
    System::update_player_enemy_collisions(context, dt);
    System::update_player_pickup_collisions(context, dt);
    System::update_health_circle_radius(context, dt);
    System::update_pickups(context, dt);
}

void Game::render()
{
    f32 width = assets.screen.texture.width;
    f32 height = assets.screen.texture.height;

    f32 window_width = GetScreenWidth();
    f32 window_height = GetScreenHeight();

    auto& player_transform = player.get_component<Component::Transform>();
    Vector2 camera {
        player_transform.pos.x - width / 2,
        player_transform.pos.y - height / 2
    };

    if (!pause)
    {

        BeginTextureMode(this->assets.screen);

        ClearBackground(Color{ 15, 15, 15, 255 });

        // Draw entities
        auto view = entity_manager.registry.view<Component::Transform, Component::Sprite>();
        for (auto entity : view)
        {
            auto [transform, sprite] = view.get<Component::Transform, Component::Sprite>(entity);
            DrawTexturePro(*sprite.texture,
                           {static_cast<f32>(sprite.offset_x), static_cast<f32>(sprite.offset_y), transform.size.x, transform.size.y},
                           {transform.pos.x - camera.x, transform.pos.y - camera.y, transform.size.x * transform.scale, transform.size.y * transform.scale},
                           {transform.size.x / 2 * transform.scale, transform.size.y / 2 * transform.scale},
                           (transform.rotation * RAD2DEG) + 90.0f,
                           sprite.tint);
        }

        auto view2 = entity_manager.registry.view<Component::Transform, Component::Physics, Component::Sprite>();
        for (auto entity : view2)
        {
            auto [transform, physics, sprite] = view2.get<Component::Transform, Component::Physics, Component::Sprite>(entity);

            if (entity_manager.registry.any_of<Component::Player>(entity))
            {
                const f32 magnitude = physics.acc.x * physics.acc.x + physics.acc.y * physics.acc.y;
                u32 power = std::min((magnitude + 300.0f) / 2300.0f * 4, 4.0f);
                if (power == 0) continue;

                DrawTexturePro(assets.engine,
                               { static_cast<float>(power - 1) * transform.size.x, 0, transform.size.x, transform.size.y },
                               { transform.pos.x - camera.x, transform.pos.y - camera.y, transform.size.x * transform.scale, transform.size.y * transform.scale },
                               { transform.size.x / 2 * transform.scale - 8, - transform.size.y / 2 + 2 },
                               (transform.rotation * RAD2DEG) + 90.0f,
                               WHITE);
                DrawTexturePro(assets.engine,
                               { static_cast<float>(power - 1) * transform.size.x, 0, transform.size.x, transform.size.y },
                               { transform.pos.x - camera.x, transform.pos.y - camera.y, transform.size.x * transform.scale, transform.size.y * transform.scale },
                               { transform.size.x / 2 * transform.scale + 8, - transform.size.y / 2 + 2 },
                               (transform.rotation * RAD2DEG) + 90.0f,
                               WHITE);
            }
        }

        // Draw red markers for each enemy
        auto viewE = entity_manager.registry.view<Component::Transform, Component::Enemy>();
        for (auto entity : viewE)
        {
            auto [transform, enemy] = viewE.get<Component::Transform, Component::Enemy>(entity);
            auto distance = Util::distance_between_points(player_transform.pos, transform.pos);
            if (distance < 2000 && distance > 700)
            {
                auto angle = Util::get_angle_between_points(player_transform.pos, transform.pos);
                auto pos = Util::get_polar_coordinates(angle, 300);
                DrawCircle(player_transform.pos.x + pos.x - camera.x, player_transform.pos.y + pos.y - camera.y, Util::lerp(1.0f, 4.0f, distance / 2000.0f), RED);
            }
        }

        // Draw shield/health bars
        auto view3 = entity_manager.registry.view<Component::Transform, Component::Sprite, Component::Health>();
        for (auto entity : view3)
        {
            auto [transform, sprite, health] = view3.get<Component::Transform, Component::Sprite, Component::Health>(entity);
            if (/*health.show_health_bar*/false && health.health > 0)
            {
                Rectangle health_bar_rect = {
                        transform.pos.x * transform.scale - camera.x,
                        transform.pos.y * transform.scale - camera.y,
                        transform.size.x * transform.scale * (health.health / 100.0f),
                        4
                };
                DrawRectanglePro(health_bar_rect,
                                 { transform.size.x / 2 * transform.scale, transform.size.y / 2 * transform.scale },
                                 (player_transform.rotation * RAD2DEG) + 90.0f,
                                 RED);
            }
            if (health.show_shield_bar && health.shield > 0)
            {
                rlSetLineWidth(Util::lerp(1.0f, 4.0f, health.shield / 100.0f));
                float radius = (transform.scale * transform.size.x) * Util::lerp(0.75f, 1.0f, health.shield / 100.0f);
                DrawCircleLines(transform.pos.x - camera.x, transform.pos.y - camera.y, radius, BLUE);
            }
        }


        // Draw circle
        rlSetLineWidth(4);
        DrawCircleLines(0.0f - camera.x, 0.0f - camera.y, circle_radius, RED);

        EndTextureMode();
    }

    BeginDrawing();
    {
        ClearBackground(MAGENTA);
        float scale = SQRT_2; // make box fit screen


        float scale_factor = std::max(window_width, window_height);

        Rectangle dest = {
                window_width / 2,
                window_height  / 2,
                scale_factor * scale,
                scale_factor * scale
        };

        DrawTexturePro(
                this->assets.screen.texture,
                { 0, 0, width, -height },
                dest,
                { scale_factor / 2 * scale, scale_factor / 2 * scale },
                -player_transform.rotation * RAD2DEG - 90.0f,
                WHITE );

//        DrawText((std::string("x: ") + std::to_string(player_transform.pos.x)).c_str(), 20, window_height - 50, 20, YELLOW);
//        DrawText((std::string("y: ") + std::to_string(player_transform.pos.y)).c_str(), 20, window_height - 70, 20, YELLOW);
//        DrawText((std::string("r: ") + std::to_string(player_transform.rotation * RAD2DEG)).c_str(), 20, window_height - 90, 20, YELLOW);
//        DrawFPS(16, window_height - 26);

        // DRAW HUD

        auto& player_component = player.get_component<Component::Player>();
        auto& player_health = player.get_component<Component::Health>();

        const f32 health_bar_width = Util::lerp(0, 200, player_health.health / 100.0f);
        DrawRectangleRounded({20, 20, health_bar_width, 30}, 4, 10,
                             player_health.health >= 80 ? DARKGREEN : player_health.health >= 30 ? ORANGE : RED);
        DrawRectangleRoundedLines({20, 20, 200, 30}, 4, 10, 4, WHITE);

        const f32 shoot_delay_width = Util::lerp(0, 200, (1.0f - player_component.shoot_delay) / 1.0f);
        DrawRectangleRounded({window_width - 20 - shoot_delay_width, 20, shoot_delay_width, 30}, 4, 10,
                             player_component.shoot_delay <= 0.1f ? WHITE : DARKGRAY);
        DrawRectangleRoundedLines({window_width - 220, 20, 200, 30}, 4, 10, 4, WHITE);

        DrawText((std::string("Level ") + std::to_string(level_index + 1) + std::string(" / ") + std::to_string(this->assets.levels.size())).c_str(), 30, 70, 32, WHITE);
        DrawText((std::string("Score ") + std::to_string(player_component.score)).c_str(), 30, 110, 32, WHITE);

        DrawText((std::string("Bonus Timer: " + std::to_string( (u32) std::floor(bonus_timer)))).c_str(), 30, 150, 32, WHITE);


        DrawText(projectile_type_to_string(player_component.projectile_type).c_str(),
                 window_width - 155.0f,
                 window_height - 100,
                 20,
                 WHITE);

        for (u32 i = 0; i < 4; ++i)
        {
            DrawTexturePro(
                    assets.projectiles,
                    { (3 - i) * 32.0f, 0.0f, 32.0f, 32.0f },
                    { window_width - ((32.0f + 4.0f) * (i+1)), window_height - 36, 32.0f, 32.0f },
                    { 16.0f, 16.0f },
                    0.0f,
                    WHITE);

            std::string text = "99";
            if      (i == 0) text = std::to_string(player_component.homing_amount);
            else if (i == 1) text = std::to_string(player_component.rocket_amount);
            else if (i == 3) text = std::to_string(player_component.shell_amount);

            DrawText(text.c_str(), window_width - ((32.0f + 4.0f) * (i+1)) - 8, window_height - 36 - 36, 20, WHITE);
        }

        if (player_component.outside_circle)
        {
            DrawTextureRec(assets.warning,
                           Rectangle { player_component.warning_timer <= 0.5f ? 0.0f : 128.0f, 0, 128, 128 },
                           Vector2 {window_width / 2 - 64, window_height * 3 / 4 - 64},
                           WHITE);
        }


        // Level fade
        DrawRectangle(0, 0, window_width, window_height, Color{5, 5, 5, static_cast<u8>(Util::lerp(0, 255, level_fade / 1.0f))});

        if (level_fade >= 1.0f)
        {
            if (game_over)
            {
                render_centered_text("Game Over!", 100, 40, WHITE);
                render_centered_text(std::string("You'll have to restart your mission").c_str(), window_height / 2 - 16, 32, WHITE);
                render_centered_text((std::string("You failed on level ") + std::to_string(level_index + 1) + std::string(" with a score of ") + std::to_string(player_component.score)).c_str(), window_height / 2 + 64, 30, WHITE);
                render_centered_text(std::string("Press ENTER to restart!").c_str(), window_height - 132, 32, WHITE);
            }
            else
            {
                if (game_won)
                {
                    render_centered_text("You win! Wait, what?", 100, 40, WHITE);
                    render_centered_text((std::string("Score: ") + std::to_string(player_component.score)).c_str(), window_height / 2 - 16, 32, WHITE);
                    render_centered_text(std::string("Press ENTER to try again!").c_str(), window_height - 132, 32, WHITE);
                }
                else
                {
                    render_centered_text("Level Complete!", 100, 40, WHITE);
                    if (bonus_timer > 0.0f)
                        render_centered_text((std::string("You earned a score bonus by finishing before the timer!")).c_str(), window_height / 2 - 16 - 64, 30, WHITE);
                    render_centered_text((std::string("Next level is ") + std::to_string(level_index + 2)).c_str(), window_height / 2 - 16, 32, WHITE);
                    render_centered_text((std::string("Current score: ") + std::to_string(player_component.score)).c_str(), window_height / 2 + 64, 30, WHITE);
                    render_centered_text(std::string("Press ENTER to continue!").c_str(), window_height - 132, 32, WHITE);
                }
            }
        }

        if (pause) {
            DrawRectangle(0, 0, window_width, window_height, Color{50, 50, 50, 200});
            const auto pause_text = "Paused";
            u32 text_height = 42;
            u32 text_width = MeasureText(pause_text, text_height);
            DrawText(pause_text, (window_width - text_width) / 2, (window_height - text_height) / 2, text_height, WHITE);
        }
    }
    EndDrawing();
}

