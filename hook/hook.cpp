#include <Windows.h>
#include <shlwapi.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include "RTTI.h"
#include "minhook/MinHook.h"

#ifdef _WIN32
#define TC_ECX void* edx,
#define STEAMCLIENT_DLL L"steamclient.dll"
#else
#define TC_ECX
#define STEAMCLIENT_DLL L"steamclient64.dll"
#endif

template <class T>
class utl_memory {
public:
    T* memory;
    int allocation_count;
    int grow_size;
};

template <class T>
class utl_buffer {
public:
    void* unk;
    utl_memory<T> memory;
    int m_Get;
    int m_Put;
};

std::string hexStr(uint8_t* data, int len) {
  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < len; ++i)
    ss << std::setw(2) << std::setfill('0') << (int)data[i];
  return ss.str();
}

typedef void*(__thiscall* ReadPipe_t)(void* ecx, utl_buffer<uint8_t>* buffer);
ReadPipe_t ReadPipe_o;
bool __fastcall ReadPipe(void* ecx, TC_ECX utl_buffer<uint8_t>* buffer) {
  std::cout << "[HOOK] ReadStart" << std::endl;
  bool ret = ReadPipe_o(ecx, buffer);
  std::cout << "[HOOK] Read:  " << hexStr(buffer->memory.memory, buffer->m_Put) << std::endl;
  return ret;
}

typedef bool(__thiscall* PeakPipe_t)(void* ecx, uint8_t* command);
PeakPipe_t PeakPipe_o;
bool __fastcall PeakPipe(void* ecx, TC_ECX uint8_t* command) {
  bool ret = PeakPipe_o(ecx, command);
  if (ret)
    std::cout << "[HOOK] Peak:  " << ret << " " << (command ? (int)*command : -1) << std::endl;
  return ret;
}

typedef void*(__thiscall* WritePipe_t)(void* ecx, utl_buffer<uint8_t>* buffer, bool notify);
WritePipe_t WritePipe_o;
bool __fastcall WritePipe(void* ecx, TC_ECX utl_buffer<uint8_t>* buffer, bool notify) {
  std::cout << "[HOOK] Write: " << hexStr(buffer->memory.memory, buffer->m_Put) << " " << notify << std::endl;
  return WritePipe_o(ecx, buffer, notify);
}

typedef bool(__thiscall* WaitMsg_t)(void* ecx, DWORD timeout);
WaitMsg_t WaitMsg_o;
bool __fastcall WaitMsg(void* ecx, TC_ECX DWORD timeout) {
  std::cout << "[HOOK] WaitStart: " << timeout << std::endl;
  bool ret = WaitMsg_o(ecx, timeout);
  std::cout << "[HOOK] WaitMsg: " << ret << std::endl;
  return ret;
}

typedef bool(__thiscall* ResetEventS_t)(void* ecx);
ResetEventS_t ResetEventS_o;
bool __fastcall ResetEventS(void* ecx) {
  bool ret = ResetEventS_o(ecx);
  std::cout << "[HOOK] ResetEvent: " << ret << std::endl;
  return ret;
}

typedef void(__thiscall* SetEventS_t)(void* ecx, bool set);
SetEventS_t SetEventS_o;
void __fastcall SetEventS(void* ecx, TC_ECX bool set) {
  std::cout << "[HOOK] SetEvent: " << set << std::endl;
  SetEventS_o(ecx, set);
}

bool init(HMODULE hSteamClient) {
  void** table = (void**)FindRTTI(hSteamClient, ".?AVCCrossProcessPipe@@");
  if (!table) {
    std::cout << "Failed to find CCrossProcessPipe VTable" << std::endl;
    return false;
  }
  std::cout << "CCrossProcessPipe VTable: " << table << std::endl;

  if (MH_CreateHook(table[5], &WritePipe, (void**)&WritePipe_o) != MH_OK) {
    std::cout << "Failed to hook CCrossProcessPipe::WritePipe" << std::endl;
    return false;
  }

  if (MH_CreateHook(table[6], &PeakPipe, (void**)&PeakPipe_o) != MH_OK) {
    std::cout << "Failed to hook CCrossProcessPipe::PeakPipe" << std::endl;
    return false;
  }

  if (MH_CreateHook(table[7], &ReadPipe, (void**)&ReadPipe_o) != MH_OK) {
    std::cout << "Failed to hook CCrossProcessPipe::ReadPipe" << std::endl;
    return false;
  }

  if (MH_CreateHook(table[8], &WaitMsg, (void**)&WaitMsg_o) != MH_OK) {
    std::cout << "Failed to hook CCrossProcessPipe::WaitMsg" << std::endl;
    return false;
  }

  if (MH_CreateHook(table[9], &ResetEventS, (void**)&ResetEventS_o) != MH_OK) {
    std::cout << "Failed to hook CCrossProcessPipe::ResetEvent" << std::endl;
    return false;
  }

  if (MH_CreateHook(table[10], &SetEventS, (void**)&SetEventS_o) != MH_OK) {
    std::cout << "Failed to hook CCrossProcessPipe::SetEvent" << std::endl;
    return false;
  }

  if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
    std::cout << "Failed to enable hooks" << std::endl;
    return false;
  }

  std::cout << "Hooks enabled" << std::endl;
  return true;
}

typedef HMODULE(WINAPI* LoadLibraryExW_t)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExW_t LoadLibraryExW_o;
HMODULE WINAPI LoadLibraryExW_h(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
  HMODULE ret = LoadLibraryExW_o(lpLibFileName, hFile, dwFlags);
  if (wcscmp(PathFindFileNameW(lpLibFileName), STEAMCLIENT_DLL) == 0) {
    init(ret);
  }
  return ret;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(hinstDLL);
    if (MH_Initialize() != MH_OK)
      return FALSE;
    if (MH_CreateHookApi(L"kernel32.dll", "LoadLibraryExW", LoadLibraryExW_h, (void**)&LoadLibraryExW_o) != MH_OK)
      return FALSE;
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
      return FALSE;
    break;
  case DLL_PROCESS_DETACH:
    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
      return FALSE;
    if (MH_Uninitialize() != MH_OK)
      return FALSE;
    break;
  }
  return TRUE;
}
 