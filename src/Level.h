#pragma once

#include <string>
#include <types.h>
#include <Component.h>

struct Level
{
    std::string name;
    std::vector<EnemyType> enemy_types;
    u32 bonus_score;
    u32 bonus_time_seconds;
    u32 circle_radius;
};