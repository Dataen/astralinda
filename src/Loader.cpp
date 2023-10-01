#include "Loader.h"

#include <fstream>
#include <json.hpp>

std::optional<std::vector<Level>>
Loader::load_levels(const std::string& file)
{
    std::ifstream input_file(file);

    if (!input_file.is_open()) {
        return std::nullopt;
    }

    nlohmann::json json_data;
    input_file >> json_data;

    input_file.close();

    std::vector<Level> levels;

    for (const auto& level_json : json_data) {
        Level level;

        level.name = level_json["name"];
        level.bonus_score = level_json["bonus_score"];
        level.circle_radius = level_json["circle_radius"];
        level.bonus_time_seconds = level_json["bonus_time_seconds"];

        for (u32 i = 0; i < level_json["basic_enemies"]; ++i)
            level.enemy_types.push_back(EnemyType::BASIC);

        for (u32 i = 0; i < level_json["shield_enemies"]; ++i)
            level.enemy_types.push_back(EnemyType::SHIELD);

        for (u32 i = 0; i < level_json["tanky_enemies"]; ++i)
            level.enemy_types.push_back(EnemyType::TANKY);

        for (u32 i = 0; i < level_json["speedy_enemies"]; ++i)
            level.enemy_types.push_back(EnemyType::SPEEDY);

        for (u32 i = 0; i < level_json["bosses"]; ++i)
            level.enemy_types.push_back(EnemyType::BOSS);

        levels.push_back(level);
    }

    return levels;
}
