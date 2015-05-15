/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>
#include "win32_spacecat.h"

#define WINDOW_CLASSNAME "ArcWindowClassname"
#define WINDOW_TITLE "Arc"
#define WINDOW_X CW_USEDEFAULT
#define WINDOW_Y CW_USEDEFAULT
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MONITOR_HZ 60
#define GAME_HZ (MONITOR_HZ/2)

bool32_t Update( PlatformIO* io, PlatformInput* input )
{
    if( input->keys[VK_ESCAPE] )
        return false;
    
    return true;
}

void Render()
{
    glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    SpaceCat runtime = {};
    
    scDefaultRuntime( &runtime );
    runtime.fps = GAME_HZ;
    runtime.UpdateFunction = Update;
    runtime.RenderFunction = Render;
    runtime.hInstance = hInstance;
    runtime.windowClassName = "ArcWindowClass";
    runtime.title = "Arc";

    return scRun( &runtime );
}
