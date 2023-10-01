#pragma once

#include <types.h>

#include <raylib.h>
#include <random>
#include <vector>
#include <algorithm>

namespace Util
{
    u8 random_u8(u8 min, u8 max);
    u32 random_u32(u32 min, u32 max);
    f32 random_f32(f32 min, f32 max);
    f32 lerp(f32 a, f32 b, f32 t);

    Vector2 get_polar_coordinates(f32 angle_rad, f32 circle_radius);
    f32 get_angle_between_points(Vector2 start, Vector2 end);
    f32 distance_between_points(Vector2 start, Vector2 end);

    std::vector<f32> get_evenly_spaced_angles(u32 num_angles);

    template <typename T>
    T pick_random_from_vector(std::vector<T>& vec)
    {
        u32 index = random_u32(0, vec.size() - 1);
        return vec[index];
    }

    template <typename T>
    void shuffle_vector(std::vector<T>& vec) {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::shuffle(vec.begin(), vec.end(), gen);
    }
}
