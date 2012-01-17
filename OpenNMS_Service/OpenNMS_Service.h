/*++

(C) 2011 OpenNMS Group, Inc. All rights reserved.

Module Name:

   OpenNMS_Service.h

Abstract:

   Declarations, defines, etc. used by the service entry point.

Environment:

    User mode

--*/

#pragma once

//---------------------------------------------------------------------------------------
//
// Main service entry point
//
//---------------------------------------------------------------------------------------
VOID WINAPI ServiceMain(DWORD argc, TCHAR* argv[]);

//---------------------------------------------------------------------------------------
//
// Main service event handler routine
// - O/S will post service event messages to this routine
//
//---------------------------------------------------------------------------------------
VOID WINAPI HandlerProc(DWORD nCtrl);

//---------------------------------------------------------------------------------------
//
// Main window event handler routine
// - O/S will post window event messages to this routine
//
//---------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------------------
//
// Main window creation event
// - posted when main window is create
// - used for app initialization
//
//---------------------------------------------------------------------------------------
LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------------------
//
// Main window close event
// - posted when main window is closed
// - used for app shutdown
//
//---------------------------------------------------------------------------------------
LRESULT OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------------------
//
// Utility/worker routines
//
//---------------------------------------------------------------------------------------
void ControlOpenNMS(HWND hWnd, bool bIsStart);
