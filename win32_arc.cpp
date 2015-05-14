/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Tunder $
   $Notice: (C) Copyright 2014 by SpaceCat, Inc. All Rights Reserved. $
   ======================================================================== */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>

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
struct PlatformFile
{
    void* content;
    int32_t size;
};

#define FONT_ITALIC 0x1
#define FONT_UNDERLINE 0x2
#define FONT_STRIKEOUT 0x4
struct PlatformFont
{
    //GLuint id;
    int32_t size;
    int32_t range;
    int32_t weight;
    int8_t style;
};

#define INPUT_MAX_KEYS 128
#define INPUT_MAX_BUTTONS 2
#define INPUT_LBUTTON 0
#define INPUT_RBUTTON 1
struct PlatformInput
{
    uint8_t keys[INPUT_MAX_KEYS];
    uint8_t prevKeys[INPUT_MAX_KEYS];
    uint8_t buttons[INPUT_MAX_BUTTONS];
    uint8_t prevButtons[INPUT_MAX_BUTTONS];
    //Vec2 mousePosition;
    //Vec2 mouseDelta;
    int32_t wheel;
    int32_t prevWheel;
};

typedef WNDCLASS Win32DefaultWindowClass_t( const char*, HINSTANCE );
typedef bool32_t Win32CreateRenderContext_t( HDC, HGLRC* );
typedef bool32_t Win32ReadFile_t( PlatformFile*, const char* );
typedef bool32_t Win32WriteFile_t( const char*, void*, int32_t );
typedef void Win32FreeFile_t( PlatformFile* );
typedef uint64_t Win32GetPerformanceFrequency_t();
typedef uint64_t Win32GetClock_t();
typedef uint64_t Win32GetLastWriteTime_t( const char* );
typedef real32_t Win32GetSecondsElapsed_t( uint64_t, uint64_t );
typedef bool32_t Win32ProcessKeyboard_t( PlatformInput*, MSG* );
typedef bool32_t Win32ProcessMouse_t( PlatformInput*, MSG* );
typedef bool32_t Win32ProcessInput_t( PlatformInput*, MSG* );

struct SpaceCatDLL
{
    HMODULE module;
    WNDPROC Win32DefaultWindowProcedure;
    Win32DefaultWindowClass_t* Win32DefaultWindowClass;
    Win32CreateRenderContext_t* Win32CreateRenderContext;
    Win32ReadFile_t* Win32ReadFile;
    Win32WriteFile_t* Win32WriteFile;
    Win32FreeFile_t* Win32FreeFile;
    Win32GetPerformanceFrequency_t* Win32GetPerformanceFrequency;
    Win32GetClock_t* Win32GetClock;
    Win32GetLastWriteTime_t* Win32GetLastWriteTime;
    Win32GetSecondsElapsed_t* Win32GetSecondsElapsed;
    Win32ProcessKeyboard_t* Win32ProcessKeyboard;
    Win32ProcessMouse_t* Win32ProcessMouse;
    Win32ProcessInput_t* Win32ProcessInput;
};

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
            dll->Win32GetPerformanceFrequency == 0 ||
            dll->Win32GetClock == 0 ||
            dll->Win32GetLastWriteTime == 0 ||
            dll->Win32GetSecondsElapsed == 0 ||
            dll->Win32ProcessKeyboard == 0 ||
            dll->Win32ProcessMouse == 0 ||
            dll->Win32ProcessInput == 0 )
        {
            OutputDebugStringA( "win32_arc.cpp: Failed to load some of the procedures of the spacecat module.\n" );
        }
        else
        {
            result = true;
        }
    }
    else
    {
        OutputDebugStringA( "win32_arc.cpp: Failed to load spacecat.dll.\n" );
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
            ShowWindow( windowHandle, SW_SHOW );

            dll.Win32GetPerformanceFrequency();

            int frames = 0;
            uint64_t fpstime = dll.Win32GetClock();

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
                uint64_t curtime = dll.Win32GetClock();
                if( dll.Win32GetSecondsElapsed( fpstime, curtime ) >= 1.0f )
                {
                    char buf[128] = {};
                    _snprintf( buf, 128, "FPS: %d\n", frames );
                    OutputDebugStringA( buf );

                    fpstime = curtime;
                    frames = 0;
                }
                else
                {
                    frames++;
                }

                // Render

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

                //SwapBuffers( deviceContext );
                lastCounter = dll.Win32GetClock();
            }
        }
        else
        {
            OutputDebugStringA( "win32_arc.cpp: Failed to create window.\n" );
        }
    }
    else
    {
        OutputDebugStringA( "win32_arc.cpp: Failed to register window class.\n" );
    }

    FreeLibrary( dll.module );

    return 0;
}
