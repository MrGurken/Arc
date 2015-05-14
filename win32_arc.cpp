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

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    WNDCLASS windowClass = Win32DefaultWindowClass( WINDOW_CLASSNAME, hInstance );

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
            if( !Win32CreateRenderContext( deviceContext, &renderContext ) )
            {
                OutputDebugStringA( "win32_arc.cpp: Failed to create render context.\n" );
                MessageBoxA( 0, "Failed to create render context.", "win32_arc.cpp", MB_OK );
                return -1;
            }

            glewExperimental = GL_TRUE;
            GLenum glewOK = glewInit();
            if( glewOK != GLEW_OK )
            {
                OutputDebugStringA( "win32_arc.cpp: Failed to initialize GLEW.\n" );
                MessageBoxA( 0, "Failed to initialize GLEW.", "win32_arc.cpp", MB_OK );;
                return -1;
            }
            
            ShowWindow( windowHandle, SW_SHOW );

            Win32GetPerformanceFrequency();

            UINT desiredSchedulerMS = 1;
            bool32_t sleepIsGranular = ( timeBeginPeriod( desiredSchedulerMS ) == TIMERR_NOERROR );

            real32_t targetSecondsPerFrame = 1.0f / (real32_t)GAME_HZ;
            uint64_t lastCounter = Win32GetClock();

            PlatformInput input = {};

            const Vertex vertices[] =
            {
                MakeVertex( Vec3( 0.0f, 1.0f, 0.0f ) ),
                MakeVertex( Vec3( 1.0f, -1.0f, 0.0f ) ),
                MakeVertex( Vec3( -1.0f, -1.0f, 0.0f ) )
            };

            const GLuint indices[] =
            {
                0, 1, 2
            };
            
            Mesh mesh = {};
            MeshAddVertices( &mesh, vertices, 3, indices, 3 );

            Shader shader = {};
            ShaderLoad( &shader, "./shaders/basic.vs", 0, "./shaders/basic.fs" );
            
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
                        if( !Win32ProcessInput( &input, &message ) )
                        {
                            TranslateMessage( &message );
                            DispatchMessage( &message );
                        }
                    }
                }

                // Update
                if( input.keys[VK_ESCAPE] )
                    running = false;
                
                // Render
                glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
                glClear( GL_COLOR_BUFFER_BIT );

                glUseProgram( shader.program );
                MeshRender( &mesh );

                // Measure and adjust time
                uint64_t workCounter = Win32GetClock();
                real32_t workSecondsElapsed = Win32GetSecondsElapsed( lastCounter, workCounter );

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
                        secondsElapsed = Win32GetSecondsElapsed( lastCounter, Win32GetClock() );
                    }
                }

                SwapBuffers( deviceContext );
                lastCounter = Win32GetClock();
            }
        }
        else
        {
            OutputDebugStringA( "win32_arc.cpp: Failed to create window.\n" );
            MessageBoxA( 0, "Failed to create window.", "win32_arc.cpp", MB_OK );
        }
    }
    else
    {
        OutputDebugStringA( "win32_arc.cpp: Failed to register window class.\n" );
        MessageBoxA( 0, "Failed to register window class.", "win32_arc.cpp", MB_OK );
    }

    return 0;
}
