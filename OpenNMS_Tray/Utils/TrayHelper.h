/*++

(C) 2011 OpenNMS Group, Inc. All rights reserved.

Module Name:

   TrayHelper.cpp

Abstract:

   Declares utility routines used by OpenNMS_Tray.

Environment:

    User mode

--*/

#pragma once

//---------------------------------------------------------------------------------------
//
// provides language TLA for the current user
//
//---------------------------------------------------------------------------------------
void GetLangTLA(TCHAR szLang[], int nSize);

//---------------------------------------------------------------------------------------
//
// Load and release resource handle (for localized strings)
//
//---------------------------------------------------------------------------------------
HMODULE GetResourceHandle();
void FreeResourceHandle(HMODULE hMod);
