#include "project_io.h"
#include <windows.h>
#include <string>
#include <vector>
#include <map>

#include <ShlObj.h>
#include <ShlDisp.h>
#include <comdef.h>

#include <atlbase.h>
#include <atlcom.h>

#include "LayerManager.h"
#include "settings_io.h"
#include "types.h"


// グローバル
extern LayerManager g_layerManager;
extern TransformSettings g_transform;
extern std::map<std::string, std::wstring> g_bitmapFiles;
extern HWND g_hMainWnd;

bool LoadProjectIni(const std::wstring& iniPath);
bool SaveProjectIni(const std::wstring& iniPath);



// ------------------------------
// INI 保存
// ------------------------------
bool SaveProjectFolder(const std::wstring& folderPath)
{
    // 1. フォルダ作成
    CreateDirectoryW(folderPath.c_str(), NULL);
    CreateDirectoryW((folderPath + L"\\bitmaps").c_str(), NULL);
    CreateDirectoryW((folderPath + L"\\layers").c_str(), NULL);

    // 2. config.ini 保存
    std::wstring iniPath = folderPath + L"\\config.ini";
    SaveProjectIni(iniPath);

    // 3. PNG コピー（bitmaps）
    for (auto& kv : g_bitmapFiles) {
        if (!kv.second.empty()) {
            std::wstring dst = folderPath + L"\\bitmaps\\" +
                std::wstring(kv.first.begin(), kv.first.end()) + L".png";
            CopyFileW(kv.second.c_str(), dst.c_str(), FALSE);
        }
    }

    // 4. PNG コピー（layers）
    for (int i = 0; i < (int)g_layerManager.layers.size(); i++) {
        auto& layer = g_layerManager.layers[i];
        if (!layer.filePath.empty()) {
            std::wstring dst = folderPath + L"\\layers\\layer" +
                std::to_wstring(i) + L".png";
            CopyFileW(layer.filePath.c_str(), dst.c_str(), FALSE);
        }
    }

    return true;
}


// ------------------------------
// INI 読み込み
// ------------------------------
bool LoadProjectFolder(const std::wstring& folderPath)
{
    // 1. config.ini 読み込み
    std::wstring iniPath = folderPath + L"\\config.ini";
    if (!LoadProjectIni(iniPath))
        return false;

    // 2. PNG 割り当て（bitmaps）
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW((folderPath + L"\\bitmaps\\*.png").c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::wstring file = folderPath + L"\\bitmaps\\" + fd.cFileName;

            std::wstring wname = fd.cFileName;      // L"HeadNormal.png"
            std::string key(wname.begin(), wname.end());
            key = key.substr(0, key.size() - 4);    // remove ".png"

            g_bitmapFiles[key] = file;

        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }

    // 3. PNG 割り当て（layers）
    HANDLE hFind2 = FindFirstFileW((folderPath + L"\\layers\\*.png").c_str(), &fd);
    if (hFind2 != INVALID_HANDLE_VALUE) {
        do {
            std::wstring file = folderPath + L"\\layers\\" + fd.cFileName;

            std::wstring wname = fd.cFileName;   // L"layer0.png"
            std::string name(wname.begin(), wname.end());

            int index = atoi(name.substr(5).c_str()); // "0.png" → 0

            if (index < (int)g_layerManager.layers.size()) {
                g_layerManager.layers[index].filePath = file;
            }

        } while (FindNextFileW(hFind2, &fd));
        FindClose(hFind2);
    }

    return true;
}

bool SaveProjectIni(const std::wstring& iniPath)
{
    WritePrivateProfileString(L"Transform", L"earMoveScale",
        std::to_wstring(g_transform.earMoveScale).c_str(), iniPath.c_str());

    WritePrivateProfileString(L"Transform", L"browUpY",
        std::to_wstring(g_transform.browUpY).c_str(), iniPath.c_str());

    WritePrivateProfileString(L"Transform", L"faceMoveScale",
        std::to_wstring(g_transform.faceMoveScale).c_str(), iniPath.c_str());

    // PNG スロット
    for (auto& kv : g_bitmapFiles) {
        std::wstring key(kv.first.begin(), kv.first.end());
        WritePrivateProfileString(L"Bitmaps", key.c_str(),
            kv.second.c_str(), iniPath.c_str());
    }

    // レイヤー
    for (int i = 0; i < g_layerManager.layers.size(); i++) {
        std::wstring section = L"Layer_" + std::to_wstring(i);

        WritePrivateProfileString(section.c_str(), L"name",
            g_layerManager.layers[i].name.c_str(), iniPath.c_str());

        WritePrivateProfileString(section.c_str(), L"file",
            g_layerManager.layers[i].filePath.c_str(), iniPath.c_str());

        WritePrivateProfileString(section.c_str(), L"enabled",
            g_layerManager.layers[i].enabled ? L"1" : L"0", iniPath.c_str());
    }

    return true;
}

bool LoadProjectIni(const std::wstring& iniPath)
{
    wchar_t buf[512];

    GetPrivateProfileString(L"Transform", L"earMoveScale", L"0", buf, 512, iniPath.c_str());
    g_transform.earMoveScale = static_cast<float>(_wtof(buf));

    GetPrivateProfileString(L"Transform", L"browUpY", L"0", buf, 512, iniPath.c_str());
    g_transform.browUpY = static_cast<float>(_wtof(buf));

    GetPrivateProfileString(L"Transform", L"faceMoveScale", L"0", buf, 512, iniPath.c_str());
    g_transform.faceMoveScale = static_cast<float>(_wtof(buf));

    // PNG スロット
    for (auto& kv : g_bitmapFiles) {
        std::wstring key(kv.first.begin(), kv.first.end());
        GetPrivateProfileString(L"Bitmaps", key.c_str(), L"", buf, 512, iniPath.c_str());
        if (wcslen(buf) > 0)
            kv.second = buf;
    }

    // レイヤー
    g_layerManager.layers.clear();
    for (int i = 0; ; i++) {
        std::wstring section = L"Layer_" + std::to_wstring(i);

        wchar_t name[256];
        GetPrivateProfileString(section.c_str(), L"name", L"", name, 256, iniPath.c_str());
        if (wcslen(name) == 0)
            break;

        wchar_t file[256];
        GetPrivateProfileString(section.c_str(), L"file", L"", file, 256, iniPath.c_str());

        wchar_t enabled[8];
        GetPrivateProfileString(section.c_str(), L"enabled", L"1", enabled, 8, iniPath.c_str());

        LayerSlot slot;
        slot.name = name;
        slot.filePath = file;
        slot.enabled = (wcscmp(enabled, L"1") == 0);

        g_layerManager.layers.push_back(slot);
    }

    return true;
}
