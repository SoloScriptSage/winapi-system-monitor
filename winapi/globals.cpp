#include "globals.h"

using namespace std;
using namespace chrono;

atomic<bool> updateFlag = true; // Flag to control the update loop
HWND hCPU, hRAM, hNetwork, hDisk;

int currentCPUUsage = 0;
int currentRAMUsage = 0;

double currentNetworkUp = 0.0;
double currentNetworkDown = 0.0;
int currentNvidiaUsage = 0;
unsigned long long currentNvidiaVRAMUsed = 0;
unsigned long long currentNvidiaVRAMTotal = 0;
unsigned int currentNvidiaTemp = 0;
unsigned int currentNvidiaClock = 0;
double currentAMDUsage = 0.0;

double currentDiskIO = 0.0;
double currentDiskSpace = 0.0;

steady_clock::time_point lastCPUAlert = steady_clock::now();
steady_clock::time_point lastRAMAlert = steady_clock::now();
steady_clock::time_point lastDiskAlert = steady_clock::now();
