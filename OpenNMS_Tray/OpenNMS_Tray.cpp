/*++

(C) 2011 OpenNMS Group, Inc. All rights reserved.

Module Name:

   OpenNMS_Tray.cpp

Abstract:

   Defines the service entry point.

Environment:

    User mode

--*/

#include "stdafx.h"
#include "AppRes.h"
#include "resource.h"
#include "TrayHelper.h"
#include "OpenNMS_Tray.h"

//---------------------------------------------------------------------------------------
typedef HRESULT (CALLBACK *FNDllGetVersion)(DLLVERSIONINFO* pdvi);

static const int WM_TRAY = WM_USER + 1;

static const LPTSTR STR_WND_NAME = _T("OpenNMS_Tray");
static const LPTSTR STR_SVC_NAME = _T("OpenNMS_Service");

//---------------------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                       LPTSTR lpCmdLine, int nCmdShow)
{
   HWND hPrevSvc = FindWindow(STR_WND_NAME, 0);

   if(!hPrevSvc)
   {
      WNDCLASS WndClass = {0};
      WndClass.lpfnWndProc = WndProc;
      WndClass.hInstance = GetModuleHandle(NULL);
      WndClass.lpszClassName = STR_WND_NAME;

      if(RegisterClass(&WndClass))
      {
         /*
          * create hidden window
          */

         HWND hWnd = CreateWindow(STR_WND_NAME, 0, 0, 0, 0, 0, 0,
                                  0, 0, WndClass.hInstance, 0);

         if(hWnd)
         {
            /*
             * main message loop
             */

            MSG msg = {0};

            while(GetMessage(&msg, NULL, 0, 0) > 0)
            { 
               TranslateMessage(&msg); 
               DispatchMessage(&msg); 
            }
         }

         UnregisterClass(STR_WND_NAME, WndClass.hInstance);
      }
   }

   return 0;
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

   case WM_TRAY:
      lr = OnTray(hWnd, wParam, lParam);
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
   LRESULT lr = 0;
   PNOTIFYICONDATA pTrayIcon = (PNOTIFYICONDATA)malloc(sizeof(NOTIFYICONDATA));

   if(pTrayIcon)
   {
      ZeroMemory(pTrayIcon,  sizeof(NOTIFYICONDATA));
      HMODULE hMod = LoadLibrary(_T("shell32.dll"));

      if(hMod)
      {
         FNDllGetVersion pfnDllGetVersion = (FNDllGetVersion)GetProcAddress(hMod, "DllGetVersion");

         if(pfnDllGetVersion)
         {
            DLLVERSIONINFO VerInfo = {0};
            VerInfo.cbSize = sizeof(VerInfo);
            pfnDllGetVersion(&VerInfo);

            if(VerInfo.dwMajorVersion == 6)
            {
               if(VerInfo.dwBuildNumber > 6)
                  pTrayIcon->cbSize = sizeof(NOTIFYICONDATA);
               else
                  pTrayIcon->cbSize = NOTIFYICONDATA_V3_SIZE;

               pTrayIcon->uVersion = NOTIFYICON_VERSION_4;
            }
            else if(VerInfo.dwMajorVersion == 5)
            {
               pTrayIcon->cbSize = NOTIFYICONDATA_V2_SIZE;
               pTrayIcon->uVersion = NOTIFYICON_VERSION;
            }
            else
            {
               pTrayIcon->cbSize = NOTIFYICONDATA_V1_SIZE;
            }
         }

         FreeLibrary(hMod);
         hMod = NULL;
      }

      pTrayIcon->hWnd = hWnd;
      pTrayIcon->uFlags = NIF_INFO | NIF_TIP | NIF_ICON | NIF_MESSAGE;
      pTrayIcon->dwInfoFlags = NIIF_USER;
      pTrayIcon->uCallbackMessage = WM_TRAY;
      pTrayIcon->uTimeout = 10000;

      pTrayIcon->hIcon = (HICON)LoadImage(GetModuleHandle(NULL),
                                          MAKEINTRESOURCE(IDI_OPENNMS_TRAY),
                                          IMAGE_ICON, 0, 0, 0);

      hMod = GetResourceHandle();

      if(hMod)
      {
         TCHAR szBuf[MAX_PATH] = _T("");
         LoadString(hMod, IDS_TRAY_TIP, szBuf, MAX_PATH);
         _tcsncpy_s(pTrayIcon->szTip, ARRAYSIZE(pTrayIcon->szTip), szBuf, _TRUNCATE);

         LoadString(hMod, IDS_TRAY_INFOTITLE, szBuf, MAX_PATH);
         _tcsncpy_s(pTrayIcon->szInfoTitle, ARRAYSIZE(pTrayIcon->szInfoTitle), szBuf, _TRUNCATE);

         LoadString(hMod, IDS_TRAY_INFOTEXT, szBuf, MAX_PATH);
         _tcsncpy_s(pTrayIcon->szInfo, ARRAYSIZE(pTrayIcon->szInfo), szBuf, _TRUNCATE);

         FreeResourceHandle(hMod);
      }

      Sleep(5000);
      Shell_NotifyIcon(NIM_ADD, pTrayIcon);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pTrayIcon);
   }

   return lr;
}

//---------------------------------------------------------------------------------------
LRESULT OnTray(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   LRESULT lr = 0;

   if(lParam == WM_RBUTTONUP)
   {
      SetForegroundWindow(hWnd);

      switch(DisplayMenu(hWnd))
      {
      case 1:
         ShellExecute(hWnd, _T("open"), _T("http://localhost:8980/opennms"), NULL, NULL, SW_SHOWDEFAULT);
         break;

      case 2:
         ControlService(IsServiceRunning());
         break;

      case 3:
         DisplayAbout(hWnd);
         break;

      case 4:
         PostMessage(hWnd, WM_CLOSE, 0, 0);
         break;
      }

      PostMessage(hWnd, WM_NULL, 0, 0);
   }
   else if(lParam == WM_LBUTTONDBLCLK || lParam == NIN_BALLOONUSERCLICK)
   {
      ShellExecute(hWnd, _T("open"), _T("http://localhost:8980/opennms"), NULL, NULL, SW_SHOWDEFAULT);
   }

   return lr;
}

//---------------------------------------------------------------------------------------
LRESULT OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   LRESULT lr = 0;
   PNOTIFYICONDATA pTrayIcon = (PNOTIFYICONDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);

   if(pTrayIcon)
   {
      Shell_NotifyIcon(NIM_DELETE, pTrayIcon);

      DestroyIcon(pTrayIcon->hIcon);
      free(pTrayIcon);
   }

   DestroyWindow(hWnd);
   return lr;
}

//---------------------------------------------------------------------------------------
void ControlService(bool bIsStop)
{
   SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
                                  SC_MANAGER_ALL_ACCESS);

   if(hSCM)
   {
      SC_HANDLE hSVC = OpenService(hSCM, STR_SVC_NAME, SERVICE_ALL_ACCESS);

      if(hSVC)
      {
         SERVICE_STATUS SvcStat = {0};

         if(bIsStop)
            ControlService(hSVC, SERVICE_CONTROL_STOP, &SvcStat);
         else
            StartService(hSVC, 0, NULL);

         CloseServiceHandle(hSVC);
      }

      CloseServiceHandle(hSCM);
   }
}

//---------------------------------------------------------------------------------------
bool IsServiceRunning()
{
   bool bIsRunning = false;
   SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT |
                                  SC_MANAGER_ALL_ACCESS);

   if(hSCM)
   {
      SC_HANDLE hSVC = OpenService(hSCM, STR_SVC_NAME, SERVICE_ALL_ACCESS);

      if(hSVC)
      {
         SERVICE_STATUS SvcStat = {0};
         ControlService(hSVC, SERVICE_CONTROL_INTERROGATE, &SvcStat);

         if(SvcStat.dwCurrentState == SERVICE_RUNNING)
            bIsRunning = true;

         CloseServiceHandle(hSVC);
      }

      CloseServiceHandle(hSCM);
   }

   return bIsRunning;
}

//---------------------------------------------------------------------------------------
int DisplayMenu(HWND hWnd)
{
   int nID = 0;
   HMENU hMenu = CreatePopupMenu();

   if(hMenu)
   {
      HMODULE hMod = GetResourceHandle();

      if(hMod)
      {
         TCHAR szBuf[MAX_PATH] = _T("");
         LoadString(hMod, IDS_MNU_LAUNCH, szBuf, MAX_PATH);
         AppendMenu(hMenu, MF_STRING, 1, szBuf);

         if(IsServiceRunning())
         {
            LoadString(hMod, IDS_MNU_STOP, szBuf, MAX_PATH);
            AppendMenu(hMenu, MF_STRING, 2, szBuf);
         }
         else
         {
            LoadString(hMod, IDS_MNU_START, szBuf, MAX_PATH);
            AppendMenu(hMenu, MF_STRING, 2, szBuf);
         }

         LoadString(hMod, IDS_MNU_ABOUT, szBuf, MAX_PATH);
         AppendMenu(hMenu, MF_STRING, 3, szBuf);

         AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

         LoadString(hMod, IDS_MNU_EXIT, szBuf, MAX_PATH);
         AppendMenu(hMenu, MF_STRING, 4, szBuf);

         FreeResourceHandle(hMod);
      }

      POINT pt = {0};
      GetCursorPos (&pt);

      SetMenuDefaultItem(hMenu, 0, TRUE);
      nID = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
      DestroyMenu(hMenu);
   }

   return nID;
}

//---------------------------------------------------------------------------------------
int DisplayAbout(HWND hWnd)
{
   HMODULE hMod = GetResourceHandle();

   if(hMod)
   {
      TCHAR szTitle[MAX_PATH] = _T("");
      LoadString(hMod, IDS_APP_NAME, szTitle, MAX_PATH);

      const int BUF_SIZE = MAX_PATH * 2;
      TCHAR szMsg[BUF_SIZE] = _T("");
      LoadString(hMod, IDS_APP_ABOUT, szMsg, BUF_SIZE);

      MessageBox(hWnd, szMsg, szTitle, MB_ICONINFORMATION | MB_OK);
      FreeResourceHandle(hMod);
   }

   return 0;
}
