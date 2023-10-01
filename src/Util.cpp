#include <Util.h>

#include <cmath>

u32 Util::random_u32(u32 min, u32 max)
{
    std::random_device rd {};
    std::mt19937 rng(rd());
    std::uniform_int_distribution<u32> dist(min, max);
    return dist(rng);
}

u8 Util::random_u8(u8 min, u8 max)
{
    std::random_device rd {};
    std::mt19937 rng(rd());
    std::uniform_int_distribution<u8> dist(min, max);
    return dist(rng);
}

f32 Util::random_f32(f32 min, f32 max)
{
    std::random_device rd {};
    std::mt19937 rng(rd());
    std::uniform_real_distribution<f32> dist(min, max);
    return dist(rng);
}

f32 Util::lerp(f32 a, f32 b, f32 t)
{
    return (1.0f - t) * a + t * b;
}

Vector2 Util::get_polar_coordinates(f32 angle_rad, f32 circle_radius)
{
    return {
        circle_radius * std::cos(angle_rad),
        circle_radius * std::sin(angle_rad)
    };
}

f32 Util::get_angle_between_points(Vector2 start, Vector2 end)
{
    return atan2f(end.y - start.y, end.x - start.x);
}

f32 Util::distance_between_points(Vector2 start, Vector2 end)
{
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<f32> Util::get_evenly_spaced_angles(u32 num_angles)
{
    std::vector<f32> angles;
    f32 angle_increment = static_cast<f32>(2.0f * M_PI / num_angles);

    for (u32 i = 1; i < num_angles + 1; ++i) {
        f32 angle = static_cast<f32>(i) * angle_increment;
        angles.push_back(angle);
    }

    return angles;
}
