#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdint>

namespace mem
{
    inline HANDLE hProcess = nullptr;
    inline DWORD   pid = 0;
    inline uintptr_t base = 0;

    DWORD FindProcessId(const wchar_t* name)
    {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return 0;
        PROCESSENTRY32W pe{};
        pe.dwSize = sizeof(pe);
        if (Process32First(snap, &pe))
        {
            do
            {
                if (_wcsicmp(pe.szExeFile, name) == 0)
                {
                    CloseHandle(snap);
                    return pe.th32ProcessID;
                }
            } while (Process32Next(snap, &pe));
        }
        CloseHandle(snap);
        return 0;
    }

    uintptr_t FindModuleBase(DWORD pid, const wchar_t* modName)
    {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) return 0;
        MODULEENTRY32W me{};
        me.dwSize = sizeof(me);
        if (Module32First(snap, &me))
        {
            do
            {
                if (_wcsicmp(me.szModule, modName) == 0)
                {
                    CloseHandle(snap);
                    return (uintptr_t)me.modBaseAddr;
                }
            } while (Module32Next(snap, &me));
        }
        CloseHandle(snap);
        return 0;
    }

    bool Attach(const wchar_t* processName)
    {
        pid = FindProcessId(processName);
        if (!pid) return false;
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (!hProcess) return false;
        base = FindModuleBase(pid, processName);
        if (!base) return false;
        return true;
    }

    template<typename T>
    T Read(uintptr_t addr)
    {
        T val{};
        ReadProcessMemory(hProcess, (LPCVOID)addr, &val, sizeof(T), nullptr);
        return val;
    }

    template<typename T>
    bool Write(uintptr_t addr, const T& val)
    {
        return WriteProcessMemory(hProcess, (LPVOID)addr, &val, sizeof(T), nullptr);
    }

    template<typename T>
    T ReadChain(uintptr_t baseAddr, std::initializer_list<uintptr_t> offsets)
    {
        uintptr_t addr = Read<uintptr_t>(baseAddr);
        for (size_t i = 0; i + 1 < offsets.size(); i++)
        {
            if (!addr) return T{};
            addr = Read<uintptr_t>(addr + offsets.begin()[i]);
        }
        if (!addr) return T{};
        return Read<T>(addr + offsets.begin()[offsets.size() - 1]);
    }
}
