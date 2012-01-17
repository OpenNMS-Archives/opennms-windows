/*++

(C) 2011 OpenNMS Group, Inc. All rights reserved.

Module Name:

   OpenNMS_Service.cpp

Abstract:

   Defines the service entry point.

Environment:

    User mode

--*/

#include "stdafx.h"
#include "resource.h"
#include "SvcHelper.h"
#include "OpenNMS_Service.h"

static const int SERVICE_TIMEOUT = 10000;
static const LPTSTR STR_SVC_NAME = _T("OpenNMS_Service");

static SERVICE_STATUS g_SvcStat = {0};
static SERVICE_STATUS_HANDLE g_hSvcStat = NULL;

//---------------------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                       LPTSTR lpCmdLine, int nCmdShow)
{
   if(!_tcsicmp(lpCmdLine, _T("/install")))
   {
      SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
                                     SC_MANAGER_ALL_ACCESS);

      if(hSCM)
      {
         TCHAR szName[MAX_PATH] = _T("");
         TCHAR szDesc[MAX_PATH] = _T("");

         HMODULE hRes = GetResourceHandle();

         if(hRes)
         {
            LoadString(hRes, IDS_SVC_NAME, szName, MAX_PATH);
            LoadString(hRes, IDS_SVC_DESC, szDesc, MAX_PATH);

            FreeResourceHandle(hRes);
         }

         TCHAR szPath[MAX_PATH] = _T("");
         GetModuleFileName(NULL, szPath, MAX_PATH);

         SC_HANDLE hSVC = CreateService(hSCM, STR_SVC_NAME,
                                        szName, SERVICE_ALL_ACCESS,
                                        SERVICE_WIN32_OWN_PROCESS,
                                        SERVICE_AUTO_START,
                                        SERVICE_ERROR_NORMAL,
                                        szPath, NULL, NULL,
                                        NULL, NULL, NULL);

         if(hSVC)
         {
            SERVICE_DESCRIPTION sd = {0};
            sd.lpDescription = szDesc;
            ChangeServiceConfig2(hSVC, SERVICE_CONFIG_DESCRIPTION, &sd);

            CloseServiceHandle(hSVC);
         }

         CloseServiceHandle(hSCM);
      }
   }
   else if(!_tcsicmp(lpCmdLine, _T("/uninstall")))
   {
      SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
                                     SC_MANAGER_ALL_ACCESS);

      if(hSCM)
      {
         SC_HANDLE hSVC = OpenService(hSCM, STR_SVC_NAME, SERVICE_ALL_ACCESS);

         if(hSVC)
         {
            ControlService(hSVC, SERVICE_CONTROL_STOP, NULL);
            DeleteService(hSVC);
            CloseServiceHandle(hSVC);
         }

         CloseServiceHandle(hSCM);
      }
   }
   else
   {
      SERVICE_TABLE_ENTRY SvcTblEntry[2] = {0};
      SvcTblEntry[0].lpServiceName = STR_SVC_NAME;
      SvcTblEntry[0].lpServiceProc = ServiceMain;

      StartServiceCtrlDispatcher(SvcTblEntry);
   }

   return 0;
}

//---------------------------------------------------------------------------------------
VOID WINAPI ServiceMain(DWORD argc, TCHAR* argv[])
{
   g_SvcStat.dwCheckPoint = 0;
   g_SvcStat.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
   g_SvcStat.dwCurrentState = SERVICE_START_PENDING;
   g_SvcStat.dwServiceSpecificExitCode = NO_ERROR;
   g_SvcStat.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
   g_SvcStat.dwWaitHint = SERVICE_TIMEOUT;
   g_SvcStat.dwWin32ExitCode = NO_ERROR;
   SetServiceStatus(g_hSvcStat, &g_SvcStat);

   g_hSvcStat = RegisterServiceCtrlHandler(STR_SVC_NAME, HandlerProc);
   HWND hPrevSvc = FindWindow(STR_SVC_NAME, 0);

   if(!hPrevSvc)
   {
      WNDCLASS WndClass = {0};
      WndClass.lpfnWndProc = WndProc;
      WndClass.hInstance = GetModuleHandle(NULL);
      WndClass.lpszClassName = STR_SVC_NAME;

      if(RegisterClass(&WndClass))
      {
         HWND hWnd = CreateWindow(STR_SVC_NAME, 0, 0, 0, 0, 0, 0,
                                  0, 0, WndClass.hInstance, 0);

         if(hWnd)
         {
            g_SvcStat.dwWaitHint = 0;
            g_SvcStat.dwCurrentState = SERVICE_RUNNING;
            SetServiceStatus(g_hSvcStat, &g_SvcStat);

            MSG msg = {0};

            while(GetMessage(&msg, NULL, 0, 0) > 0)
            { 
               TranslateMessage(&msg); 
               DispatchMessage(&msg); 
            }
         }

         UnregisterClass(STR_SVC_NAME, WndClass.hInstance);
      }
   }

   g_SvcStat.dwCurrentState = SERVICE_STOPPED;
   SetServiceStatus(g_hSvcStat, &g_SvcStat);
}

//---------------------------------------------------------------------------------------
VOID WINAPI HandlerProc(DWORD nCtrl)
{
   switch(nCtrl)
   {
   case SERVICE_CONTROL_STOP:
   case SERVICE_CONTROL_SHUTDOWN:
      g_SvcStat.dwWaitHint = SERVICE_TIMEOUT;
      g_SvcStat.dwCurrentState = SERVICE_STOP_PENDING;

      SetServiceStatus(g_hSvcStat, &g_SvcStat);
      PostMessage(FindWindow(STR_SVC_NAME, 0), WM_CLOSE, 0, 0);
      break;

   case SERVICE_CONTROL_INTERROGATE:
      SetServiceStatus(g_hSvcStat, &g_SvcStat);
      break;
   }
}

//---------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
   LRESULT lr = 0;

   switch (nMsg)
	{
   case WM_CREATE:
      lr = OnCreate(hWnd, wParam, lParam);
      break;

   case WM_CLOSE:
      lr = OnClose(hWnd, wParam, lParam);
      break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

   default:
		lr = DefWindowProc(hWnd, nMsg, wParam, lParam);
      break;
	}

   return lr;
}

//---------------------------------------------------------------------------------------
LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   ControlOpenNMS(hWnd, true);
   return 0;
}

//---------------------------------------------------------------------------------------
LRESULT OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   ControlOpenNMS(hWnd, false);
	DestroyWindow(hWnd);

   return 0;
}

//---------------------------------------------------------------------------------------
void ControlOpenNMS(HWND hWnd, bool bIsStart)
{
   TCHAR szVerb[MAX_PATH] = _T("");

   OSVERSIONINFO VerInfo = {0};
   VerInfo.dwOSVersionInfoSize = sizeof(VerInfo);
   GetVersionEx(&VerInfo);

   if(VerInfo.dwMajorVersion > 5)
      _tcsncpy_s(szVerb, MAX_PATH, _T("runas"), _TRUNCATE);
   else
      _tcsncpy_s(szVerb, MAX_PATH, _T("open"), _TRUNCATE);

   HKEY hKey = NULL;
   TCHAR szVersion[MAX_PATH] = _T("");

   if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   _T("Software\\JavaSoft\\Java Development Kit"),
                   0, KEY_READ, &hKey) == ERROR_SUCCESS)
   {
      DWORD dwSize = sizeof(szVersion);
      RegQueryValueEx(hKey, _T("CurrentVersion"), NULL,
                      NULL, (LPBYTE)szVersion, &dwSize);

      RegCloseKey(hKey);
      hKey = NULL;
   }

   TCHAR szRegKey[MAX_PATH] = _T("Software\\JavaSoft\\Java Development Kit\\");
   _tcsncat_s(szRegKey, MAX_PATH - _tcslen(szRegKey), szVersion, _TRUNCATE);

   TCHAR szFile[MAX_PATH] = _T("");

   if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   szRegKey, 0, KEY_READ,
                   &hKey) == ERROR_SUCCESS)
   {
      DWORD dwSize = sizeof(szFile);
      RegQueryValueEx(hKey, _T("JavaHome"), NULL,
                      NULL, (LPBYTE)szFile, &dwSize);

      RegCloseKey(hKey);
   }

   _tcsncat_s(szFile, MAX_PATH - _tcslen(szFile), _T("\\bin\\java.exe"), _TRUNCATE);

   TCHAR szFolder[MAX_PATH] = _T("");
   GetModuleFileName(NULL, szFolder, MAX_PATH);

   LPTSTR pszCur = _tcsrchr(szFolder, _T('\\'));

   if(pszCur)
      *pszCur = NULL;

   TCHAR szFwdDelim[MAX_PATH] = _T("");
   _tcsncpy_s(szFwdDelim, MAX_PATH, szFolder, _TRUNCATE);

   pszCur = _tcsrchr(szFwdDelim, _T('\\'));

   if(pszCur)
      *pszCur = NULL;

   pszCur = _tcschr(szFwdDelim, _T('\\'));

   while(pszCur)
   {
      *pszCur = _T('/');
      pszCur = _tcschr(szFwdDelim, _T('\\'));
   }

   const int PARAM_SIZE = 1024;
   TCHAR szParams[PARAM_SIZE] = _T("-Xmx256m -XX:MaxPermSize=128m -Dopennms.home=\"");

   _tcsncat_s(szParams, PARAM_SIZE - _tcslen(szParams), szFwdDelim, _TRUNCATE);
   _tcsncat_s(szParams, PARAM_SIZE - _tcslen(szParams), _T("\" -jar \""), _TRUNCATE);
   _tcsncat_s(szParams, PARAM_SIZE - _tcslen(szParams), szFwdDelim, _TRUNCATE);
   _tcsncat_s(szParams, PARAM_SIZE - _tcslen(szParams), _T("/lib/opennms_bootstrap.jar\" "), _TRUNCATE);

   if(bIsStart)
      _tcsncat_s(szParams, PARAM_SIZE - _tcslen(szParams), _T("start"), _TRUNCATE);
   else
      _tcsncat_s(szParams, PARAM_SIZE - _tcslen(szParams), _T("stop"), _TRUNCATE);

   ShellExecute(hWnd, szVerb, szFile, szParams, szFolder, SW_HIDE);
}
