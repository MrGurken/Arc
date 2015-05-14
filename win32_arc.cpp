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

typedef int32_t bool32_t;
typedef float real32_t;
typedef double real64_t;

// SpaceCat DLL stuff
struct PlatformFile
{
    void* content;
    int32_t size;
};

typedef WNDCLASS Win32DefaultWindowClass_t( const char*, HINSTANCE );
typedef bool32_t Win32ReadFile_t( PlatformFile*, const char* );
typedef bool32_t Win32WriteFile_t( const char*, void*, int32_t );
typedef void Win32FreeFile_t( PlatformFile* );

struct SpaceCatDLL
{
    HMODULE module;
    WNDPROC Win32DefaultWindowProcedure;
    Win32DefaultWindowClass_t* Win32DefaultWindowClass;
    Win32ReadFile_t* Win32ReadFile;
    Win32WriteFile_t* Win32WriteFile;
    Win32FreeFile_t* Win32FreeFile;
};

static bool32_t SpaceCatLoad( SpaceCatDLL* dll )
{
    bool32_t result = false;
    
    dll->module = LoadLibrary( "spacecat.dll" );
    
    if( dll->module )
    {
        //Win32DefaultWindowProcedure = (WNDPROC)GetProcAddress( spacecatDLL, "Win32DefaultWindowProcedure" );
        //Win32DefaultWindowClass = (Win32DefaultWindowClass_t*)GetProcAddress( spacecatDLL, "Win32DefaultWindowClass" );

        dll->Win32DefaultWindowProcedure = (WNDPROC)GetProcAddress( dll->module, "Win32DefaultWindowProcedure" );
        dll->Win32DefaultWindowClass = (Win32DefaultWindowClass_t*)GetProcAddress( dll->module, "Win32DefaultWindowClass" );
        dll->Win32ReadFile = (Win32ReadFile_t*)GetProcAddress( dll->module, "Win32ReadFile" );
        dll->Win32WriteFile = (Win32WriteFile_t*)GetProcAddress( dll->module, "Win32WriteFile" );
        dll->Win32FreeFile = (Win32FreeFile_t*)GetProcAddress( dll->module, "Win32FreeFile" );

        if( dll->Win32DefaultWindowProcedure == 0 ||
            dll->Win32DefaultWindowClass == 0 ||
            dll->Win32ReadFile == 0 ||
            dll->Win32WriteFile == 0 ||
            dll->Win32FreeFile == 0 )
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
