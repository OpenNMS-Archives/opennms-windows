/*++

(C) 2011 OpenNMS Group, Inc. All rights reserved.

Module Name:

   LaunchNMS.cpp

Abstract:

   Defines the launcher application entry point.

Environment:

    User mode

--*/

#include "stdafx.h"
#include "LaunchNMS.h"

int APIENTRY _tWinMain(HINSTANCE hInst,
                       HINSTANCE hPrevInst,
                       LPTSTR lpCmdLine,
                       int nCmdShow)
{
   TCHAR szPath[MAX_PATH] = _T("");
   GetModuleFileName(NULL, szPath, MAX_PATH);
   HKEY hKey = NULL;

   if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   _T("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                   0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
   {
      RegSetValueEx(hKey, _T("Launch OpenNMS"), 0,
                    REG_SZ, (LPBYTE)szPath,
                    DWORD(_tcslen(szPath) * sizeof(TCHAR)));

      RegCloseKey(hKey);
   }

   LPTSTR pszCur = _tcsrchr(szPath, _T('\\'));

   if(pszCur)
      *pszCur = NULL;

   TCHAR szFile[MAX_PATH] = _T("");
   _tcsncpy_s(szFile, MAX_PATH, szPath, _TRUNCATE);
   _tcsncat_s(szFile, MAX_PATH - _tcslen(szFile), _T("\\OpenNMS_Tray.exe"), _TRUNCATE);

   TCHAR szVerb[MAX_PATH] = _T("");

   OSVERSIONINFO VerInfo = {0};
   VerInfo.dwOSVersionInfoSize = sizeof(VerInfo);
   GetVersionEx(&VerInfo);

   if(VerInfo.dwMajorVersion > 5)
      _tcsncpy_s(szVerb, MAX_PATH, _T("runas"), _TRUNCATE);
   else
      _tcsncpy_s(szVerb, MAX_PATH, _T("open"), _TRUNCATE);

   ShellExecute(NULL, szVerb, szFile, NULL, szPath, SW_SHOWDEFAULT);
   return 0;
}
