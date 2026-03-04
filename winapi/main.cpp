#include "main.h"
#include "ui.h"
#include "cpu_monitor.h"
#include "ram_monitor.h"
#include "disk_monitor.h"
#include "network_monitor.h"
#include "gpu_monitor.h"
#include "alerts.h"
#include "globals.h"

#pragma comment(lib, "iphlpapi.lib")

#define WM_START_THREADS (WM_USER + 1)

using namespace std;
using namespace chrono;

thread cpuThread;
thread ramThread;
thread diskThread;
thread networkThread;
thread alertThread;
thread gpuThread;

// ─── Color scheme ─────────────────────────────────────────────────────────────
#define CLR_BG       RGB(15,  15,  25)
#define CLR_PANEL    RGB(25,  25,  40)
#define CLR_BORDER   RGB(50,  50,  80)
#define CLR_TEXT     RGB(220, 220, 230)
#define CLR_DIM      RGB(130, 130, 150)
#define CLR_BAR_BG   RGB(45,  45,  60)
#define CLR_CPU      RGB(86,  156, 214)
#define CLR_RAM      RGB(78,  201, 176)
#define CLR_DISK     RGB(220, 140, 50)
#define CLR_NET      RGB(106, 200, 106)
#define CLR_NVIDIA   RGB(118, 185, 0)
#define CLR_AMD      RGB(237, 100, 80)

static HFONT g_font = nullptr;
static HFONT g_fontBold = nullptr;

static void InitFonts() {
    g_font = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
    g_fontBold = CreateFontA(13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, "Consolas");
}

static void DrawBar(HDC hdc, RECT r, float pct, COLORREF color) {
    HBRUSH bg = CreateSolidBrush(CLR_BAR_BG);
    FillRect(hdc, &r, bg);
    DeleteObject(bg);

    pct = max(0.0f, min(pct, 100.0f));
    int fillW = (int)((r.right - r.left) * (pct / 100.0f));
    if (fillW > 0) {
        RECT fill = { r.left, r.top, r.left + fillW, r.bottom };
        HBRUSH fb = CreateSolidBrush(color);
        FillRect(hdc, &fill, fb);
        DeleteObject(fb);
    }
}

static void DrawPanel(HDC hdc, RECT r, COLORREF accent, LPCWSTR title) {
    // Background
    HBRUSH pb = CreateSolidBrush(CLR_PANEL);
    FillRect(hdc, &r, pb);
    DeleteObject(pb);

    // Border
    HPEN pen = CreatePen(PS_SOLID, 1, CLR_BORDER);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);

    // Accent bar on left edge
    RECT accent_bar = { r.left, r.top, r.left + 3, r.bottom };
    HBRUSH ab = CreateSolidBrush(accent);
    FillRect(hdc, &accent_bar, ab);
    DeleteObject(ab);

    // Title
    SelectObject(hdc, g_fontBold);
    SetTextColor(hdc, accent);
    SetBkMode(hdc, TRANSPARENT);
    RECT tr = { r.left + 12, r.top + 8, r.right - 8, r.top + 26 };
    DrawTextW(hdc, title, -1, &tr, DT_LEFT | DT_SINGLELINE);
}

static void DrawText2(HDC hdc, int x, int y, int w, LPCWSTR text, COLORREF color) {
    SelectObject(hdc, g_font);
    SetTextColor(hdc, color);
    RECT r = { x, y, x + w, y + 18 };
    DrawTextW(hdc, text, -1, &r, DT_LEFT | DT_SINGLELINE);
}

static void PaintAll(HDC hdc) {
    wchar_t buf[256];

    // ── CPU ──────────────────────────────────────────────────────────────────
    RECT cpu = { 15, 15, 395, 185 };
    DrawPanel(hdc, cpu, CLR_CPU, L"CPU");
    swprintf(buf, 256, L"Usage:   %d%%", currentCPUUsage);
    DrawText2(hdc, 27, 32, 355, buf, CLR_TEXT);
    DrawBar(hdc, { 27, 155, 383, 169 }, (float)currentCPUUsage, CLR_CPU);

    // ── RAM ──────────────────────────────────────────────────────────────────
    RECT ram = { 405, 15, 785, 185 };
    DrawPanel(hdc, ram, CLR_RAM, L"MEMORY");
    swprintf(buf, 256, L"Load:    %d%%", currentRAMUsage);
    DrawText2(hdc, 417, 32, 355, buf, CLR_TEXT);
    DrawBar(hdc, { 417, 155, 773, 169 }, (float)currentRAMUsage, CLR_RAM);

    // ── DISK ─────────────────────────────────────────────────────────────────
    RECT disk = { 15, 195, 395, 365 };
    DrawPanel(hdc, disk, CLR_DISK, L"DISK  (C:\\)");

    swprintf(buf, 256, L"Space:    %.1f%% used", currentDiskSpace);
    DrawText2(hdc, 27, 212, 355, buf, CLR_TEXT);

    swprintf(buf, 256, L"I/O:      %.1f%% active", currentDiskIO);
    DrawText2(hdc, 27, 230, 355, buf, CLR_TEXT);

    // Two bars
    DrawText2(hdc, 27, 295, 60, L"Space", CLR_DIM);
    DrawBar(hdc, { 27, 313, 383, 327 }, (float)currentDiskSpace, CLR_DISK);

    DrawText2(hdc, 27, 330, 60, L"I/O", CLR_DIM);
    DrawBar(hdc, { 27, 348, 383, 362 }, (float)currentDiskIO, CLR_DISK);

    // ── NETWORK ───────────────────────────────────────────────────────────────
    RECT net = { 405, 195, 785, 365 };
    DrawPanel(hdc, net, CLR_NET, L"NETWORK");
    swprintf(buf, 256, L"Upload:   %.2f KB/s", currentNetworkUp);
    DrawText2(hdc, 417, 212, 355, buf, CLR_TEXT);
    swprintf(buf, 256, L"Download: %.2f KB/s", currentNetworkDown);
    DrawText2(hdc, 417, 230, 355, buf, CLR_TEXT);
    DrawBar(hdc, { 417, 315, 773, 329 }, (float)min(currentNetworkUp / 1024.0 * 100.0, 100.0), CLR_NET);
    DrawBar(hdc, { 417, 335, 773, 349 }, (float)min(currentNetworkDown / 1024.0 * 100.0, 100.0), CLR_NET);

    // ── NVIDIA ────────────────────────────────────────────────────────────────
    RECT nv = { 15, 375, 395, 545 };
    DrawPanel(hdc, nv, CLR_NVIDIA, L"NVIDIA RTX 3050 Ti");
    swprintf(buf, 256, L"Usage:   %d%%", currentNvidiaUsage);
    DrawText2(hdc, 27, 392, 355, buf, CLR_TEXT);
    swprintf(buf, 256, L"VRAM:    %llu / %llu MB",
        currentNvidiaVRAMUsed / 1024 / 1024,
        currentNvidiaVRAMTotal / 1024 / 1024);
    DrawText2(hdc, 27, 410, 355, buf, CLR_TEXT);
    swprintf(buf, 256, L"Temp:    %u C", currentNvidiaTemp);
    DrawText2(hdc, 27, 428, 355, buf, CLR_TEXT);
    swprintf(buf, 256, L"Clock:   %u MHz", currentNvidiaClock);
    DrawText2(hdc, 27, 446, 355, buf, CLR_TEXT);
    DrawBar(hdc, { 27, 515, 383, 529 }, (float)currentNvidiaUsage, CLR_NVIDIA);

    // ── AMD ───────────────────────────────────────────────────────────────────
    RECT amd = { 405, 375, 785, 545 };
    DrawPanel(hdc, amd, CLR_AMD, L"AMD RADEON");
    swprintf(buf, 256, L"Usage:   %.1f%%", currentAMDUsage);
    DrawText2(hdc, 417, 392, 355, buf, CLR_TEXT);
    DrawBar(hdc, { 417, 515, 773, 529 }, (float)currentAMDUsage, CLR_AMD);
}

void AlertThread() {
    while (updateFlag) {
        CheckAndShowAlerts();
        this_thread::sleep_for(1s);
    }
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS SoftwareMainClass = NewWindowClass(
        (HBRUSH)COLOR_WINDOW,
        LoadCursor(NULL, IDC_ARROW),
        hInstance,
        LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
        L"MainWNDClass",
        SoftwareMainProcedure
    );

    if (!RegisterClass(&SoftwareMainClass)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return -1;
    }

    InitFonts();

    MSG SoftwareMainMessage = { 0 };

    CreateWindow(L"MainWNDClass", L"System Monitor",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        820, 620,   // ← fits the grid
        NULL, NULL, hInstance, NULL);

    while (GetMessage(&SoftwareMainMessage, NULL, 0, 0)) {
        TranslateMessage(&SoftwareMainMessage);
        DispatchMessage(&SoftwareMainMessage);
    }

    return 0;
}

WNDCLASS NewWindowClass(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCWSTR Name, WNDPROC Procedure) {
    WNDCLASS NWC = { 0 };
    NWC.hIcon = Icon;
    NWC.hCursor = Cursor;
    NWC.hInstance = hInst;
    NWC.lpszClassName = Name;
    NWC.hbrBackground = BGColor;
    NWC.lpfnWndProc = Procedure;
    return NWC;
}

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case MENU_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_CREATE:
        SetTimer(hWnd, 1, 1000, NULL);
        MainWndAddMenus(hWnd);
        MainWndAddWidgets(hWnd);
        PostMessage(hWnd, WM_START_THREADS, 0, 0);
        break;

    case WM_START_THREADS:
        cpuThread = thread(UpdateCPUUsage);
        ramThread = thread(UpdateMemoryUsage);
        diskThread = thread(UpdateDiskUsage);
        networkThread = thread(UpdateNetworkUsage);
        alertThread = thread(AlertThread);
        gpuThread = thread(UpdateGPUUsage);
        break;

    case WM_DESTROY:
        KillTimer(hWnd, 1);
        if (g_font)     DeleteObject(g_font);
        if (g_fontBold) DeleteObject(g_fontBold);

        updateFlag = false;
        if (cpuThread.joinable())     cpuThread.join();
        if (ramThread.joinable())     ramThread.join();
        if (diskThread.joinable())    diskThread.join();
        if (networkThread.joinable()) networkThread.join();
        if (alertThread.joinable())   alertThread.join();
        PostQuitMessage(0);
        break;

    case WM_TIMER:
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT cr; GetClientRect(hWnd, &cr);
        HDC     memDC = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, cr.right, cr.bottom);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

        HBRUSH bgBrush = CreateSolidBrush(CLR_BG);
        FillRect(memDC, &cr, bgBrush);
        DeleteObject(bgBrush);

        PaintAll(memDC);

        BitBlt(hdc, 0, 0, cr.right, cr.bottom, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_ERASEBKGND:
        return 1;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}