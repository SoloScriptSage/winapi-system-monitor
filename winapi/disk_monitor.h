#pragma once
#include <windows.h>
#include <pdh.h>
#include <thread>
#include <chrono>
#include <cstdio>
#include <atomic>

#pragma comment(lib, "pdh.lib")

void InitDiskMonitor();
void GetDiskUsage();
void UpdateDiskUsage();