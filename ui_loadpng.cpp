#include <windows.h>
#include <commdlg.h>
#include <string>
#include "LayerManager.h"
#include <map>

extern std::map<std::string, std::wstring> g_bitmapFiles;
extern LayerManager g_layerManager;
extern HWND g_hMainWnd;

// Direct2D 再ロード関数
void LoadHeadBitmapsForWindow();
void LoadHeadBitmapsForLayered();


// --------------------------------------
// スロット PNG を読み込む（HeadNormal など）
// --------------------------------------
void LoadSlotPng(const std::string& slotName)
{
    wchar_t fileName[MAX_PATH] = L"";
    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.lpstrFilter = L"PNG Files (*.png)\0*.png\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (!GetOpenFileName(&ofn))
        return;

    // 絶対パス化
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(fileName, MAX_PATH, fullPath, nullptr);

    // 保存
    g_bitmapFiles[slotName] = fullPath;

    // 再描画
    LoadHeadBitmapsForWindow();
    LoadHeadBitmapsForLayered();
    InvalidateRect(g_hMainWnd, NULL, FALSE);
}


// --------------------------------------
// レイヤー PNG を読み込む（layer0, layer1…）
// --------------------------------------
void LoadLayerPng(int index)
{
    if (index < 0 || index >= (int)g_layerManager.layers.size())
        return;

    wchar_t fileName[MAX_PATH] = L"";
    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.lpstrFilter = L"PNG Files (*.png)\0*.png\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    if (!GetOpenFileName(&ofn))
        return;

    // 絶対パス化
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(fileName, MAX_PATH, fullPath, nullptr);

    // 保存
    g_layerManager.layers[index].filePath = fullPath;

    // 再描画
    LoadHeadBitmapsForWindow();
    LoadHeadBitmapsForLayered();
    InvalidateRect(g_hMainWnd, NULL, FALSE);
}
