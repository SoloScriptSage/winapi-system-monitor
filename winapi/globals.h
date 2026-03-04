#pragma once

#include <Windows.h>
#include <chrono>
#include <vector>
#include <atomic>
#include <thread>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

extern atomic<bool>updateFlag;
extern HWND hCPU, hRAM, hNetwork, hDisk; // Handles to the CPU, RAM, and Network labels;

extern int currentCPUUsage;
extern int currentRAMUsage;

extern double currentNetworkUp;
extern double currentNetworkDown;
extern int currentNvidiaUsage;
extern unsigned long long currentNvidiaVRAMUsed;
extern unsigned long long currentNvidiaVRAMTotal;
extern unsigned int currentNvidiaTemp;
extern unsigned int currentNvidiaClock;
extern double currentAMDUsage;

extern double currentDiskIO;
extern double currentDiskSpace;

extern chrono::steady_clock::time_point lastCPUAlert;
extern chrono::steady_clock::time_point lastRAMAlert;
extern chrono::steady_clock::time_point lastDiskAlert;
