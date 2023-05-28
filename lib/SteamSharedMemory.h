#pragma once
#include <windows.h>

struct SteamSharedMemory {
public:
    int Connected;
    // SteamPipe initialization state.
    //   0 - Available for initialization; set by client
    //   1 - Connected to server's pipe and waiting for client-side init; set by server
    //   2 - Client-side performed its initialization; set by client
    //   3 - Server-side performed some handle checks; set by server
    int MasterState;

    int SVProcessId; // Server's process identifier
    int CLProcessId; // Client's process identifier

    HANDLE Input; // Handle for reading pipe handle
    HANDLE Output; // Handle for writing pipe handle

    HANDLE SyncRead; // Read sync event
    HANDLE SyncWrite; // Write sync event

    bool Success; // Successfully initialized all handles
};