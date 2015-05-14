/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>
#include "w:\code\SpaceCat\win32_spacecat.h"

#define WINDOW_CLASSNAME "ArcWindowClassname"
#define WINDOW_TITLE "Arc"
#define WINDOW_X CW_USEDEFAULT
#define WINDOW_Y CW_USEDEFAULT
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MONITOR_HZ 60
#define GAME_HZ (MONITOR_HZ/2)

typedef int32_t bool32_t;
typedef float real32_t;
typedef double real64_t;

// SpaceCat DLL stuff
static bool32_t SpaceCatLoad( SpaceCatDLL* dll )
{
    bool32_t result = false;
    
    dll->module = LoadLibrary( "spacecat.dll" );
    
    if( dll->module )
    {
        dll->Win32DefaultWindowProcedure = (WNDPROC)GetProcAddress( dll->module, "Win32DefaultWindowProcedure" );
        dll->Win32DefaultWindowClass = (Win32DefaultWindowClass_t*)GetProcAddress( dll->module, "Win32DefaultWindowClass" );
        dll->Win32CreateRenderContext = (Win32CreateRenderContext_t*)GetProcAddress( dll->module, "Win32CreateRenderContext" );
        dll->Win32ReadFile = (Win32ReadFile_t*)GetProcAddress( dll->module, "Win32ReadFile" );
        dll->Win32WriteFile = (Win32WriteFile_t*)GetProcAddress( dll->module, "Win32WriteFile" );
        dll->Win32FreeFile = (Win32FreeFile_t*)GetProcAddress( dll->module, "Win32FreeFile" );
        dll->Win32ReadFont = (Win32ReadFont_t*)GetProcAddress( dll->module, "Win32ReadFont" );
        dll->Win32GetPerformanceFrequency = (Win32GetPerformanceFrequency_t*)GetProcAddress( dll->module, "Win32GetPerformanceFrequency" );
        dll->Win32GetClock = (Win32GetClock_t*)GetProcAddress( dll->module, "Win32GetClock" );
        dll->Win32GetLastWriteTime = (Win32GetLastWriteTime_t*)GetProcAddress( dll->module, "Win32GetLastWriteTime" );
        dll->Win32GetSecondsElapsed = (Win32GetSecondsElapsed_t*)GetProcAddress( dll->module, "Win32GetSecondsElapsed" );
        dll->Win32ProcessKeyboard = (Win32ProcessKeyboard_t*)GetProcAddress( dll->module, "Win32ProcessKeyboard" );
        dll->Win32ProcessMouse = (Win32ProcessMouse_t*)GetProcAddress( dll->module, "Win32ProcessMouse" );
        dll->Win32ProcessInput = (Win32ProcessInput_t*)GetProcAddress( dll->module, "Win32ProcessInput" );

        if( dll->Win32DefaultWindowProcedure == 0 ||
            dll->Win32DefaultWindowClass == 0 ||
            dll->Win32CreateRenderContext == 0 ||
            dll->Win32ReadFile == 0 ||
            dll->Win32WriteFile == 0 ||
            dll->Win32FreeFile == 0 ||
            dll->Win32ReadFont == 0 ||
            dll->Win32GetPerformanceFrequency == 0 ||
            dll->Win32GetClock == 0 ||
            dll->Win32GetLastWriteTime == 0 ||
            dll->Win32GetSecondsElapsed == 0 ||
            dll->Win32ProcessKeyboard == 0 ||
            dll->Win32ProcessMouse == 0 ||
            dll->Win32ProcessInput == 0 )
        {
            OutputDebugStringA( "win32_arc.cpp: Failed to load some of the procedures of the spacecat module.\n" );
            MessageBoxA( 0, "win32_arc.cpp", "Failed to load some of the procedures of the spacecat module.", MB_OK );
        }
        else
        {
            result = true;
        }
    }
    else
    {
        OutputDebugStringA( "win32_arc.cpp: Failed to load spacecat.dll.\n" );
        MessageBoxA( 0, "win32_arc.cpp", "Failed to load spacecat.dll.", MB_OK );
    }

    return result;
}

static void SpaceCatUnload( SpaceCatDLL* dll )
{
    FreeLibrary( dll->module );
    // TODO: Set function pointers to 0 ?
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    SpaceCatDLL dll = {};
    SpaceCatLoad( &dll );
    
    WNDCLASS windowClass = dll.Win32DefaultWindowClass( WINDOW_CLASSNAME, hInstance );

    if( RegisterClass( &windowClass ) )
    {
        HWND windowHandle = CreateWindow( windowClass.lpszClassName,
                                          WINDOW_TITLE,
                                          WS_OVERLAPPEDWINDOW,
                                          WINDOW_X, WINDOW_Y,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          0, 0, hInstance, 0 );

        if( windowHandle )
        {
            HDC deviceContext = GetDC( windowHandle );
            HGLRC renderContext = 0;
            if( !dll.Win32CreateRenderContext( deviceContext, &renderContext ) )
            {
                OutputDebugStringA( "win32_arc.cpp: Failed to create render context.\n" );
                MessageBoxA( 0, "win32_arc.cpp", "Failed to create render context.", MB_OK );
                return -1;
            }

            glewExperimental = GL_TRUE;
            GLenum glewOK = glewInit();
            if( glewOK != GLEW_OK )
            {
                OutputDebugStringA( "win32_arc.cpp: Failed to initialize GLEW.\n" );
                MessageBoxA( 0, "win32_arc.cpp", "Failed to initialize GLEW.", MB_OK );
                return -1;
            }
            
            ShowWindow( windowHandle, SW_SHOW );

            dll.Win32GetPerformanceFrequency();

            UINT desiredSchedulerMS = 1;
            bool32_t sleepIsGranular = ( timeBeginPeriod( desiredSchedulerMS ) == TIMERR_NOERROR );

            real32_t targetSecondsPerFrame = 1.0f / (real32_t)GAME_HZ;
            uint64_t lastCounter = dll.Win32GetClock();
            
            bool32_t running = true;
            while( running )
            {
                MSG message;
                while( PeekMessage( &message, 0, 0, 0, PM_REMOVE ) )
                {
                    if( message.message == WM_QUIT )
                    {
                        running = false;
                    }
                    else
                    {
                        TranslateMessage( &message );
                        DispatchMessage( &message );
                    }
                }

                // Update
                
                // Render
                glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
                glClear( GL_COLOR_BUFFER_BIT );

                // Measure and adjust time
                uint64_t workCounter = dll.Win32GetClock();
                real32_t workSecondsElapsed = dll.Win32GetSecondsElapsed( lastCounter, workCounter );

                real32_t secondsElapsed = workSecondsElapsed;
                while( secondsElapsed < targetSecondsPerFrame )
                {
                    if( sleepIsGranular )
                    {
                        DWORD sleepMS = (DWORD)( 1000.0f * ( targetSecondsPerFrame - secondsElapsed ) );

                        if( sleepMS > 0 )
                        {
                            Sleep( sleepMS );
                        }
                    }

                    // busy wait
                    while( secondsElapsed < targetSecondsPerFrame )
                    {
                        secondsElapsed = dll.Win32GetSecondsElapsed( lastCounter, dll.Win32GetClock() );
                    }
                }

                SwapBuffers( deviceContext );
                lastCounter = dll.Win32GetClock();
            }
        }
        else
        {
            OutputDebugStringA( "win32_arc.cpp: Failed to create window.\n" );
            MessageBoxA( 0, "win32_arc.cpp", "Failed to create window.", MB_OK );
        }
    }
    else
    {
        OutputDebugStringA( "win32_arc.cpp: Failed to register window class.\n" );
        MessageBoxA( 0, "win32_arc.cpp", "Failed to register window class.", MB_OK );
    }

    FreeLibrary( dll.module );

    return 0;
}
