#include "LayerManager.h"
#include <algorithm>
#include <windows.h>

void LayerManager::AddLayer(const std::wstring& name)
{
    LayerSlot slot;
    slot.name = name;
    slot.filePath = L"";
    slot.enabled = true;
    layers.push_back(slot);
}

void LayerManager::RemoveLayer(int index)
{
    if (index < 0 || index >= (int)layers.size()) return;
    layers.erase(layers.begin() + index);
}

void LayerManager::MoveLayerUp(int index)
{
    if (index <= 0 || index >= (int)layers.size()) return;
    std::swap(layers[index], layers[index - 1]);
}

void LayerManager::MoveLayerDown(int index)
{
    if (index < 0 || index >= (int)layers.size() - 1) return;
    std::swap(layers[index], layers[index + 1]);
}

void LayerManager::LoadConfig()
{
    const wchar_t* ini = L"layers.ini";

    layers.clear();

    for (int i = 0; ; i++)
    {
        std::wstring section = L"Layer_" + std::to_wstring(i);

        wchar_t name[256];
        GetPrivateProfileString(section.c_str(), L"name", L"", name, 256, ini);
        if (wcslen(name) == 0)
            break;

        wchar_t file[256];
        GetPrivateProfileString(section.c_str(), L"file", L"", file, 256, ini);

        wchar_t enabled[8];
        GetPrivateProfileString(section.c_str(), L"enabled", L"1", enabled, 8, ini);

        LayerSlot slot;
        slot.name = name;
        slot.filePath = file;
        slot.enabled = (wcscmp(enabled, L"1") == 0);

        layers.push_back(slot);
    }
}


void LayerManager::SaveConfig()
{
    const wchar_t* ini = L"layers.ini";

    // 既存セクションをクリア
    WritePrivateProfileString(NULL, NULL, NULL, ini);

    for (int i = 0; i < layers.size(); i++)
    {
        std::wstring section = L"Layer_" + std::to_wstring(i);

        WritePrivateProfileString(section.c_str(), L"name",
            layers[i].name.c_str(), ini);

        WritePrivateProfileString(section.c_str(), L"file",
            layers[i].filePath.c_str(), ini);

        WritePrivateProfileString(section.c_str(), L"enabled",
            layers[i].enabled ? L"1" : L"0", ini);
    }
}

