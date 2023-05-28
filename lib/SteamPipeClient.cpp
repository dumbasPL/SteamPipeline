#include "SteamPipeClient.h"
#include <Windows.h>
#include <cassert>
#include <format>

SteamPipeClient::SteamPipeClient() {
  syncRead = CreateEventA(NULL, TRUE, FALSE, NULL);
  syncWrite = CreateEventA(NULL, TRUE, FALSE, NULL);
}

SteamPipeClient::SteamPipeClient(HANDLE syncRead) : syncRead(syncRead) {
  syncWrite = CreateEventA(NULL, TRUE, FALSE, NULL);
}

SteamPipeClient::SteamPipeClient(HANDLE syncRead, HANDLE syncWrite) : syncRead(syncRead), syncWrite(syncWrite) {}

SteamPipeClient::~SteamPipeClient() {
  CloseHandle(readPipeRead);
  CloseHandle(readPipeWrite);
  CloseHandle(writePipeRead);
  CloseHandle(writePipeWrite);
  CloseHandle(syncRead);
  CloseHandle(syncWrite);
}

bool SteamPipeClient::Connect(const char *ipcName) {
  const auto eventName = std::format("{}_SharedMemLock", ipcName);
  const HANDLE event = OpenEventA(SYNCHRONIZE, FALSE, eventName.c_str());
  if (!event)
    return false;

  const auto fileMapName = std::format("{}_SharedMemFile", ipcName);
  const HANDLE fileMap = OpenFileMappingA(FILE_MAP_WRITE, FALSE, fileMapName.c_str());
  if (!fileMap) {
    CloseHandle(event);
    return false;
  }

  const DWORD wait = WaitForSingleObject(event, 5000);
  if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED) {
    CloseHandle(fileMap);
    CloseHandle(event);
    return false;
  }

  const auto view = MapViewOfFile(fileMap, FILE_MAP_WRITE, 0, 0, 0);
  if (!view) {
    CloseHandle(fileMap);
    CloseHandle(event);
    return false;
  }

  const auto sharedMem = reinterpret_cast<SteamSharedMemory*>(view);
  assert(sharedMem->MasterState == 0);

  sharedMem->CLProcessId = GetCurrentProcessId();
  sharedMem->Input = NULL;
  sharedMem->Output = NULL;

  // if DuplicateHandle fails in this client, the server will clone this value
  sharedMem->SyncRead = syncRead;

  const HANDLE remoteProcess = OpenProcess(PROCESS_DUP_HANDLE, TRUE, sharedMem->SVProcessId);
  if (remoteProcess) {
    const HANDLE currentProcess = GetCurrentProcess();
    BOOL success = DuplicateHandle(currentProcess, syncWrite, remoteProcess, &sharedMem->SyncWrite, EVENT_ALL_ACCESS, FALSE, 0);
    if (success) {
      success &= DuplicateHandle(currentProcess, syncRead, remoteProcess, &sharedMem->SyncRead, EVENT_ALL_ACCESS, FALSE, 0);

      success &= CreatePipe(&writePipeRead, &writePipeWrite, 0, 0x2000);
      success &= DuplicateHandle(currentProcess, writePipeWrite, remoteProcess, &sharedMem->Output, GENERIC_WRITE, FALSE, 0);

      success &= CreatePipe(&readPipeRead, &readPipeWrite, 0, 0x2000);
      success &= DuplicateHandle(currentProcess, readPipeRead, remoteProcess, &sharedMem->Input, GENERIC_READ, FALSE, 0);

      sharedMem->Success = success != 0;
    }
    CloseHandle(remoteProcess);
  }

  sharedMem->MasterState = 1;
  int waited = 0;
  while (sharedMem->MasterState != 2) {
    if (waited++ >= 5000)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
  }

  if (sharedMem->MasterState != 2) {
    UnmapViewOfFile(view);
    CloseHandle(fileMap);
    CloseHandle(event);
    return false;
  }

  if (!sharedMem->Success) {
    CloseHandle(readPipeRead);
    readPipeRead = INVALID_HANDLE_VALUE;
    CloseHandle(readPipeWrite);
    readPipeWrite = INVALID_HANDLE_VALUE;
    CloseHandle(writePipeRead);
    writePipeRead = INVALID_HANDLE_VALUE;
    CloseHandle(writePipeWrite);
    writePipeWrite = INVALID_HANDLE_VALUE;

    CloseHandle(syncWrite);
    syncWrite = INVALID_HANDLE_VALUE;

    writePipeRead = sharedMem->Input;
    readPipeWrite = sharedMem->Output;
    syncWrite = sharedMem->SyncWrite;
  }

  sharedMem->MasterState = 3;

  UnmapViewOfFile(view);
  CloseHandle(fileMap);
  CloseHandle(event);
  return true;
}
