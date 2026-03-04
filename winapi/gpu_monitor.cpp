#include "gpu_monitor.h"
#include "globals.h"
#include "ui.h"
#include <sstream>
#include <iomanip>

// ─── NVML dynamic bindings ────────────────────────────────────────────────────

typedef int nvmlReturn_t;
#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

struct nvmlUtilization_t { unsigned int gpu; unsigned int memory; };
struct nvmlMemory_t { unsigned long long total; unsigned long long free; unsigned long long used; };

typedef struct nvmlDevice* nvmlDevice_t;

typedef nvmlReturn_t(*pfn_nvmlInit)();
typedef nvmlReturn_t(*pfn_nvmlShutdown)();
typedef nvmlReturn_t(*pfn_nvmlDeviceGetHandleByIndex)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t(*pfn_nvmlDeviceGetUtilizationRates)(nvmlDevice_t, nvmlUtilization_t*);
typedef nvmlReturn_t(*pfn_nvmlDeviceGetMemoryInfo)(nvmlDevice_t, nvmlMemory_t*);
typedef nvmlReturn_t(*pfn_nvmlDeviceGetTemperature)(nvmlDevice_t, int, unsigned int*);
typedef nvmlReturn_t(*pfn_nvmlDeviceGetClockInfo)(nvmlDevice_t, int, unsigned int*);

static HMODULE                          hNvml = nullptr;
static nvmlDevice_t                     nvmlDevice = nullptr;
static pfn_nvmlInit                     _nvmlInit = nullptr;
static pfn_nvmlShutdown                 _nvmlShutdown = nullptr;
static pfn_nvmlDeviceGetHandleByIndex   _nvmlDeviceGetHandleByIndex = nullptr;
static pfn_nvmlDeviceGetUtilizationRates _nvmlDeviceGetUtilizationRates = nullptr;
static pfn_nvmlDeviceGetMemoryInfo      _nvmlDeviceGetMemoryInfo = nullptr;
static pfn_nvmlDeviceGetTemperature     _nvmlDeviceGetTemperature = nullptr;
static pfn_nvmlDeviceGetClockInfo       _nvmlDeviceGetClockInfo = nullptr;

static bool nvmlReady = false;

// ─── PDH for AMD ─────────────────────────────────────────────────────────────

static PDH_HQUERY   pdhQuery = nullptr;
static PDH_HCOUNTER pdhUsageCounter = nullptr;
static bool         pdhReady = false;

// ─── Init ─────────────────────────────────────────────────────────────────────

void InitGPUMonitor() {
    // --- NVIDIA ---
    hNvml = LoadLibraryA("nvml.dll");
    if (hNvml) {
        _nvmlInit = (pfn_nvmlInit)GetProcAddress(hNvml, "nvmlInit_v2");
        _nvmlShutdown = (pfn_nvmlShutdown)GetProcAddress(hNvml, "nvmlShutdown");
        _nvmlDeviceGetHandleByIndex = (pfn_nvmlDeviceGetHandleByIndex)GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex_v2");
        _nvmlDeviceGetUtilizationRates = (pfn_nvmlDeviceGetUtilizationRates)GetProcAddress(hNvml, "nvmlDeviceGetUtilizationRates");
        _nvmlDeviceGetMemoryInfo = (pfn_nvmlDeviceGetMemoryInfo)GetProcAddress(hNvml, "nvmlDeviceGetMemoryInfo");
        _nvmlDeviceGetTemperature = (pfn_nvmlDeviceGetTemperature)GetProcAddress(hNvml, "nvmlDeviceGetTemperature");
        _nvmlDeviceGetClockInfo = (pfn_nvmlDeviceGetClockInfo)GetProcAddress(hNvml, "nvmlDeviceGetClockInfo");

        if (_nvmlInit && _nvmlInit() == NVML_SUCCESS) {
            if (_nvmlDeviceGetHandleByIndex &&
                _nvmlDeviceGetHandleByIndex(0, &nvmlDevice) == NVML_SUCCESS) {
                nvmlReady = true;
            }
        }
    }

    // --- AMD via PDH ---
    if (PdhOpenQuery(nullptr, 0, &pdhQuery) == ERROR_SUCCESS) {
        if (PdhAddEnglishCounterA(pdhQuery,
            "\\GPU Engine(*)\\Utilization Percentage",
            0, &pdhUsageCounter) == ERROR_SUCCESS) {
            PdhCollectQueryData(pdhQuery); // first collection to prime it
            pdhReady = true;
        }
    }
}

// ─── NVIDIA polling ──────────────────────────────────────────────────────────

static void PollNvidia() {
    if (!nvmlReady) {
        currentNvidiaUsage = 0;
        currentNvidiaVRAMUsed = 0;
        currentNvidiaVRAMTotal = 0;
        currentNvidiaTemp = 0;
        currentNvidiaClock = 0;
        return;
    }

    nvmlUtilization_t util = {};
    nvmlMemory_t      mem = {};
    unsigned int      temp = 0;
    unsigned int      clock = 0;

    _nvmlDeviceGetUtilizationRates(nvmlDevice, &util);
    _nvmlDeviceGetMemoryInfo(nvmlDevice, &mem);
    _nvmlDeviceGetTemperature(nvmlDevice, NVML_TEMPERATURE_GPU, &temp);
    _nvmlDeviceGetClockInfo(nvmlDevice, 0, &clock);

    currentNvidiaUsage = (int)util.gpu;
    currentNvidiaVRAMUsed = mem.used;
    currentNvidiaVRAMTotal = mem.total;
    currentNvidiaTemp = temp;
    currentNvidiaClock = clock;
}

// ─── AMD polling ─────────────────────────────────────────────────────────────

static void PollAMD() {
    if (!pdhReady) {
        currentAMDUsage = 0.0;
        return;
    }

    PdhCollectQueryData(pdhQuery);

    PDH_FMT_COUNTERVALUE val = {};
    PdhGetFormattedCounterValue(pdhUsageCounter, PDH_FMT_DOUBLE, nullptr, &val);

    currentAMDUsage = val.doubleValue;
}

// ─── Thread ──────────────────────────────────────────────────────────────────

void UpdateGPUUsage() {
    InitGPUMonitor();
    while (updateFlag) {
        PollNvidia();
        PollAMD();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ShutdownGPUMonitor();
}

void ShutdownGPUMonitor() {
    if (nvmlReady && _nvmlShutdown) _nvmlShutdown();
    if (hNvml) FreeLibrary(hNvml);
    if (pdhQuery) PdhCloseQuery(pdhQuery);
}