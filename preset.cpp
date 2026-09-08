#include <windows.h>
#include <string>
#include "types.h"
#include "preset.h"

// TransformSettings（main.cpp 側のグローバル）
extern TransformSettings g_transform;

// ------------------------------
// プリセット保存
// ------------------------------
void SavePreset(const std::wstring& name)
{
    const wchar_t* ini = L"presets.ini";
    std::wstring section = L"Preset_" + name;

    WritePrivateProfileString(section.c_str(), L"earMoveScale",
        std::to_wstring(g_transform.earMoveScale).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"browUpY",
        std::to_wstring(g_transform.browUpY).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"browUpRotL",
        std::to_wstring(g_transform.browUpRotL).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"browUpRotR",
        std::to_wstring(g_transform.browUpRotR).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"earringTopRotScale",
        std::to_wstring(g_transform.earringTopRotScale).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"earringBottomRotScale",
        std::to_wstring(g_transform.earringBottomRotScale).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"faceMoveScale",
        std::to_wstring(g_transform.faceMoveScale).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotEarringX",
        std::to_wstring(g_transform.pivotEarringX).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotEarringY",
        std::to_wstring(g_transform.pivotEarringY).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotBrowLX",
        std::to_wstring(g_transform.pivotBrowLX).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotBrowLY",
        std::to_wstring(g_transform.pivotBrowLY).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotBrowRX",
        std::to_wstring(g_transform.pivotBrowRX).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotBrowRY",
        std::to_wstring(g_transform.pivotBrowRY).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotEarX",
        std::to_wstring(g_transform.pivotEarX).c_str(), ini);

    WritePrivateProfileString(section.c_str(), L"pivotEarY",
        std::to_wstring(g_transform.pivotEarY).c_str(), ini);
}

// ------------------------------
// プリセット読み込み
// ------------------------------
void LoadPreset(const std::wstring& name)
{
    const wchar_t* ini = L"presets.ini";
    std::wstring section = L"Preset_" + name;

    auto getf = [&](const wchar_t* key, float def) {
        wchar_t buf[64];
        GetPrivateProfileString(section.c_str(), key, L"", buf, 64, ini);
        if (wcslen(buf) == 0) return def;
        return static_cast<float>(wcstof(buf, nullptr));
        };

    g_transform.earMoveScale = getf(L"earMoveScale", g_transform.earMoveScale);
    g_transform.browUpY = getf(L"browUpY", g_transform.browUpY);
    g_transform.browUpRotL = getf(L"browUpRotL", g_transform.browUpRotL);
    g_transform.browUpRotR = getf(L"browUpRotR", g_transform.browUpRotR);
    g_transform.earringTopRotScale = getf(L"earringTopRotScale", g_transform.earringTopRotScale);
    g_transform.earringBottomRotScale = getf(L"earringBottomRotScale", g_transform.earringBottomRotScale);
    g_transform.faceMoveScale = getf(L"faceMoveScale", g_transform.faceMoveScale);

    g_transform.pivotEarringX = getf(L"pivotEarringX", g_transform.pivotEarringX);
    g_transform.pivotEarringY = getf(L"pivotEarringY", g_transform.pivotEarringY);

    g_transform.pivotBrowLX = getf(L"pivotBrowLX", g_transform.pivotBrowLX);
    g_transform.pivotBrowLY = getf(L"pivotBrowLY", g_transform.pivotBrowLY);

    g_transform.pivotBrowRX = getf(L"pivotBrowRX", g_transform.pivotBrowRX);
    g_transform.pivotBrowRY = getf(L"pivotBrowRY", g_transform.pivotBrowRY);

    g_transform.pivotEarX = getf(L"pivotEarX", g_transform.pivotEarX);
    g_transform.pivotEarY = getf(L"pivotEarY", g_transform.pivotEarY);
}

// ------------------------------
// プリセット削除
// ------------------------------
void DeletePreset(const std::wstring& name)
{
    const wchar_t* ini = L"presets.ini";
    std::wstring section = L"Preset_" + name;

    WritePrivateProfileString(section.c_str(), NULL, NULL, ini);
}

// ------------------------------
// プリセットのエクスポート
// ------------------------------
void ExportPreset(const std::wstring& name)
{
    const wchar_t* srcIni = L"presets.ini";
    std::wstring section = L"Preset_" + name;

    wchar_t fileName[MAX_PATH] = L"";
    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.lpstrFilter = L"Preset INI (*.ini)\0*.ini\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"ini";

    if (!GetSaveFileName(&ofn))
        return;

    WritePrivateProfileSection(section.c_str(), NULL, fileName);

    wchar_t buffer[2048];
    GetPrivateProfileSection(section.c_str(), buffer, 2048, srcIni);
    WritePrivateProfileSection(section.c_str(), buffer, fileName);
}

// ------------------------------
// プリセットのインポート
// ------------------------------
void ImportPreset()
{
    wchar_t fileName[MAX_PATH] = L"";
    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.lpstrFilter = L"Preset INI (*.ini)\0*.ini\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (!GetOpenFileName(&ofn))
        return;

    wchar_t sections[4096];
    GetPrivateProfileSectionNames(sections, 4096, fileName);

    wchar_t* p = sections;
    while (*p)
    {
        std::wstring section = p;

        if (section.rfind(L"Preset_", 0) == 0)
        {
            wchar_t buffer[2048];
            GetPrivateProfileSection(section.c_str(), buffer, 2048, fileName);
            WritePrivateProfileSection(section.c_str(), buffer, L"presets.ini");
        }

        p += wcslen(p) + 1;
    }
}
