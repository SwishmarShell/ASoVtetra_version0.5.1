#pragma comment(lib, "comctl32.lib")   // InitCommonControlsEx のため
#pragma comment(lib, "d2d1.lib")       // Direct2D
#pragma comment(lib, "windowscodecs.lib") // WIC
#pragma comment(lib, "ws2_32.lib")     // Winsock（tcp.cpp にも入れているなら重複可）

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "ui_settings.h"

#include <d2d1.h>
#include <wincodec.h>
#include <mutex>
#include <map>
#include <string>

#include "preset.h"
#include "settings_io.h"
#include "render.h"
#include "types.h"
#include "LayerManager.h"


// ------------------------------
// グローバル変数（本体）
// ------------------------------
// 起動直後は、編集モード（true), 本番モード（fales)
bool g_editMode = true;

int g_windowWidth = 0;
int g_windowHeight = 0;

int g_baseWidth = 0;
int g_baseHeight = 0;

ID2D1Factory* g_pD2DFactory = nullptr;
ID2D1HwndRenderTarget* g_pRenderTarget = nullptr;


ID2D1DCRenderTarget* g_pDCRenderTarget = nullptr;
HBITMAP g_hBitmap = nullptr;
HDC g_hMemDC = nullptr;
int g_canvasWidth = 0;
int g_canvasHeight = 0;

IWICImagingFactory* g_pWICFactory = nullptr;


float g_earring_top_pos = 0.0f;
float g_earring_top_vel = 0.0f;
float g_earring_bottom_pos = 0.0f;
float g_earring_bottom_vel = 0.0f;

int g_eyeL_state = 0;
int g_eyeR_state = 0;

TransformSettings g_transform;
MouthState g_mouthState = MouthState::Closed;

std::map<std::string, std::wstring> g_bitmapFiles;
std::map<std::string, ID2D1Bitmap*> g_bitmaps;

std::map<std::string, ID2D1Bitmap*> g_bitmapsWindow;   // 編集モード用（g_pRenderTarget）
std::map<std::string, ID2D1Bitmap*> g_bitmapsLayered;  // 固定モード用（g_pDCRenderTarget）

std::map<std::wstring, ID2D1Bitmap*> g_layerBitmapsWindow;
std::map<std::wstring, ID2D1Bitmap*> g_layerBitmapsLayered;

HWND g_hMainWnd = NULL;
HWND hLayerPanel = NULL;

LayerManager g_layerManager;

// 追加: 設定ダイアログの HWND をグローバル変数として宣言
HWND g_hSettingsDlg = nullptr;

bool g_isLayeredMode = false;

// ------------------------------
// InitBitmapSlots
// ------------------------------
void InitBitmapSlots()
{
    g_bitmapFiles["HeadNormal"] = L"HeadNormal.png";
    g_bitmapFiles["HeadMove"] = L"HeadMove.png";
    g_bitmapFiles["MouthOpen"] = L"MouthOpen.png";
    g_bitmapFiles["MouthClose"] = L"MouthClose.png";
    g_bitmapFiles["Teeth"] = L"Teeth.png";

    g_bitmapFiles["EyeL_Open"] = L"EyeL_Open.png";
    g_bitmapFiles["EyeL_Half"] = L"EyeL_Half.png";
    g_bitmapFiles["EyeL_Close"] = L"EyeL_Close.png";

    g_bitmapFiles["EyeR_Open"] = L"EyeR_Open.png";
    g_bitmapFiles["EyeR_Half"] = L"EyeR_Half.png";
    g_bitmapFiles["EyeR_Close"] = L"EyeR_Close.png";

    g_bitmapFiles["R_Ear"] = L"R_Ear.png";
    g_bitmapFiles["R_Earring_Top"] = L"R_Earring_Top.png";
    g_bitmapFiles["R_Earring_Bottom"] = L"R_Earring_Bottom.png";

    g_bitmapFiles["BrowL"] = L"BrowL.png";
    g_bitmapFiles["BrowR"] = L"BrowR.png";

    g_bitmapFiles["FCline"] = L"FCline.png";


}

// ------------------------------
// ToggleWindowMode
// ------------------------------
void ToggleWindowMode(HWND hwnd)
{
    g_editMode = !g_editMode;

    if (g_editMode)
        OutputDebugString(L"[Mode] EDIT MODE\n");
    else
        OutputDebugString(L"[Mode] LAYERED MODE\n");


    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    // いまのクライアントサイズを取得
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    int clientW = rcClient.right - rcClient.left;
    int clientH = rcClient.bottom - rcClient.top;

    if (g_editMode)
    {
        // 編集モード：枠あり・非レイヤード
        style = WS_OVERLAPPEDWINDOW;
        exStyle &= ~(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT);
        exStyle |= WS_EX_APPWINDOW;

        // ★ 固定モード用 RT とビットマップを捨てる
        if (g_pDCRenderTarget) {
            g_pDCRenderTarget->Release();
            g_pDCRenderTarget = nullptr;
        }
        for (auto& kv : g_bitmapsLayered) {
            if (kv.second) kv.second->Release();
        }
        g_bitmapsLayered.clear();
    }


    else
    {
        // 固定モード：レイヤード＋最前面（マウス透過は WM_NCHITTEST でやる）
        style = WS_POPUP;
        exStyle &= ~WS_EX_APPWINDOW;
        exStyle |= WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT;

        CreateMemorySurface(hwnd);
        CreateDCRenderTarget();
        // ★ 編集モード用 RT とビットマップを捨てる
        if (g_pRenderTarget) {
            g_pRenderTarget->Release();
            g_pRenderTarget = nullptr;
        }
        for (auto& kv : g_bitmapsWindow) {
            if (kv.second) kv.second->Release();
        }
        g_bitmapsWindow.clear();
    }

    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    // このスタイルで「同じクライアントサイズ」を持つ外側矩形を計算
    RECT rcNew = { 0, 0, clientW, clientH };
    AdjustWindowRectEx(&rcNew, style, FALSE, exStyle);
    int newW = rcNew.right - rcNew.left;
    int newH = rcNew.bottom - rcNew.top;

    // 位置はそのまま、サイズだけ新しい外側サイズに
    RECT rcWin;
    GetWindowRect(hwnd, &rcWin);


    // 固定モードに入るときは TOPMOST を明示
    UINT flags = SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_SHOWWINDOW;

    if (!g_editMode) {
        // 固定モード → 最前面固定
        SetWindowPos(hwnd, HWND_TOPMOST,
            rcWin.left, rcWin.top,
            newW, newH,
            flags);
        // ★ レイヤード用 PNG を再ロード
        LoadHeadBitmapsForLayered();
    }
    else {
        // 編集モード → NOTOPMOST に戻す
        SetWindowPos(hwnd, HWND_NOTOPMOST,
            rcWin.left, rcWin.top,
            newW, newH,
            flags);
        // ★ 編集モード用 PNG を再ロード
        LoadHeadBitmapsForWindow();
    }

    // サイズはリセットしておくと安全
    g_windowWidth = 0;
    g_windowHeight = 0;

    InvalidateRect(hwnd, NULL, FALSE);
}





// ------------------------------
// WndProc
// ------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MOUSEACTIVATE:
        if (!g_editMode) {
            return MA_NOACTIVATEANDEAT; // クリックを食べて下に渡す
        }
        break;


    case WM_SIZE:
    {
        g_windowWidth = LOWORD(lParam);
        g_windowHeight = HIWORD(lParam);

        if (g_pRenderTarget)
        {
            g_pRenderTarget->Resize(
                D2D1::SizeU(g_windowWidth, g_windowHeight));
        }
        return 0;
    }


    case WM_CREATE:
        CreateD2DResources(hwnd);




        RECT rc;
        GetClientRect(hwnd, &rc);
        if (g_pRenderTarget) {
            g_pRenderTarget->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
        }

        // 固定モードにしたら、DrawToLayeredWindowを呼ぶようにするためのTIMER記述
        SetTimer(hwnd, 1, 33, nullptr);

        return 0;

    case WM_TIMER:
    {
        if (!g_editMode)
        {
            // 固定モード：レイヤードだけ更新
            DrawToLayeredWindow(hwnd);
        }
        else
        {
            // 編集モード：普通の再描画
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    }

    case WM_PAINT:
    {
        if (g_editMode)
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            DrawScene(hwnd); // 黒背景＋PNG（編集モード）
            EndPaint(hwnd, &ps);
        }
        else
        {
            // 固定モードでは WM_PAINT では描画しない
            ValidateRect(hwnd, nullptr);
        }
        return 0;
    }


    case WM_KEYDOWN:
        if (wParam == VK_F1)
            ToggleWindowMode(hwnd);

        if (wParam == VK_F2)
        {
            if (!g_hSettingsDlg)
            {
                g_hSettingsDlg = CreateDialog(
                    GetModuleHandle(NULL),
                    MAKEINTRESOURCE(IDD_SETTINGS),
                    hwnd,
                    SettingsProc
                );
                ShowWindow(g_hSettingsDlg, SW_SHOW);
            }
            else
            {
                BOOL visible = IsWindowVisible(g_hSettingsDlg);
                ShowWindow(g_hSettingsDlg, visible ? SW_HIDE : SW_SHOW);
            }
        }
        return 0;

    case WM_DESTROY:
        // Direct2D リソース解放
        if (g_pRenderTarget) { g_pRenderTarget->Release(); g_pRenderTarget = nullptr; }
        if (g_pD2DFactory) { g_pD2DFactory->Release(); g_pD2DFactory = nullptr; }
        if (g_pWICFactory) { g_pWICFactory->Release(); g_pWICFactory = nullptr; }

        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ------------------------------
// WinMain
// ------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{

    extern LayerManager g_layerManager;
    g_layerManager.LoadConfig();

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    // 失敗してたらログ出す
    if (FAILED(hr)) {
        OutputDebugString(L"[COM] CoInitializeEx FAILED\n");
    }

    // ここで WIC ファクトリ作成
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&g_pWICFactory)
    );
    if (FAILED(hr)) {
        wchar_t buf[256];
        swprintf_s(buf, L"[COM] CoCreateInstance WIC FAILED hr=0x%08X\n", hr);
        OutputDebugString(buf);
    }




    CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&g_pWICFactory)
    );

    SetProcessDPIAware(); // Settings ダイアログの DPI 対応を ON

    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX) };
    icc.dwICC = ICC_BAR_CLASSES;  // Trackbar, ProgressBar など
    InitCommonControlsEx(&icc);

    // CoInitialize(nullptr);

    InitBitmapSlots();
    LoadSettings();


    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ASoVtetraWindowClass";
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        0, // WS_EX_LAYERED | WS_EX_TOPMOST,// F1キーで透明。WS_EX_TRANSPARENT(起動直後に透明モードの場合)間に入れる
        L"ASoVtetraWindowClass",
        L"ASoVtetra",
        WS_OVERLAPPEDWINDOW, // ここも一時的に WS_POPUP → WS_OVERLAPPEDWINDOW にしてOK
        0, 0, 900, 900,
        nullptr, nullptr, hInstance, nullptr);
    g_hMainWnd = hwnd;

    ShowWindow(hwnd, nCmdShow);


    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return 0;
}
