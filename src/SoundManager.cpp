#include <SoundManager.h>
#include <raylib.h>
#include <Util.h>

SoundManager::SoundManager(Assets& assets)
    : assets(assets)
{
}


void SoundManager::play_hit(ProjectileType type)
{
    if (type == ProjectileType::LASER)
    {
        PlaySound(Util::random_u32(1, 2) == 2 ? assets.hit_laser1 : assets.hit_laser2);
    } else {
        u32 sound = Util::random_u32(1, 3);
        if      (sound == 1) PlaySound(assets.hit1);
        else if (sound == 2) PlaySound(assets.hit2);
        else if (sound == 3) PlaySound(assets.hit3);
    }
}

void SoundManager::play_shoot()
{
    PlaySound(assets.shoot1);
}

void SoundManager::play_die()
{
    PlaySound(assets.ship_death);
}

void SoundManager::play_engine()
{

}

void SoundManager::play_warning()
{

}

void SoundManager::play_pickup()
{
    PlaySound(assets.pickup);
}

void SoundManager::play_win_level()
{

}

void SoundManager::play_game_over()
{
    PlaySound(assets.game_over1);
}

void SoundManager::play_no_ammo()
{
    PlaySound(assets.no_ammo);
}

