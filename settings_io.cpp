#include <windows.h>
#include <string>
#include <map>
#include "types.h"

#include "settings_io.h"

// main.cpp 側のグローバル変数を参照
extern std::map<std::string, std::wstring> g_bitmapFiles;
extern TransformSettings g_transform;

// ------------------------------
// float 読み込み（main.cpp と同じ）
// ------------------------------
float GetPrivateProfileFloat(const wchar_t* section, const wchar_t* key, float def, const wchar_t* ini)
{
    wchar_t buf[64];
    GetPrivateProfileString(section, key, L"", buf, 64, ini);
    if (wcslen(buf) == 0) return def;
    return static_cast<float>(wcstof(buf, nullptr));
}

// ------------------------------
// settings.ini 保存
// ------------------------------
void SaveSettings()
{
    const wchar_t* ini = L"settings.ini";

    // PNG ファイルパス保存
    for (auto& kv : g_bitmapFiles)
    {
        std::wstring key(kv.first.begin(), kv.first.end());
        WritePrivateProfileString(L"Bitmaps", key.c_str(), kv.second.c_str(), ini);
    }

    // TransformSettings 保存
    WritePrivateProfileString(L"Transform", L"earMoveScale",
        std::to_wstring(g_transform.earMoveScale).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"browUpY",
        std::to_wstring(g_transform.browUpY).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"browUpRotL",
        std::to_wstring(g_transform.browUpRotL).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"browUpRotR",
        std::to_wstring(g_transform.browUpRotR).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"earringTopRotScale",
        std::to_wstring(g_transform.earringTopRotScale).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"earringBottomRotScale",
        std::to_wstring(g_transform.earringBottomRotScale).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"faceMoveScale",
        std::to_wstring(g_transform.faceMoveScale).c_str(), ini);

    // pivot
    WritePrivateProfileString(L"Transform", L"pivotEarringX",
        std::to_wstring(g_transform.pivotEarringX).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"pivotEarringY",
        std::to_wstring(g_transform.pivotEarringY).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"pivotBrowLX",
        std::to_wstring(g_transform.pivotBrowLX).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"pivotBrowLY",
        std::to_wstring(g_transform.pivotBrowLY).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"pivotBrowRX",
        std::to_wstring(g_transform.pivotBrowRX).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"pivotBrowRY",
        std::to_wstring(g_transform.pivotBrowRY).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"pivotEarX",
        std::to_wstring(g_transform.pivotEarX).c_str(), ini);

    WritePrivateProfileString(L"Transform", L"pivotEarY",
        std::to_wstring(g_transform.pivotEarY).c_str(), ini);
}

// ------------------------------
// settings.ini 読み込み
// ------------------------------
void LoadSettings()
{
    const wchar_t* ini = L"settings.ini";
    wchar_t buf[MAX_PATH];

    // PNG パス読み込み
    for (auto& kv : g_bitmapFiles)
    {
        std::wstring key(kv.first.begin(), kv.first.end());

        GetPrivateProfileString(
            L"Bitmaps",
            key.c_str(),
            L"",
            buf,
            MAX_PATH,
            ini
        );

        if (wcslen(buf) > 0)
            kv.second = buf;
    }

    // TransformSettings 読み込み
    g_transform.earMoveScale =
        GetPrivateProfileFloat(L"Transform", L"earMoveScale", g_transform.earMoveScale, ini);

    g_transform.browUpY =
        GetPrivateProfileFloat(L"Transform", L"browUpY", g_transform.browUpY, ini);

    g_transform.browUpRotL =
        GetPrivateProfileFloat(L"Transform", L"browUpRotL", g_transform.browUpRotL, ini);

    g_transform.browUpRotR =
        GetPrivateProfileFloat(L"Transform", L"browUpRotR", g_transform.browUpRotR, ini);

    g_transform.earringTopRotScale =
        GetPrivateProfileFloat(L"Transform", L"earringTopRotScale", g_transform.earringTopRotScale, ini);

    g_transform.earringBottomRotScale =
        GetPrivateProfileFloat(L"Transform", L"earringBottomRotScale", g_transform.earringBottomRotScale, ini);

    g_transform.faceMoveScale =
        GetPrivateProfileFloat(L"Transform", L"faceMoveScale", g_transform.faceMoveScale, ini);

    g_transform.pivotEarringX =
        GetPrivateProfileFloat(L"Transform", L"pivotEarringX", g_transform.pivotEarringX, ini);

    g_transform.pivotEarringY =
        GetPrivateProfileFloat(L"Transform", L"pivotEarringY", g_transform.pivotEarringY, ini);

    g_transform.pivotBrowLX =
        GetPrivateProfileFloat(L"Transform", L"pivotBrowLX", g_transform.pivotBrowLX, ini);

    g_transform.pivotBrowLY =
        GetPrivateProfileFloat(L"Transform", L"pivotBrowLY", g_transform.pivotBrowLY, ini);

    g_transform.pivotBrowRX =
        GetPrivateProfileFloat(L"Transform", L"pivotBrowRX", g_transform.pivotBrowRX, ini);

    g_transform.pivotBrowRY =
        GetPrivateProfileFloat(L"Transform", L"pivotBrowRY", g_transform.pivotBrowRY, ini);

    g_transform.pivotEarX =
        GetPrivateProfileFloat(L"Transform", L"pivotEarX", g_transform.pivotEarX, ini);

    g_transform.pivotEarY =
        GetPrivateProfileFloat(L"Transform", L"pivotEarY", g_transform.pivotEarY, ini);

    SaveSettings();
}
