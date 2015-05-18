/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

#include "arc.cpp"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    SpaceCat runtime = {};
    
    scDefaultRuntime( &runtime );
    runtime.fps = GAME_FPS;
    runtime.InitFunction = Init;
    runtime.UpdateFunction = Update;
    runtime.RenderFunction = Render;
    runtime.hInstance = hInstance;
    runtime.windowClassName = "ArcWindowClass";
    runtime.title = "Arc";
    runtime.windowWidth = GAME_WIDTH;
    runtime.windowHeight = GAME_HEIGHT;

    return scRun( &runtime );
}
