#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <map>
#include <string>
#include <cmath>
#include "types.h"
#include <mutex>

#include "render.h"
#include "LayerManager.h"



std::map<std::wstring, ID2D1Bitmap*> g_paramBitmaps;

extern std::map<std::wstring, ID2D1Bitmap*> g_paramBitmaps;

extern LayerManager g_layerManager;

// レイヤー名 → ビットマップ のマップ
extern std::map<std::wstring, ID2D1Bitmap*> g_layerBitmapsWindow;
extern std::map<std::wstring, ID2D1Bitmap*> g_layerBitmapsLayered;


extern IWICBitmapDecoder* decoder = nullptr;
IWICStream* stream = nullptr;

// グローバル変数
// ここには「extern」だけを書く
extern ID2D1DCRenderTarget* g_pDCRenderTarget;
extern HBITMAP g_hBitmap;
extern HDC g_hMemDC;
extern int g_canvasWidth;
extern int g_canvasHeight;

extern IWICImagingFactory* g_pWICFactory;


// main.cpp 側のグローバルを参照
extern ID2D1Factory* g_pD2DFactory;
extern ID2D1HwndRenderTarget* g_pRenderTarget;

// ★ 追加：RTごとにビットマップを分ける
extern std::map<std::string, ID2D1Bitmap*> g_bitmapsWindow;   // 編集モード用（g_pRenderTarget）
extern std::map<std::string, ID2D1Bitmap*> g_bitmapsLayered;  // 固定モード用（g_pDCRenderTarget）


extern int g_windowWidth;
extern int g_windowHeight;
extern int g_baseWidth;
extern int g_baseHeight;

extern std::map<std::string, std::wstring> g_bitmapFiles;
extern std::map<std::string, ID2D1Bitmap*> g_bitmaps;


extern FaceParams g_latestParams;// TCP受信スレッドが g_latestParamsに値を入れている。
extern std::mutex g_paramsMutex;

extern float g_earring_top_pos;
extern float g_earring_top_vel;
extern float g_earring_bottom_pos;
extern float g_earring_bottom_vel;

extern int g_eyeL_state;
extern int g_eyeR_state;

extern TransformSettings g_transform;
extern MouthState g_mouthState;


// ------------------------------
// Direct2D 初期化
// ------------------------------
extern void CreateD2DResources(HWND hwnd)
{
    if (!g_pD2DFactory)
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);

    if (!g_pRenderTarget)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        g_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hwnd,
                D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
            &g_pRenderTarget);
    }
}

// ========================
// DCRenderTarget を作る関数
// 


extern void CreateDCRenderTarget()
{
    if (!g_pD2DFactory) return;

    D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.0f, 96.0f);

    g_pD2DFactory->CreateDCRenderTarget(&props, &g_pDCRenderTarget);
}

//メモリDCとビットマップを作る関数
extern void CreateMemorySurface(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    g_canvasWidth = rc.right - rc.left;
    g_canvasHeight = rc.bottom - rc.top;
    // 画面 DC 取得
    HDC hdcScreen = GetDC(hwnd);
    // 古い DC 破棄
    if (g_hMemDC)
        DeleteDC(g_hMemDC);

    g_hMemDC = CreateCompatibleDC(hdcScreen);
    // 古いビットマップ破棄
    if (g_hBitmap)
        DeleteObject(g_hBitmap);
    // ★ 32bit ARGB の DIB セクション作る（透明対応）
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_canvasWidth;
    bmi.bmiHeader.biHeight = -g_canvasHeight; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;              // ★ ビットの深さ：32bit
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    g_hBitmap = CreateDIBSection(
        hdcScreen,
        &bmi,
        DIB_RGB_COLORS,
        &pvBits,
        NULL,
        0
    );

    // ★★★ ここが決定打 ★★★
    // DIB のメモリを完全透明 (0,0,0,0) で初期化する
    if (pvBits)
    {
        memset(pvBits, 0, g_canvasWidth * g_canvasHeight * 4);
    }



    // DIB をメモリ DC にセット
    SelectObject(g_hMemDC, g_hBitmap);
    // DC 解放
    ReleaseDC(hwnd, hdcScreen);
}

// ------------------------------
// PNG 読み込み
// ------------------------------
ID2D1Bitmap* LoadBitmapForTarget(ID2D1RenderTarget* pRT, const wchar_t* filename)
{
    if (!g_pWICFactory || !pRT) return nullptr;

    IWICBitmapDecoder* decoder = nullptr;
    HRESULT hr = g_pWICFactory->CreateDecoderFromFilename(
        filename, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) return nullptr;

    IWICBitmapFrameDecode* frame = nullptr;
    decoder->GetFrame(0, &frame);

    IWICFormatConverter* converter = nullptr;
    g_pWICFactory->CreateFormatConverter(&converter);

    converter->Initialize(
        frame, GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0,
        WICBitmapPaletteTypeCustom);

    ID2D1Bitmap* bitmap = nullptr;
    pRT->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap);

    // 最初に読み込んだ PNG のサイズが基準サイズとして使われる(条件「0 のとき」に変える）
    if (g_baseWidth == 0 && g_baseHeight == 0) {
        // 原寸サイズを保存
        UINT w = 0, h = 0;
        frame->GetSize(&w, &h);
        g_baseWidth = (int)w;
        g_baseHeight = (int)h;
    }

    converter->Release();
    frame->Release();
    decoder->Release();

    return bitmap;
}


// ------------------------------
// 編集モード用ロード
// ------------------------------
void LoadHeadBitmapsForWindow()
{
    if (!g_pRenderTarget) return;

    // 既存の破棄
    for (auto& kv : g_bitmapsWindow) {
        if (kv.second) kv.second->Release();
    }
    g_bitmapsWindow.clear();

    for (auto& kv : g_bitmapFiles)
    {
        const std::string& slot = kv.first;
        const std::wstring& file = kv.second;

        g_bitmapsWindow[slot] = LoadBitmapForTarget(g_pRenderTarget, file.c_str());
    }
    // ★ レイヤー PNG の再ロード
    for (auto& layer : g_layerManager.layers)
    {
        if (layer.filePath.empty()) continue;
        g_layerBitmapsWindow[layer.name] =
            LoadBitmapForTarget(g_pRenderTarget, layer.filePath.c_str());
    }
}


// ------------------------------
// 固定モード用ロード
// ------------------------------
void LoadHeadBitmapsForLayered()
{
    if (!g_pDCRenderTarget) return;

    for (auto& kv : g_bitmapsLayered) {
        if (kv.second) kv.second->Release();
    }
    g_bitmapsLayered.clear();

    for (auto& kv : g_bitmapFiles)
    {
        const std::string& slot = kv.first;
        const std::wstring& file = kv.second;

        g_bitmapsLayered[slot] = LoadBitmapForTarget(g_pDCRenderTarget, file.c_str());
    }
    // ★ レイヤー PNG の再ロード
    for (auto& layer : g_layerManager.layers)
    {
        if (layer.filePath.empty()) continue;
        g_layerBitmapsLayered[layer.name] =
            LoadBitmapForTarget(g_pDCRenderTarget, layer.filePath.c_str());
    }
}

// ------------------------------
// 全スロット描画（DCRenderTarget用）
// 固定モード側（DrawAllSlots）
// ------------------------------
extern void DrawAllSlots(ID2D1RenderTarget* pRT)
{
    if (!pRT) return;

    if (g_bitmapsLayered.empty()) {
        LoadHeadBitmapsForLayered();
    }


    // 背景は完全透明
    pRT->Clear(D2D1::ColorF(0, 0, 0, 0));


    // PNG（HeadNormal）があれば描画
    auto it = g_bitmapsLayered.find("HeadNormal");
    if (it != g_bitmapsLayered.end() && it->second) {
        D2D1_SIZE_F size = it->second->GetSize();
        D2D1_RECT_F dst = D2D1::RectF(0, 0, size.width, size.height);

        pRT->DrawBitmap(
            it->second,
            dst,
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    }

    // ★ レイヤー PNG の描画（追加）
    for (auto& layer : g_layerManager.layers)
    {
        if (!layer.enabled) continue;

        auto it2 = g_layerBitmapsLayered.find(layer.name);
        if (it2 == g_layerBitmapsLayered.end()) continue;

        ID2D1Bitmap* bmp = it2->second;
        if (!bmp) continue;

        D2D1_SIZE_F size = bmp->GetSize();
        D2D1_RECT_F dst = D2D1::RectF(0, 0, size.width, size.height);

        pRT->DrawBitmap(bmp, dst, 1.0f);
    }
}


// DCRenderTarget に DC をバインドして描画する

extern void DrawToLayeredWindow(HWND hwnd)
{
    if (!g_pDCRenderTarget)
        CreateDCRenderTarget();

    if (!g_hMemDC)
        CreateMemorySurface(hwnd);

    RECT rc;
    GetClientRect(hwnd, &rc);
    g_pDCRenderTarget->BindDC(g_hMemDC, &rc);

    g_pDCRenderTarget->BeginDraw();
    DrawAllSlots(g_pDCRenderTarget);
    g_pDCRenderTarget->EndDraw();

    RECT wr;
    GetWindowRect(hwnd, &wr);

    POINT ptSrc = { 0, 0 };                 // DC 内の原点
    POINT ptDst = { wr.left, wr.top };      // 画面上の表示位置
    SIZE sizeWnd = { g_canvasWidth, g_canvasHeight };

    BLENDFUNCTION bf = {};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    UpdateLayeredWindow(
        hwnd,
        nullptr,
        &ptDst,
        &sizeWnd,
        g_hMemDC,
        &ptSrc,
        0,
        &bf,
        ULW_ALPHA);
}









// ------------------------------
// 描画ヘルパー
// ------------------------------
static void DrawBitmapWithTransform(
    ID2D1Bitmap* pBitmap,
    const D2D1_RECT_F& destRect,
    float opacity = 1.0f,
    const D2D1_MATRIX_3X2_F* pTransform = nullptr)
{
    if (!g_pRenderTarget || !pBitmap) return;

    if (pTransform)
        g_pRenderTarget->SetTransform(*pTransform);
    else
        g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    g_pRenderTarget->DrawBitmap(
        pBitmap,
        destRect,
        opacity,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

    g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

// ------------------------------
// バネ物理
// ------------------------------
float physics_spring(float target, float& pos, float& vel, float stiffness, float damping)
{
    float accel = (target - pos) * stiffness;
    vel += accel;
    vel *= damping;
    pos += vel;
    return pos;
}

// ------------------------------
// 表情更新（口の開閉）
// ------------------------------
void UpdateStatesFromParams(const FaceParams& p)
{
    static bool mouthOpenFlag = false;

    if (!mouthOpenFlag)
    {
        if (p.jawOpen >= 0.24f)
            mouthOpenFlag = true;
    }
    else
    {
        if (p.jawOpen <= 0.20f)
            mouthOpenFlag = false;
    }

    g_mouthState = mouthOpenFlag ? MouthState::Open : MouthState::Closed;
}

// ------------------------------
// 描画本体
// ------------------------------
void DrawScene(HWND hwnd)
{
    //ログ出力
    OutputDebugString(L"[DrawScene] called\n");

    wchar_t dbg[256];
    swprintf_s(dbg, L"[DrawScene] bitmaps size = %d\n", (int)g_bitmaps.size());
    OutputDebugString(dbg);
    //

    if (!g_pD2DFactory) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    }


    if (!g_pRenderTarget) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        g_windowWidth = rc.right - rc.left;
        g_windowHeight = rc.bottom - rc.top;

        g_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hwnd,
                D2D1::SizeU(g_windowWidth, g_windowHeight)),
            &g_pRenderTarget);
    }
    if (!g_pRenderTarget) return;


    // 編集モード側（DrawScene）
    // 初回ロード(ビットマップがまだならロード)
    if (g_bitmapsWindow.empty() || g_layerBitmapsWindow.empty()) {
        LoadHeadBitmapsForWindow();
    }

    // 編集モード用：白背景（不透明）
    g_pRenderTarget->BeginDraw();
    g_pRenderTarget->Clear(D2D1::ColorF(1, 1, 1, 1));


    extern LayerManager g_layerManager;
    extern std::map<std::wstring, ID2D1Bitmap*> g_layerBitmapsWindow;

    for (auto& layer : g_layerManager.layers)
    {
        if (!layer.enabled)
            continue;

        auto it = g_layerBitmapsWindow.find(layer.name);
        if (it == g_layerBitmapsWindow.end())
            continue;

        ID2D1Bitmap* bmp = it->second;
        if (!bmp)
            continue;

        D2D1_SIZE_F size = bmp->GetSize();
        D2D1_RECT_F dst = D2D1::RectF(0, 0, size.width, size.height);

        g_pRenderTarget->DrawBitmap(
            bmp,
            dst,
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
    }


    HRESULT hr = g_pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        g_pRenderTarget->Release();
        g_pRenderTarget = nullptr;

        for (auto& kv : g_bitmapsWindow) {
            if (kv.second) kv.second->Release();
        }
        g_bitmapsWindow.clear();

        g_baseWidth = 0;
        g_baseHeight = 0;
    }
}
