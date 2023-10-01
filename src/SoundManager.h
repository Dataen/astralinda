
#pragma once

#include <Assets.h>

class SoundManager
{
public:
    SoundManager(Assets& assets);

    void play_hit(ProjectileType type);
    void play_shoot();
    void play_die();
    void play_engine();
    void play_warning();
    void play_pickup();
    void play_win_level();
    void play_game_over();
    void play_no_ammo();

private:
    Assets& assets;
};