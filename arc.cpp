/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

#include "arc.h"

void BodyUpdate( Body* body )
{
    if( body->alive )
    {
        body->position += body->velocity;
        body->bounds.TopLeft( Vec2( body->position.x, body->position.y ) );
        body->velocity *= body->drag;
        
        // NOTE: We don't want to check if position.y < 0 because objects in the
        // world often spawn 'above' the screen
        if( body->position.x < -body->bounds.width ||
            body->position.x > GAME_WIDTH ||
            body->position.y > GAME_HEIGHT )
        {
            body->alive = false;
        }
    }
}

void PowerupSetup( Powerup* powerups )
{
    for( int i=0; i<POWERUP_MAX; i++ )
    {
        powerups[i].body.bounds = Rect( 0, 0, POWERUP_SIZE, POWERUP_SIZE );
        powerups[i].body.drag = 1.0f; // no drag
        powerups[i].body.alive = false;
    }
}

void PowerupSpawn( Powerup* powerups, int32_t type,
                   Vec3 position, Vec3 velocity )
{
    for( int i=0; i<POWERUP_MAX; i++ )
    {
        if( !powerups[i].body.alive )
        {
            powerups[i].body.position = position;
            powerups[i].body.velocity = velocity;
            powerups[i].body.alive = true;
            powerups[i].type = type;
            break;
        }
    }
}

void PowerupUpdate( Powerup* powerup )
{
    BodyUpdate( &powerup->body );
}

void ProjectileSetup( Projectile* projectiles )
{
    for( int i=0; i<PROJECTILE_MAX; i++ )
    {
        projectiles[i].body.bounds = Rect( 0, 0, PROJECTILE_SIZE, PROJECTILE_SIZE );
        projectiles[i].lifetime = 0;
    }
}
    
void ProjectileSpawn( Projectile* projectiles, int16_t type,
                      Vec3 position, Vec3 velocity,
                      int32_t lifetime, real32_t drag = 1.0f )
{
    for( int i=0; i<PROJECTILE_MAX; i++ )
    {
        if( projectiles[i].lifetime <= 0 )
        {
            projectiles[i].body.position = position;
            projectiles[i].body.velocity = velocity;
            projectiles[i].body.drag = drag;
            projectiles[i].body.alive = true;
            projectiles[i].lifetime = lifetime;
            projectiles[i].type = type;
            break;
        }
    }
}

void ProjectileUpdate( Projectile* projectile )
{
    if( projectile->lifetime <= 0 )
        projectile->body.alive = false;
    
    BodyUpdate( &projectile->body );
}

void EnemySetup( Enemy* enemies )
{
    for( int i=0; i<ENEMY_MAX; i++ )
    {
        enemies[i].body.bounds = Rect( 0, 0, 32, 32 );
        enemies[i].body.drag = 1.0f; // no drag
        enemies[i].health = 0;
    }
}

void EnemySpawn( Enemy* enemies, Vec3 position, Vec3 velocity )
{
    for( int i=0; i<ENEMY_MAX; i++ )
    {
        if( !enemies[i].body.alive)
        {
            enemies[i].body.position = position;
            enemies[i].body.velocity = velocity;
            enemies[i].body.alive = true;
            enemies[i].health = 2;
            break;
        }
    }
}

void EnemyUpdate( Enemy* enemy, Projectile* projectiles, Powerup* powerups )
{
    if( enemy->health <= 0 )
        enemy->body.alive = false;
    
    if( enemy->body.alive )
    {
        BodyUpdate( &enemy->body );
        
        // Collision against player projectiles
        for( int i=0; i<PROJECTILE_MAX; i++ )
        {
            if( projectiles[i].body.alive &&
                projectiles[i].type == PROJECTILE_PLAYER_OWNED &&
                enemy->body.bounds.Intersect( projectiles[i].body.bounds ) )
            {
                enemy->health--;
                //projectiles[i].lifetime = 0;
                projectiles[i].body.alive = false;
            }
        }

        // Check if we should drop a powerup
        if( enemy->health <= 0 )
        {
            if( rand() % 100 < ENEMY_DROP_RATE )
            {
                PowerupSpawn( powerups, POWERUP_HEALTH,
                              enemy->body.position, enemy->body.velocity );
            }

            enemy->body.alive = false;
        }
        else // if we're still alive, check to see if we should shoot
        {
            if( enemy->cooldown > 0 )
                enemy->cooldown--;
            else
            {
                Vec3 center = enemy->body.position;
                center.x += enemy->body.bounds.width * 0.5f - PROJECTILE_SIZE*0.5f;
                ProjectileSpawn( projectiles, PROJECTILE_ENEMY_OWNED, center,
                                 Vec3( 0.0f, 12.0f, 0 ), 60 );
                    
                enemy->cooldown = ENEMY_COOLDOWN;
            }
        }
    }
}

void PlayerSetup( Player* player )
{
    player->body.position = Vec3( GAME_WIDTH*0.5f-PLAYER_WIDTH*0.5f,
                                  GAME_HEIGHT - PLAYER_HEIGHT - 8.0f,
                                  0.0f );
    player->body.bounds = Rect( 0, 0, PLAYER_WIDTH, PLAYER_HEIGHT );
    player->body.drag = PLAYER_DRAG;
    player->body.alive = true;
    player->cooldown = 0;
    player->health = PLAYER_MAX_HEALTH;
    player->speed = 2.5f;
    player->maxCooldown = 10;
}

void PlayerUpdate( Player* player, Projectile* projectiles,
                   Powerup* powerups, PlatformInput* input )
{
    if( player->health <= 0 )
    {
        player->body.alive = false;
        return;
    }
    
    // Movement
    // TODO: VK_LEFT is not cross platform
    Vec3 movement;
    if( input->keys[VK_LEFT] )
        movement.x -= 1.0f;
    if( input->keys[VK_RIGHT] )
        movement.x += 1.0f;
    if( input->keys[VK_UP] )
        movement.y -= 1.0f;
    if( input->keys[VK_DOWN] )
        movement.y += 1.0f;

    movement.Normalize();
    player->body.velocity += movement * player->speed;
    if( player->body.velocity.Length() > PLAYER_MAXSPEED )
    {
        player->body.velocity.Normalize();
        player->body.velocity *= PLAYER_MAXSPEED;
    }

    BodyUpdate( &player->body );

    // Shooting
    if( player->cooldown > 0 )
    {
        player->cooldown--;
    }
    else
    {
        // TODO: VK_SPACE is not cross platform
        if( KeyDown( input, VK_SPACE ) )
        {
            Vec3 center = player->body.position;
            center.x += PLAYER_WIDTH*0.5f;
            center.x -= PROJECTILE_SIZE*0.5f;
            
            ProjectileSpawn( projectiles, PROJECTILE_PLAYER_OWNED, center,
                             Vec3( 0.0f, -12.0f, 0.0f ), 60 );
            player->cooldown = player->maxCooldown;
        }
    }

    // Collision with enemy bullets
    for( int i=0; i<PROJECTILE_MAX; i++ )
    {
        if( projectiles[i].body.alive &&
            projectiles[i].type == PROJECTILE_ENEMY_OWNED &&
            player->body.bounds.Intersect( projectiles[i].body.bounds ) )
        {
            player->health--;
            //projectiles[i].lifetime = 0;
            projectiles[i].body.alive = false;
        }
    }

    // Collision with powerups
    for( int i=0; i<POWERUP_MAX; i++ )
    {
        if( powerups[i].body.alive &&
            player->body.bounds.Intersect( powerups[i].body.bounds ) )
        {
            switch( powerups[i].type )
            {
                case POWERUP_HEALTH: player->health++; break;
                case POWERUP_SPEED: player->speed += 0.5f; break;
                case POWERUP_FIRERATE: player->maxCooldown -= 2; break;
            }

            powerups[i].body.alive = false;
        }
    }
}

void SetWorldMatrix( Body* body, GLuint location )
{
    Mat4 worldMatrix = ( Mat4::MakeTranslation( body->position ) *
                         Mat4::MakeScale( body->bounds.width, body->bounds.height, 0.0f ) );
    glUniformMatrix4fv( location, 1, GL_TRUE, worldMatrix.m );
}

FUNCTION_INIT( Init )
{
    bool32_t result = true;

    srand( (uint32_t)time(0) );
    
    Gamestate* gamestate = (Gamestate*)memory->pointer;

    DEBUGMSG( "Memory allocated: %d\nMemory used: %d\n", memory->size, sizeof(Gamestate) );

    gamestate->projection = Mat4::MakeOrthographic( 0.0f, GAME_WIDTH, 0, GAME_HEIGHT, -1.0f, 1.0f );
    gamestate->view = Mat4::MakeTranslation( 0.0f, 0.0f, 0.0f );

    if( ShaderLoad( &gamestate->shader, "./shaders/basic_tex.vs", 0, "./shaders/basic_tex.fs" ) )
    {
        result = ShaderGetUniform( &gamestate->shader, "ProjectionMatrix" ) &&
            ShaderGetUniform( &gamestate->shader, "ViewMatrix" ) &&
            ShaderGetUniform( &gamestate->shader, "WorldMatrix" ) &&
            ShaderGetUniform( &gamestate->shader, "Color" );

        result = result && TextureLoad( io, &gamestate->texture, "./textures/checkers_blue.bmp" );

        if( !result )
            MessageBoxA( 0, "Failed to get uniforms.", "arc.cpp", MB_OK );
    }
    else
    {
        result = false;
        MessageBoxA( 0, "Failed to load shader.", "arc.cpp", MB_OK );
    }
    
    MeshQuad( &gamestate->quad );
    PlayerSetup( &gamestate->player );
    EnemySetup( gamestate->enemies );
    ProjectileSetup( gamestate->projectiles );
    PowerupSetup( gamestate->powerups );

    strcpy( gamestate->mainScreen.name, "Main Menu" );
    gamestate->mainScreen.objects[0].bounds = Rect( 32, 32, 128, 24 );
    
    strcpy( gamestate->gameplayScreen.name, "Gameplay" );

    gamestate->font.size = 24;
    result = result && io->ReadFont( &gamestate->font, "Verdana" );
    
    return result;
}

FUNCTION_UPDATE( Update )
{
    Gamestate* gamestate = (Gamestate*)memory->pointer;

    if( gamestate->curScreen == 0 )
    {
        if( input->keys[VK_ESCAPE] )
            return false;
        
        if( input->keys[VK_RETURN] )
            gamestate->curScreen = 1;

        gamestate->mainScreen.objects[0].hovered = false;
        gamestate->mainScreen.objects[0].pressed = false;
        
        if( gamestate->mainScreen.objects[0].bounds.Contains( input->mousePosition ) )
        {
            gamestate->mainScreen.objects[0].hovered = true;

            if( input->buttons[INPUT_LBUTTON] )
                gamestate->mainScreen.objects[0].pressed = true;
        }
    }
    else
    {
        // TODO: VK_ESCAPE is not cross platform
        if( input->keys[VK_ESCAPE] )
            return false;

        if( KeyPressed( input, 'S' ) )
            EnemySpawn( gamestate->enemies, Vec3( GAME_WIDTH*0.5f-16.0f, -32.0f, 0 ), Vec3( 0, 3, 0 ) );

        PlayerUpdate( &gamestate->player, gamestate->projectiles,
                      gamestate->powerups, input );

        for( int i=0; i<POWERUP_MAX; i++ )
            PowerupUpdate( &gamestate->powerups[i] );
    
        for( int i=0; i<PROJECTILE_MAX; i++ )
            ProjectileUpdate( &gamestate->projectiles[i] );

        for( int i=0; i<ENEMY_MAX; i++ )
            EnemyUpdate( &gamestate->enemies[i], gamestate->projectiles, gamestate->powerups );
    }
    
    return true;
}

FUNCTION_RENDER( Render )
{
    Gamestate* gamestate = (Gamestate*)memory->pointer;

    if( gamestate->curScreen == 0 )
    {
        glClearColor( 0.0f, 1.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        glUseProgram( gamestate->shader.program );
        glBindTexture( GL_TEXTURE_2D, gamestate->texture.id );
        glUniformMatrix4fv( gamestate->shader.uniforms[0], 1, GL_TRUE, gamestate->projection.m );
        glUniformMatrix4fv( gamestate->shader.uniforms[1], 1, GL_TRUE, gamestate->view.m );

        UIObject* obj = &gamestate->mainScreen.objects[0];
        Mat4 world = ( Mat4::MakeTranslation( obj->bounds.x, obj->bounds.y, 0.0f ) *
                       Mat4::MakeScale( obj->bounds.width, obj->bounds.height, 0.0f ) );
        glUniformMatrix4fv( gamestate->shader.uniforms[2], 1, GL_TRUE, world.m );

        Vec3 clr;
        if( obj->pressed )
            clr = Vec3( 1.0f, 0.0f, 0.0f );
        else if( obj->hovered )
            clr = Vec3( 0.0f, 1.0f, 0.0f );
        else
            clr = Vec3( 1.0f, 1.0f, 1.0f );
        
        glUniform3f( gamestate->shader.uniforms[3], clr.x, clr.y, clr.z );
        
        MeshRender( &gamestate->quad );

        glUseProgram( 0 );
        TextRender( &gamestate->font, obj->bounds.TopLeft(), "Testing...", Point2( GAME_WIDTH, GAME_HEIGHT ) );
    }
    else
    {
        glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        glUseProgram( gamestate->shader.program );
        glBindTexture( GL_TEXTURE_2D, gamestate->texture.id );
        glUniformMatrix4fv( gamestate->shader.uniforms[0], 1, GL_TRUE, gamestate->projection.m );
        glUniformMatrix4fv( gamestate->shader.uniforms[1], 1, GL_TRUE, gamestate->view.m );
        glUniform3f( gamestate->shader.uniforms[3], 1.0f, 1.0f, 1.0f );

        if( gamestate->player.body.alive )
        {
            SetWorldMatrix( &gamestate->player.body,
                            gamestate->shader.uniforms[2] );
            MeshRender( &gamestate->quad );
        }

        for( int i=0; i<POWERUP_MAX; i++ )
        {
            if( gamestate->powerups[i].body.alive )
            {
                SetWorldMatrix( &gamestate->powerups[i].body,
                                gamestate->shader.uniforms[2] );
                MeshRender( &gamestate->quad );
            }
        }

        for( int i=0; i<PROJECTILE_MAX; i++ )
        {
            if( gamestate->projectiles[i].body.alive )
            {
                SetWorldMatrix( &gamestate->projectiles[i].body,
                                gamestate->shader.uniforms[2] );
                MeshRender( &gamestate->quad );
            }
        }

        for( int i=0; i<ENEMY_MAX; i++ )
        {
            if( gamestate->enemies[i].body.alive )
            {
                SetWorldMatrix( &gamestate->enemies[i].body,
                                gamestate->shader.uniforms[2] );
                MeshRender( &gamestate->quad );
            }
        }
    }
}
