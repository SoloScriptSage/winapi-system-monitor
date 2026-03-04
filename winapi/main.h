#ifndef SOFTWARE_DEFINITIONS_H
#define SOFTWARE_DEFINITIONS_H

#include <Windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <iostream>
#include <thread>
#include "resource.h"
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <atomic>
#include <chrono>

#define TEXT_BUFFER_SIZE 256

#define MENU_NEW    2001
#define MENU_OPEN   2002
#define MENU_SAVE   2003
#define MENU_EXIT   2004

char Buffer[TEXT_BUFFER_SIZE];
char filename[MAX_PATH];
OPENFILENAMEA ofn;

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure);

void MainWndAddMenus(HWND hWnd);
void MainWndAddWidgets(HWND hWnd);

void SaveData(LPCSTR path);
void LoadData(LPCSTR path);
void SetOpenFileParameters(HWND hWND);

#endif // SOFTWARE_DEFINITIONS_H