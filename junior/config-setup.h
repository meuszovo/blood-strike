#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#undef GetPrivateProfileInt
#define GetPrivateProfileInt GetPrivateProfileIntA
#include <string>
#include <stdio.h>

inline BOOL WritePrivateProfileInt(LPCSTR lpAppName, LPCSTR lpKeyName, int nInteger, LPCSTR lpFileName) {
    char lpString[1024];
    sprintf_s(lpString, "%d", nInteger);
    return WritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);
}

inline BOOL WritePrivateProfileFloat(LPCSTR lpAppName, LPCSTR lpKeyName, float nInteger, LPCSTR lpFileName) {
    char lpString[1024];
    sprintf_s(lpString, "%f", nInteger);
    return WritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);
}

inline float GetPrivateProfileFloat(LPCSTR lpAppName, LPCSTR lpKeyName, float flDefault, LPCSTR lpFileName)
{
    char szData[32];
    GetPrivateProfileStringA(lpAppName, lpKeyName, std::to_string(flDefault).c_str(), szData, 32, lpFileName);
    return (float)atof(szData);
}
