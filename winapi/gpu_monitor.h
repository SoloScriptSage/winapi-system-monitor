#pragma once
#include <windows.h>
#include <pdh.h>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>

#pragma comment(lib, "pdh.lib")

void InitGPUMonitor();
void UpdateGPUUsage();
void ShutdownGPUMonitor();