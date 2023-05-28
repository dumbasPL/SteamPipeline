#include "SteamPipeServer.h"
#include <Windows.h>
#include <cassert>
#include <chrono>
#include <thread>
#include <vector>
#include <format>

SteamPipeServer::SteamPipeServer(const char* ipcName) {
  const auto eventName = std::format("{}_SharedMemLock", ipcName);
  event = CreateEventA(NULL, FALSE, FALSE, eventName.c_str());
  assert(event != NULL);

  const auto fileMapName = std::format("{}_SharedMemFile", ipcName);
  fileMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 0x400, fileMapName.c_str());
  assert(fileMap != NULL);

  const auto view = MapViewOfFile(fileMap, FILE_MAP_WRITE, 0, 0, 0);
  assert(view != NULL);

  sharedMem = reinterpret_cast<SteamSharedMemory*>(view);

  Reset();
}

SteamPipeServer::~SteamPipeServer() {
  UnmapViewOfFile(sharedMem);
  CloseHandle(event);
  CloseHandle(fileMap);
}

SteamPipeClient SteamPipeServer::AcceptConnection() {
  while (true) {
    switch (sharedMem->MasterState) {
    case 0:
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      break;
    case 1: {
      auto x = AcceptConnectionInternal();
      if (x) {
        return *x.release();
      }
    }
    case 2:
      if (lastAccept + std::chrono::seconds(5) < std::chrono::steady_clock::now()) {
        Reset();
      }
      break;
    case 3:
      Reset();
      break;
    }
  }
}

std::unique_ptr<SteamPipeClient> SteamPipeServer::AcceptConnectionInternal() {
  // try to open the connecting process in order to copy our handles to it
  const HANDLE remoteProcess = OpenProcess(PROCESS_DUP_HANDLE, TRUE, sharedMem->CLProcessId);
  if (remoteProcess) {
    const HANDLE currentProcess = GetCurrentProcess();

    // if we already have a valid handle to the client's read sync event, use it
    HANDLE syncRead = NULL;
    if (sharedMem->Success) {
      syncRead = sharedMem->SyncRead;
    } else {
      if (!DuplicateHandle(remoteProcess, sharedMem->SyncRead, currentProcess, &syncRead, EVENT_ALL_ACCESS, FALSE, 0)) {
        Reset();
        return nullptr;
      }
    }

    // make the connecting process close it's handles because we will be overwriting them
    sharedMem->Success = false;

    auto client = std::make_unique<SteamPipeClient>(syncRead);
    
    CreatePipe(&client->writePipeRead, &client->writePipeWrite, 0, 0x2000);
    CreatePipe(&client->readPipeRead, &client->readPipeWrite, 0, 0x2000);

    DuplicateHandle(currentProcess, client->syncWrite, remoteProcess, &sharedMem->SyncWrite, EVENT_ALL_ACCESS, FALSE, 0);
    DuplicateHandle(currentProcess, client->readPipeRead, remoteProcess, &sharedMem->Input, GENERIC_READ, FALSE, 0);
    DuplicateHandle(currentProcess, client->writePipeWrite, remoteProcess, &sharedMem->Output, GENERIC_WRITE, FALSE, 0);
    sharedMem->SVProcessId = GetCurrentProcessId();
    CloseHandle(remoteProcess);

    sharedMem->Connected = 1;
    sharedMem->MasterState = 2;
    lastAccept = std::chrono::steady_clock::now();
    return client;
  }

  // if we failed to open the connecting process, and
  // the connecting process also failed to open us, abort
  if (!sharedMem->Success) {
    Reset();
    return nullptr;
  }

  // construct using handles provided to us by the connecting process
  auto client = std::make_unique<SteamPipeClient>(sharedMem->SyncRead, sharedMem->SyncWrite);
  client->writePipeRead = sharedMem->Input;
  client->readPipeWrite = sharedMem->Output;

  sharedMem->Connected = 1;
  sharedMem->MasterState = 2;
  lastAccept = std::chrono::steady_clock::now();
  return client;
}

void SteamPipeServer::Reset() {
  ZeroMemory(sharedMem, sizeof(SteamSharedMemory));
  sharedMem->SVProcessId = GetCurrentProcessId();
  SetEvent(event);
}
