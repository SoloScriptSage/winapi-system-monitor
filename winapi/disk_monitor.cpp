#include "disk_monitor.h"
#include "alerts.h"
#include "globals.h"

using namespace std;

static PDH_HQUERY   diskQuery = nullptr;
static PDH_HCOUNTER diskCounter = nullptr;
static bool         pdhReady = false;

void InitDiskMonitor() {
    if (PdhOpenQuery(nullptr, 0, &diskQuery) == ERROR_SUCCESS) {
        if (PdhAddEnglishCounterA(diskQuery,
            "\\PhysicalDisk(_Total)\\% Disk Time",
            0, &diskCounter) == ERROR_SUCCESS) {
            PdhCollectQueryData(diskQuery); // prime it
            pdhReady = true;
        }
    }
}

void GetDiskUsage() {
    // --- Storage space ---
    ULARGE_INTEGER freeBytesAvailable, totalBytes, freeBytes;
    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalBytes, &freeBytes)) {
        currentDiskSpace = (1.0 - ((double)freeBytes.QuadPart / totalBytes.QuadPart)) * 100.0;

        if (currentDiskSpace > DISK_ALERT_THRESHOLD && ShouldShowAlert(lastDiskAlert)) {
            MessageBox(NULL, L"High Disk Usage! Free up space.", L"Disk Alert", MB_OK | MB_ICONWARNING);
        }
    }

    // --- I/O activity ---
    if (!pdhReady) {
        currentDiskIO = 0.0;
        return;
    }

    PdhCollectQueryData(diskQuery);

    PDH_FMT_COUNTERVALUE val = {};
    PdhGetFormattedCounterValue(diskCounter, PDH_FMT_DOUBLE, nullptr, &val);
    currentDiskIO = min(val.doubleValue, 100.0);
}

void UpdateDiskUsage() {
    InitDiskMonitor();
    while (updateFlag) {
        GetDiskUsage();
        this_thread::sleep_for(chrono::seconds(1));
    }
    if (diskQuery) PdhCloseQuery(diskQuery);
}