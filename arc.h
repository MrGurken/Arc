#if !defined(ARC_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>
#include <ctime>
#include "win32_spacecat.h"

#define DEBUGMSG( msg, ... ) { char buf[128] = {}; _snprintf( buf, 128, msg, __VA_ARGS__ ); OutputDebugStringA( buf ); }

struct UIObject
{
    Rect bounds;
    bool32_t hovered;
    bool32_t pressed;
};

#define SCREEN_MAX_NAME 32
#define SCREEN_MAX_OBJECTS 16
struct Screen
{
    char name[SCREEN_MAX_NAME];
    UIObject objects[SCREEN_MAX_OBJECTS];
};

struct Body
{
    Vec3 position;
    Vec3 velocity;
    Rect bounds;
    real32_t drag;
    bool32_t alive;
};

#define POWERUP_MAX 8
#define POWERUP_SIZE 16
#define POWERUP_HEALTH 0
#define POWERUP_SPEED 1
#define POWERUP_FIRERATE 2
struct Powerup
{
    Body body;
    int32_t type;
};

#define PROJECTILE_MAX 16
#define PROJECTILE_SIZE 8
#define PROJECTILE_PLAYER_OWNED 0
#define PROJECTILE_ENEMY_OWNED 1
struct Projectile
{
    Body body;
    int16_t lifetime;
    int16_t type;
};

#define ENEMY_MAX 16
#define ENEMY_COOLDOWN 32
#define ENEMY_DROP_RATE 20
struct Enemy
{
    Body body;
    int16_t health;
    int16_t cooldown;
};

#define PLAYER_WIDTH 32.0f
#define PLAYER_HEIGHT 32.0f
#define PLAYER_MAXSPEED 6.0f
#define PLAYER_DRAG 0.65f
#define PLAYER_MAX_HEALTH 3
struct Player
{
    Body body;
    int32_t cooldown;
    int32_t maxCooldown;
    int32_t health;
    real32_t speed;
};

#define GAME_WIDTH 640
#define GAME_HEIGHT 480
#define GAME_FPS 30
#define GAME_TIME(seconds) (seconds*GAME_FPS)
struct Gamestate
{
    Mat4 projection;
    Mat4 view;
    Shader shader;
    Texture texture;
    Player player;
    Mesh quad;
    Projectile projectiles[PROJECTILE_MAX];
    Enemy enemies[ENEMY_MAX];
    Powerup powerups[POWERUP_MAX];
    Screen mainScreen;
    Screen gameplayScreen;
    int curScreen;
    PlatformFont font;
};

#define ARC_H
#endif
