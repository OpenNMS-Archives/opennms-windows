/*++

(C) 2011 OpenNMS Group, Inc. All rights reserved.

Module Name:

   OpenNMS_Tray.h

Abstract:

   Declarations, defines, etc. used by the application entry point.

Environment:

    User mode

--*/

#pragma once

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
// Tray icon event
// - posted when user interacts with our tray icon
//
//---------------------------------------------------------------------------------------
LRESULT OnTray(HWND hWnd, WPARAM wParam, LPARAM lParam);

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
// Service control routines (start, stop, and query)
//
//---------------------------------------------------------------------------------------
void ControlService(bool bIsStop);
bool IsServiceRunning();

//---------------------------------------------------------------------------------------
//
// Utility/worker routines
//
//---------------------------------------------------------------------------------------
int DisplayMenu(HWND hWnd);
int DisplayAbout(HWND hWnd);

