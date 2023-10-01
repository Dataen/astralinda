#pragma once

#include <types.h>
#include <Entity.h>
#include <EntityManager.h>
#include <Assets.h>
#include <Loader.h>
#include <System.h>

#include <raylib.h>
#include <vector>
#include <memory>
#include "SoundManager.h"

struct GameSpecification
{
    u32 width;
    u32 height;
    bool resizable_window;
};

class Game
{
public:
    explicit Game(const GameSpecification& spec);
    void run();

private:
    void load_assets();
    void setup_level(u32 level_num);

    void update_player(f32 dt);
    void update(f32 dt);
    void render();

    Assets assets;
    std::unique_ptr<SoundManager> sound_manger { nullptr };
    EntityManager entity_manager {};
    Entity player;

    bool pause { false };
    bool game_over { false };
    bool game_won { false };
    bool game_start { false };

    // Level stuff

    f32 bonus_timer { 0.0f };

    u32 level_index { 0 };
    f32 circle_radius { 2000 };
    f32 level_fade { 0 };
};
