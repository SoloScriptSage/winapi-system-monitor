#include "network_monitor.h"
#include "globals.h"

void GetNetworkUsage() {
    MIB_IFTABLE* pIfTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    if (GetIfTable(NULL, &dwSize, false) == ERROR_INSUFFICIENT_BUFFER) {
        pIfTable = (MIB_IFTABLE*)malloc(dwSize);
        if (pIfTable == NULL) {
            currentNetworkUp = 0.0;
            currentNetworkDown = 0.0;
            return;
        }
    }
    else {
        currentNetworkUp = 0.0;
        currentNetworkDown = 0.0;
        return;
    }

    if ((dwRetVal = GetIfTable(pIfTable, &dwSize, false)) == NO_ERROR) {
        ULONGLONG totalSent = 0, totalReceived = 0;

        for (DWORD i = 0; i < pIfTable->dwNumEntries; i++) {
            if (pIfTable->table[i].dwType == IF_TYPE_ETHERNET_CSMACD ||
                pIfTable->table[i].dwType == IF_TYPE_IEEE80211) {
                totalSent += pIfTable->table[i].dwOutOctets;
                totalReceived += pIfTable->table[i].dwInOctets;
            }
        }

        static ULONGLONG prevSent = 0, prevReceived = 0;

        if (prevSent > totalSent)     prevSent = totalSent;
        if (prevReceived > totalReceived) prevReceived = totalReceived;

        currentNetworkUp = (totalSent - prevSent) / 1024.0;
        currentNetworkDown = (totalReceived - prevReceived) / 1024.0;

        prevSent = totalSent;
        prevReceived = totalReceived;
    }
    else {
        currentNetworkUp = 0.0;
        currentNetworkDown = 0.0;
    }

    free(pIfTable);
}

void UpdateNetworkUsage() {
    while (updateFlag) {
        GetNetworkUsage();
        this_thread::sleep_for(chrono::seconds(1));
    }
}