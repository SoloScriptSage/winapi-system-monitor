#include "alerts.h"
#include "globals.h"

using namespace std;
using namespace chrono;

const int ALERT_INTERVAL = 30; // Alert interval in seconds

bool ShouldShowAlert(steady_clock::time_point& lastAlertTime) {
	auto now = steady_clock::now();
	auto duration = now - lastAlertTime;  // Duration between now and last alert

	if (duration_cast<seconds>(duration).count() >= ALERT_INTERVAL) {
		lastAlertTime = now;
		return true;
	}
	return false;
}

void CheckAndShowAlerts() {
    if (currentCPUUsage > CPU_ALERT_THRESHOLD && ShouldShowAlert(lastCPUAlert)) {
        MessageBox(NULL, L"High CPU Usage! Close some apps.", L"CPU Alert", MB_OK | MB_ICONWARNING);
    }
    if (currentRAMUsage > MEMORY_ALERT_THRESHOLD && ShouldShowAlert(lastRAMAlert)) {
        MessageBox(NULL, L"High RAM Usage! Close unused programs.", L"RAM Alert", MB_OK | MB_ICONWARNING);
    }
    if (currentDiskSpace > DISK_ALERT_THRESHOLD && ShouldShowAlert(lastDiskAlert)) {
        MessageBox(NULL, L"High Disk Usage! Free up space.", L"Disk Alert", MB_OK | MB_ICONWARNING);
    }
}
