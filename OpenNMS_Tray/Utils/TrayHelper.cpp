/*++

(C) 2011 OpenNMS Group, Inc. All rights reserved.

Module Name:

   TrayHelper.cpp

Abstract:

   Defines utility routines used by OpenNMS_Tray.

Environment:

    User mode

--*/

#include "stdafx.h"
#include "resource.h"
#include "TrayHelper.h"

//---------------------------------------------------------------------------------------
void GetLangTLA(TCHAR szLang[], int nSize)
{
   short nLangID = GetUserDefaultLangID();
   short nPrimaryLangID = PRIMARYLANGID(nLangID);

   if(nPrimaryLangID == LANG_ENGLISH)
      _tcsncpy_s(szLang, nSize, _T("ENU"), _TRUNCATE);
   else
      _tcsncpy_s(szLang, nSize, _T("ENU"), _TRUNCATE);
}

//---------------------------------------------------------------------------------------
HMODULE GetResourceHandle()
{
   TCHAR szPath[MAX_PATH] = _T("");
   GetModuleFileName(NULL, szPath, MAX_PATH);

   LPTSTR pszCur = _tcsrchr(szPath, _T('\\'));

   if(pszCur)
      *pszCur = NULL;

   pszCur = _tcsrchr(szPath, _T('\\'));

   if(pszCur)
      *pszCur = NULL;

   _tcsncat_s(szPath, MAX_PATH - _tcslen(szPath), _T("\\lib\\Mainr"), _TRUNCATE);

   TCHAR szLang[MAX_PATH] = _T("");
   GetLangTLA(szLang, MAX_PATH);

   _tcsncat_s(szPath, MAX_PATH - _tcslen(szPath), szLang, _TRUNCATE);
   _tcsncat_s(szPath, MAX_PATH - _tcslen(szPath), _T(".dll"), _TRUNCATE);

   return LoadLibrary(szPath);
}

//---------------------------------------------------------------------------------------
void FreeResourceHandle(HMODULE hMod)
{
   FreeLibrary(hMod);
   hMod = NULL;
}
