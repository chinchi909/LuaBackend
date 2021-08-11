#ifndef MEMORYLIB
#define MEMORYLIB

#include <windows.h>
#include <iostream>
#include <string>
#include <psapi.h>
#include "TlHelp32.h"

#include <algorithm>
#include <type_traits>
#include <utility>

using namespace std;

class MemoryLib
{
    private:
        static inline STARTUPINFOA _sInfo;
        static inline PROCESS_INFORMATION _pInfo;
        static inline bool _bigEndian = false;

        class protect_lock {
        private:
            uintptr_t address;
            size_t size;
            DWORD protection;
            bool is_acquired;

        public:
            protect_lock(uintptr_t address, size_t size) : address(address), size(size), protection(0), is_acquired(false) {
                if (VirtualProtect((void*)address, size, PAGE_READWRITE, &protection) != 0)
                    is_acquired = true;
            }

            ~protect_lock() {
                if (good())
                    VirtualProtect((void*)address, size, protection, &protection);
            }

            bool good() const noexcept {
                return is_acquired;
            }
        };

    public:
        static inline uint64_t ExecAddress;
        static inline uint64_t BaseAddress;
        static inline DWORD PIdentifier = NULL;
        static inline HANDLE PHandle = NULL;
        static inline char PName[MAX_PATH];

    static HMODULE FindBaseAddr(HANDLE InputHandle, string InputName)
    {
        HMODULE hMods[1024];
        DWORD cbNeeded;
        unsigned int i;

        if (EnumProcessModules(InputHandle, hMods, sizeof(hMods), &cbNeeded))
        {
            for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                TCHAR szModName[MAX_PATH];
                if (GetModuleFileNameEx(InputHandle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
                {
                    wstring wstrModName = szModName;
                    wstring wstrModContain = wstring(InputName.begin(), InputName.end());

                    if (wstrModName.find(wstrModContain) != string::npos)
                        return hMods[i];
                }
            }
        }

        return nullptr;
    }

    static DWORD FindProcessId(const std::wstring& processName)
    {
        PROCESSENTRY32 processInfo;
        processInfo.dwSize = sizeof(processInfo);

        HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (processesSnapshot == INVALID_HANDLE_VALUE)
            return 0;

        Process32First(processesSnapshot, &processInfo);
        if (!processName.compare(processInfo.szExeFile))
        {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }

        while (Process32Next(processesSnapshot, &processInfo))
        {
            if (!processName.compare(processInfo.szExeFile))
            {
                CloseHandle(processesSnapshot);
                return processInfo.th32ProcessID;
            }
        }

        CloseHandle(processesSnapshot);
        return 0;
    }

    static void SetBaseAddr(uint64_t InputAddress)
    {
        BaseAddress = InputAddress;
    }

    static int ExecuteProcess(string InputName, uint64_t InputAddress, bool InputEndian)
    {
        ZeroMemory(&_sInfo, sizeof(_sInfo)); _sInfo.cb = sizeof(_sInfo);
        ZeroMemory(&_pInfo, sizeof(_pInfo));

        if (CreateProcessA(InputName.c_str(), NULL, NULL, NULL, TRUE, 0, NULL, NULL, &_sInfo, &_pInfo) == 0)
            return -1;

        BaseAddress = InputAddress;
        _bigEndian = InputEndian;

        return 0;
    };

    static bool LatchProcess(string InputName, uint64_t InputAddress, bool InputEndian)
    {
        ZeroMemory(&_sInfo, sizeof(_sInfo)); _sInfo.cb = sizeof(_sInfo);
        ZeroMemory(&_pInfo, sizeof(_pInfo));

        PIdentifier = FindProcessId(wstring(InputName.begin(), InputName.end()));
        PHandle = OpenProcess(PROCESS_ALL_ACCESS, false, PIdentifier);

        GetProcessImageFileNameA(MemoryLib::PHandle, PName, MAX_PATH);
        BaseAddress = InputAddress;

        ExecAddress = (uint64_t)FindBaseAddr(PHandle, PName);
        _bigEndian = InputEndian;

        if (PHandle == NULL)
            return false;

        return true;
    };

    static void ExternProcess(DWORD InputID, HANDLE InputH, uint64_t InputAddress)
    {
        PIdentifier = InputID;
        PHandle = InputH;

        GetProcessImageFileNameA(MemoryLib::PHandle, PName, MAX_PATH);

        BaseAddress = InputAddress;
        ExecAddress = (uint64_t)FindBaseAddr(PHandle, PName);
    };

    template <typename T>
    static T readScalar(uint64_t Address, bool absolute = false) {
        if (absolute) {
            return readScalarAbsolute<T>(Address);
        } else {
            return readScalarAbsolute<T>(Address + BaseAddress);
        }
    }

    template <typename T, std::enable_if_t<std::is_trivially_constructible_v<T>, int> = 0>
    static T readScalarAbsolute(uint64_t Address) {
        protect_lock lock(Address, sizeof(T));
        if (!lock.good())
            return 0;

        return *reinterpret_cast<T*>(Address);
    }

    template <typename T>
    static void writeScalar(uint64_t Address, T const& t, bool absolute = false) {
        if (absolute) {
            writeScalarAbsolute<T>(Address, t);
        } else {
            writeScalarAbsolute<T>(Address + BaseAddress, t);
        }
    }

    template <typename T, std::enable_if_t<std::is_trivially_copy_assignable_v<T>, int> = 0>
    static void writeScalarAbsolute(uint64_t Address, T t) {
        protect_lock lock(Address, sizeof(T));
        if (!lock.good())
            return;

        *reinterpret_cast<T*>(Address) = t;
    }

    static uint8_t ReadByte(uint64_t Address, bool absolute = false) { return readScalar<uint8_t>(Address, absolute); }
    static uint16_t ReadShort(uint64_t Address, bool absolute = false) { return readScalar<uint16_t>(Address, absolute); }
    static uint32_t ReadInt(uint64_t Address, bool absolute = false) { return readScalar<uint32_t>(Address, absolute); }
    static uint64_t ReadLong(uint64_t Address, bool absolute = false) { return readScalar<uint64_t>(Address, absolute); }
    static float ReadFloat(uint64_t Address, bool absolute = false) { return readScalar<float>(Address, absolute); }
    static bool ReadBool(uint64_t Address, bool absolute = false) { return ReadByte(Address, absolute) != 0; }

    static vector<uint8_t> ReadBytes(uint64_t Address, int Length, bool absolute = false) {
        if (absolute) {
            return ReadBytesAbsolute(Address, Length);
        } else {
            return ReadBytesAbsolute(Address + BaseAddress, Length);
        }
    }

    static string ReadString(uint64_t Address, int Length, bool absolute = false) {
        if (absolute) {
            return ReadStringAbsolute(Address, Length);
        } else {
            return ReadStringAbsolute(Address + BaseAddress, Length);
        }
    }

    static void WriteByte(uint64_t Address, uint8_t Input, bool absolute = false) { writeScalar<uint8_t>(Address, Input, absolute); }
    static void WriteShort(uint64_t Address, uint16_t Input, bool absolute = false) { writeScalar<uint16_t>(Address, Input, absolute); }
    static void WriteInt(uint64_t Address, uint32_t Input, bool absolute = false) { writeScalar<uint32_t>(Address, Input, absolute); }
    static void WriteLong(uint64_t Address, uint64_t Input, bool absolute = false) { writeScalar<uint64_t>(Address, Input, absolute); }
    static void WriteFloat(uint64_t Address, float Input, bool absolute = false) { writeScalar<float>(Address, Input, absolute); }
    static void WriteBool(uint64_t Address, bool Input, bool absolute = false) { WriteByte(Address, Input ? 1 : 0, absolute); }

    static void WriteBytes(uint64_t Address, vector<uint8_t> Input, bool absolute = false) {
        if (absolute) {
            WriteBytesAbsolute(Address, std::move(Input));
        } else {
            WriteBytesAbsolute(Address + BaseAddress, std::move(Input));
        }
    }

    static void WriteString(uint64_t Address, string Input, bool absolute = false) {
        if (absolute) {
            WriteStringAbsolute(Address, std::move(Input));
        } else {
            WriteStringAbsolute(Address + BaseAddress, std::move(Input));
        }
    }

    static uint8_t ReadByteAbsolute(uint64_t Address) { return readScalarAbsolute<uint8_t>(Address); }
    static uint16_t ReadShortAbsolute(uint64_t Address) { return readScalarAbsolute<uint16_t>(Address); }
    static uint32_t ReadIntAbsolute(uint64_t Address) { return readScalarAbsolute<uint32_t>(Address); }
    static uint64_t ReadLongAbsolute(uint64_t Address) { return readScalarAbsolute<uint64_t>(Address); }
    static float ReadFloatAbsolute(uint64_t Address) { return readScalarAbsolute<float>(Address); }
    static bool ReadBoolAbsolute(uint64_t Address) { return ReadByteAbsolute(Address) != 0; }

    static vector<uint8_t> ReadBytesAbsolute(uint64_t Address, int Length) {
        vector<uint8_t> _buffer;

        protect_lock lock(Address, static_cast<size_t>(Length));
        if (lock.good()) {
            _buffer.resize(Length);
            std::memcpy(_buffer.data(), (void*)Address, Length);
        }

        return _buffer;
    }

    static string ReadStringAbsolute(uint64_t Address, int Length) {
        string _output;

        protect_lock lock(Address, static_cast<size_t>(Length));
        if (lock.good()) {
            _output.resize(Length);
            std::memcpy(_output.data(), (void*)Address, Length);
        }

        return _output;
    }

    static void WriteByteAbsolute(uint64_t Address, uint8_t Input) { writeScalarAbsolute<uint8_t>(Address, Input); }
    static void WriteShortAbsolute(uint64_t Address, uint16_t Input) { writeScalarAbsolute<uint16_t>(Address, Input); }
    static void WriteIntAbsolute(uint64_t Address, uint32_t Input) { writeScalarAbsolute<uint32_t>(Address, Input); }
    static void WriteLongAbsolute(uint64_t Address, uint64_t Input) { writeScalarAbsolute<uint64_t>(Address, Input); }
    static void WriteFloatAbsolute(uint64_t Address, float Input) { writeScalarAbsolute<float>(Address, Input); }
    static void WriteBoolAbsolute(uint64_t Address, bool Input) { WriteByteAbsolute(Address, Input ? 1 : 0); }

    static void WriteBytesAbsolute(uint64_t Address, vector<uint8_t> Input) {
        protect_lock lock(Address, Input.size());
        if (lock.good())
            std::memcpy((void*)Address, Input.data(), Input.size());
    }

    static void WriteStringAbsolute(uint64_t Address, string Input) {
        protect_lock lock(Address, Input.size());
        if (lock.good())
            std::memcpy((void*)Address, Input.data(), Input.size());
    }

    static void WriteExec(uint64_t Address, vector<uint8_t> Input) {
        protect_lock lock(Address, Input.size());
        if (lock.good())
            std::memcpy((void*)(Address + ExecAddress), Input.data(), Input.size());
    }

    static uint64_t GetPointer(uint64_t Address, uint64_t Offset, bool absolute = false) {
        uint64_t _temp = ReadLong(Address, absolute);
        return _temp + Offset;
    }

    static uint64_t GetPointerAbsolute(uint64_t Address, uint64_t Offset) {
        uint64_t _temp = ReadLongAbsolute(Address);
        return _temp + Offset;
    }
};
#endif
