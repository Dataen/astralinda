#ifndef LIMITEDSPACE_ASSETS_H
#define LIMITEDSPACE_ASSETS_H

#include <Level.h>

#include <raylib.h>

struct Assets
{
    RenderTexture2D screen {};

    Texture2D title {};
    Texture2D ships {};
    Texture2D projectiles {};
    Texture2D engine {};
    Texture2D stars {};
    Texture2D effects {};
    Texture2D warning {};
    Texture2D pickups {};

    Sound hit_laser1;
    Sound hit_laser2;
    Sound hit1;
    Sound hit2;
    Sound hit3;
    Sound shoot1;
    Sound game_over1;
    Sound ship_death;
    Sound no_ammo;
    Sound pickup;

    std::vector<Level> levels;
};

#endif //LIMITEDSPACE_ASSETS_H
