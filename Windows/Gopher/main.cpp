/*-------------------------------------------------------------------------------
    Gopher free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
---------------------------------------------------------------------------------*/

//changes 0.96 -> 0.97: speed variable is global, detects bumpers, all timed (no enter), lbumper speed toggler
//changes 0.97 -> 0.98: performance improvements, operational volume function, shorter beeps, no XY text
//changes 0.98 -> 0.985: 144Hz, Y to hide window(added float stillHoldingY), code cleanup, comments added
//changes 0.985 -> 0.986: Adding configuration file, changing from beeps to vibration.
//changes 0.986 -> 0.989: Improved speeds and speed reporting, created automatic config generator!
//TODO FOR FUTURE VERSIONS - offload speeds into config file

#include <windows.h> // for Beep()
#include <iostream>


#pragma comment(lib, "XInput9_1_0.lib")
#pragma comment(lib, "winmm") // for volume

#include "Gopher.h"




bool ChangeVolume(double nVolume, bool bScalar); // Not used yet
BOOL isRunningAsAdministrator(); // Check if administrator, makes on-screen keyboard clickable

/* TODO:
 * Enable/disable button
 * Key Codes:
 *   http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
 * xinput:
 *   http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference.xinput_gamepad%28v=vs.85%29.aspx
 */

BOOL isRunningAsAdministrator()
{
  BOOL   fRet = FALSE;
  HANDLE hToken = NULL;

  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
  {
    TOKEN_ELEVATION Elevation;
    DWORD cbSize = sizeof( TOKEN_ELEVATION );

    if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof( Elevation), &cbSize))
    {
      fRet = Elevation.TokenIsElevated;
    }
  }

  if (hToken)
  {
    CloseHandle(hToken);
  }

  return fRet;
}

// This works, but it's not enabled in the software since the best button for it is still undecided
bool ChangeVolume(double nVolume, bool bScalar) //o b
{
  HRESULT hr = NULL;
  bool decibels = false;
  bool scalar = false;
  double newVolume = nVolume;

  CoInitialize(NULL);
  IMMDeviceEnumerator *deviceEnumerator = NULL;
  hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
                        __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
  IMMDevice *defaultDevice = NULL;

  hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
  deviceEnumerator->Release();
  deviceEnumerator = NULL;

  IAudioEndpointVolume *endpointVolume = NULL;
  hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume),
                               CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
  defaultDevice->Release();
  defaultDevice = NULL;

  // -------------------------
  float currentVolume = 0;
  endpointVolume->GetMasterVolumeLevel(&currentVolume);
  //printf("Current volume in dB is: %f\n", currentVolume);

  hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
  //CString strCur=L"";
  //strCur.Format(L"%f",currentVolume);
  //AfxMessageBox(strCur);

  // printf("Current volume as a scalar is: %f\n", currentVolume);
  if (bScalar == false)
  {
    hr = endpointVolume->SetMasterVolumeLevel((float)newVolume, NULL);
  }
  else if (bScalar == true)
  {
    hr = endpointVolume->SetMasterVolumeLevelScalar((float)newVolume, NULL);
  }
  endpointVolume->Release();

  CoUninitialize();

  return FALSE;
}



// QuickCodePage.cpp : Defines the entry point for the application.
//


#include "resource.h"

const UINT WM_TRAY = WM_USER + 1;
HINSTANCE g_hInstance = NULL;
HICON g_hIcon = NULL;


void LoadStringSafe(UINT nStrID, LPTSTR szBuf, UINT nBufLen)
{
	UINT nLen = LoadString(g_hInstance, nStrID, szBuf, nBufLen);
	if (nLen >= nBufLen)
		nLen = nBufLen - 1;
	szBuf[nLen] = 0;
}

HWND g_hCodePages = NULL;
long g_nLastCodePage = 0;

struct CWndSizeData {
	HWND m_hWnd;
	RECT m_stRect;
	int m_nResizeFlags;
};

struct CPopupWndData {
	HWND m_hWnd;
	SIZE m_stInitialSize;

	CWndSizeData* m_pControls; // currently we have 3 child controls
	DWORD m_dwControls;

	static CPopupWndData* s_pThis;
};

CPopupWndData* CPopupWndData::s_pThis = NULL;

BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		if (CPopupWndData::s_pThis)
		{
			CPopupWndData::s_pThis->m_hWnd = hWnd;

			if (g_hIcon)
			{
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)g_hIcon);
				SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)g_hIcon);
			}

			// display it
			SendMessage(hWnd, WM_COMMAND, MAKELONG(0, CBN_SELENDOK), 0);
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hWnd, IDCANCEL);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_HIDE:
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}

		break;
		return 0;

	}
	return FALSE;
}


LRESULT CALLBACK HiddenWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		NOTIFYICONDATA stData;
		ZeroMemory(&stData, sizeof(stData));
		stData.cbSize = sizeof(stData);
		stData.hWnd = hWnd;
		stData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		stData.uCallbackMessage = WM_TRAY;
		stData.hIcon = g_hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_TRAYICON));
		LoadStringSafe(IDS_TIP, stData.szTip, _countof(stData.szTip));
		if (!Shell_NotifyIcon(NIM_ADD, &stData))
			return -1; // oops
	}
	return 0;

	case WM_DESTROY:
	{
		NOTIFYICONDATA stData;
		ZeroMemory(&stData, sizeof(stData));
		stData.cbSize = sizeof(stData);
		stData.hWnd = hWnd;
		Shell_NotifyIcon(NIM_DELETE, &stData);
	}
	return 0;

	case WM_TRAY:
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			SendMessage(hWnd, WM_COMMAND, ID_SHOW, 0);
			break;

		case WM_RBUTTONDOWN:
		{
			HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_POPUP));
			if (hMenu)
			{
				HMENU hSubMenu = GetSubMenu(hMenu, 0);
				if (hSubMenu)
				{
					POINT stPoint;
					GetCursorPos(&stPoint);

					TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, stPoint.x, stPoint.y, 0, hWnd, NULL);
				}

				DestroyMenu(hMenu);
			}
		}
		break;
		}
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SHOW:
			if (CPopupWndData::s_pThis)
				SetForegroundWindow(CPopupWndData::s_pThis->m_hWnd);
			else
			{
				CPopupWndData stData;
				ZeroMemory(&stData, sizeof(stData));
				CPopupWndData::s_pThis = &stData;

				DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_GOPHER), NULL, (DLGPROC)DialogProc);

				if (stData.m_pControls)
					delete[] stData.m_pControls;
				CPopupWndData::s_pThis = NULL;
			}
			return 0;

		case ID_QUIT:
			PostQuitMessage(0);
			return 0;
		}
		break;

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI gophersThread( LPVOID lpParam )
{
	std::vector<CXBOXController*> controllers;
	std::vector<Gopher*> gophers;
	for (int i = 0; i < 5; i++)
	{
		CXBOXController *controller = new CXBOXController(i + 1);
		controllers.push_back(controller);
		Gopher* gopher = new Gopher(controller);
		gopher->loadConfigFile();
		gophers.push_back(gopher);
	}
	while (!*(BOOL*)lpParam) {
		for(auto gopher : gophers)
			gopher->loop();
	}

	for (auto gopher : gophers) delete gopher;
	for (auto controller : controllers) delete controller;

	return 0;
}



int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{

	TCHAR szTxt[0x100];
	wsprintf(szTxt, _T("hInstance=%08x\n"), hInstance);
	OutputDebugString(szTxt);

	WNDCLASS stWC;
	ZeroMemory(&stWC, sizeof(stWC));
	stWC.lpszClassName = _T("Gopher360");

	HWND hHiddenWnd = FindWindow(stWC.lpszClassName, NULL);
	if (hHiddenWnd)
		PostMessage(hHiddenWnd, WM_TRAY, 0, WM_LBUTTONDBLCLK);
	else
	{

		stWC.hInstance = hInstance;
		stWC.lpfnWndProc = HiddenWndProc;

		ATOM aClass = RegisterClass(&stWC);
		if (aClass)
		{
			g_hInstance = hInstance;
			if (hHiddenWnd = CreateWindow((LPCTSTR)aClass, _T(""), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL))
			{
				BOOL terminateFlag = 0;
				HANDLE gophersTread = CreateThread(NULL, 0, gophersThread, &terminateFlag, 0, 0);

				MSG stMsg;
				while (GetMessage(&stMsg, NULL, 0, 0) > 0)
				{
					TranslateMessage(&stMsg);
					DispatchMessage(&stMsg);
				}

				if (IsWindow(hHiddenWnd))
					DestroyWindow(hHiddenWnd);

				terminateFlag = TRUE;
				WaitForMultipleObjects(1, &gophersTread, TRUE, INFINITE);
				CloseHandle(gophersTread);
			}
			UnregisterClass((LPCTSTR)aClass, g_hInstance);
		}
	}
	return 0;
}
