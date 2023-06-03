#include "SteamPipeClient.h"
#include <Windows.h>
#include <cassert>
#include <format>
#include <algorithm>
#include <array>

#pragma pack(push, 1)
struct MessageHeader {
  DWORD size;
  uint8_t type;
};

static_assert(sizeof(MessageHeader) == 5);
#pragma pack(pop)

SteamPipeClient::SteamPipeClient() {
  syncRead = CreateEventA(NULL, TRUE, FALSE, NULL);
  syncWrite = CreateEventA(NULL, TRUE, FALSE, NULL);
}

SteamPipeClient::SteamPipeClient(HANDLE syncRead) : syncRead(syncRead) {
  syncWrite = CreateEventA(NULL, TRUE, FALSE, NULL);
}

SteamPipeClient::SteamPipeClient(HANDLE syncRead, HANDLE syncWrite) : syncRead(syncRead), syncWrite(syncWrite) {}

SteamPipeClient::~SteamPipeClient() {
  Destroy();
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
  sharedMem->SyncRead = syncWrite;

  const HANDLE remoteProcess = OpenProcess(PROCESS_DUP_HANDLE, TRUE, sharedMem->SVProcessId);
  if (remoteProcess) {
    const HANDLE currentProcess = GetCurrentProcess();
    BOOL success = DuplicateHandle(currentProcess, syncRead, remoteProcess, &sharedMem->SyncWrite, EVENT_ALL_ACCESS, FALSE, 0);
    if (success) {
      success &= DuplicateHandle(currentProcess, syncWrite, remoteProcess, &sharedMem->SyncRead, EVENT_ALL_ACCESS, FALSE, 0);

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
    CloseHandle(readPipeWrite);
    readPipeWrite = INVALID_HANDLE_VALUE;

    CloseHandle(writePipeRead);
    writePipeRead = INVALID_HANDLE_VALUE;

    CloseHandle(writePipeWrite);
    writePipeWrite = INVALID_HANDLE_VALUE;

    CloseHandle(readPipeRead);
    readPipeRead = INVALID_HANDLE_VALUE;

    CloseHandle(syncRead);
    syncRead = INVALID_HANDLE_VALUE;

    writePipeRead = sharedMem->Input;
    readPipeWrite = sharedMem->Output;
    syncRead = sharedMem->SyncWrite;
  }

  sharedMem->MasterState = 3;
  remoteProcessId = sharedMem->SVProcessId;

  UnmapViewOfFile(view);
  CloseHandle(fileMap);
  CloseHandle(event);
  return true;
}

void SteamPipeClient::Destroy() {
  CloseHandle(readPipeRead);
  CloseHandle(readPipeWrite);
  CloseHandle(writePipeRead);
  CloseHandle(writePipeWrite);
  CloseHandle(syncRead);
  CloseHandle(syncWrite);
  remoteProcessId = -1;
}

inline void PushVector(std::vector<uint8_t> &data, const void *ptr, size_t size) {
  data.insert(data.end(), (uint8_t*)ptr, (uint8_t*)ptr + size);
}

bool SteamPipeClient::ReadInternal(std::vector<uint8_t> &data) {
  DWORD read;
  MessageHeader header;
  if (!ReadFile(writePipeRead, &header, sizeof(header), &read, NULL) || read != sizeof(header))
    return false;

  PushVector(data, &header, sizeof(header));

  DWORD totalRead = 0;
  DWORD sizeToRead = header.size - sizeof(header.type);
  std::array<uint8_t, 1024> buffer;
  while (totalRead < sizeToRead) {
    DWORD toRead = std::min<DWORD>(sizeToRead - totalRead, buffer.size());
    if (!ReadFile(writePipeRead, buffer.data(), toRead, &read, NULL))
      return false;
    PushVector(data, buffer.data(), read);
    totalRead += read;
  }

  return true;
}

bool SteamPipeClient::Peak(uint8_t* type) {
  DWORD BytesRead = 0, TotalBytesAvail = 0, BytesLeftThisMessage = 0;
  MessageHeader header;
  if (!PeekNamedPipe(writePipeRead, &header, type ? sizeof(header) : 0, &BytesRead, &TotalBytesAvail, &BytesLeftThisMessage))
    return false;
  
  if (!TotalBytesAvail)
    return false;

  if (type)
    *type = header.type;

  return true;
}

#include <iostream>

bool SteamPipeClient::Read(std::vector<uint8_t> &data) {
  // NOTE: Fuck the event, we are not part of a game loop so 
  //       we can just wait and block on file reads
  // WaitForSingleObject(syncRead, INFINITE);

  while (true) {
    if (!ReadInternal(data))
      return false;
    
    if (!Peak(NULL))
      break;
  }

  ResetEvent(syncRead);
  return true;
}

bool SteamPipeClient::Write(std::vector<uint8_t> &data) {
  DWORD written;

  MessageHeader* header = (MessageHeader*)data.data();
  if (!WriteFile(readPipeWrite, header, sizeof(MessageHeader), &written, NULL))
    return false;

  bool notify = true;

  size_t totalWritten = sizeof(MessageHeader);
  while (totalWritten < data.size()) {
    DWORD toWrite = std::min<DWORD>(data.size() - totalWritten, notify ? 1024 : 0x100000);
    if (!WriteFile(readPipeWrite, data.data() + totalWritten, toWrite, &written, NULL))
      return false;
    totalWritten += written;
    if (notify) {
      SetEvent(syncWrite);
      notify = false;
    }
  }

  if (notify)
    SetEvent(syncWrite);

  return true;
}
