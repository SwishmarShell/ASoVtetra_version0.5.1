#include "ui_api.h"
#include "render.h"
#include <windows.h>
#include <commdlg.h>
#include <map>

extern bool g_editMode;
extern std::map<std::string, std::wstring> g_bitmapFiles;

// 新しいビットマップマップ
extern std::map<std::string, ID2D1Bitmap*> g_bitmapsWindow;
extern std::map<std::string, ID2D1Bitmap*> g_bitmapsLayered;

// RT
extern ID2D1HwndRenderTarget* g_pRenderTarget;
extern ID2D1DCRenderTarget* g_pDCRenderTarget;

void SelectPngForSlot(HWND owner, const std::string& slot)
{
    LONG ex = GetWindowLong(owner, GWL_EXSTYLE);
    SetWindowLong(owner, GWL_EXSTYLE, ex & ~WS_EX_LAYERED & ~WS_EX_TRANSPARENT);

    SetWindowPos(owner, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    wchar_t fileName[MAX_PATH] = L"";
    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"PNG Files\0*.png\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        g_bitmapFiles[slot] = fileName;

        // ★ ここが重要：RTごとに読み込む
        if (g_editMode)
        {
            // 編集モード → HwndRenderTarget 用に読み込む
            if (g_pRenderTarget)
            {
                if (g_bitmapsWindow[slot])
                    g_bitmapsWindow[slot]->Release();

                g_bitmapsWindow[slot] =
                    LoadBitmapForTarget(g_pRenderTarget, fileName);
            }
        }
        else
        {
            // 固定モード → DCRenderTarget 用に読み込む
            if (g_pDCRenderTarget)
            {
                if (g_bitmapsLayered[slot])
                    g_bitmapsLayered[slot]->Release();

                g_bitmapsLayered[slot] =
                    LoadBitmapForTarget(g_pDCRenderTarget, fileName);
            }
        }
    }

    SetWindowLong(owner, GWL_EXSTYLE, ex);
    SetWindowPos(owner, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}
