#include "ui.h"
#include <Windows.h>

// Global variables for UI controls
HWND hStaticControl;
HWND hEditControl;
HWND hNumberControl;
HWND hGPU_Nvidia;
HWND hGPU_AMD;

RECT rc;

// Widgets and Menus

void MainWndAddMenus(HWND hWnd) {
	HMENU hMenu = CreateMenu(); // Create a general menu

	HMENU hFileMenu = CreateMenu(); // Create a "File" menu
	HMENU hNewMenu = CreateMenu(); // Create a "New" menu
	HMENU hEditMenu = CreateMenu(); // Create an "Edit" menu

	// hFileMenu is the handle to the "File" menu
	// MF_STRING is the menu item type
	// 1 is the ID of the menu item
	// L"New" is the text of the menu item
	AppendMenu(hFileMenu, MF_POPUP, (UINT_PTR)hNewMenu, L"New");
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, 2002, L"Open");
	AppendMenu(hFileMenu, MF_STRING, 2003, L"Save");
	AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hFileMenu, MF_STRING, 2004, L"Exit");
	// Add the "File" menu to the main menu
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");

	// Adding elements to the "New" menu
	AppendMenu(hNewMenu, MF_STRING, NULL, L"Project");
	AppendMenu(hNewMenu, MF_STRING, NULL, L"File");
	AppendMenu(hNewMenu, MF_STRING, NULL, L"Repository");

	// Adding elements to the "Edit" menu
	AppendMenu(hEditMenu, MF_STRING, 2005, L"Cut");
	AppendMenu(hEditMenu, MF_STRING, 2006, L"Copy");
	AppendMenu(hEditMenu, MF_STRING, 2007, L"Paste");

	// Add the "Edit" menu to the main menu
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"Edit");

	// Set the menu to the window
	SetMenu(hWnd, hMenu);
}

void MainWndAddWidgets(HWND hWnd) {
	// Drawn via WM_PAINT
}
