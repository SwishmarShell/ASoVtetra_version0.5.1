#pragma once

#include <windows.h>   // HWND
#include <d2d1.h>
#include "types.h"     // FaceParams, TransformSettings, MouthState
#include <string>

#include <wincodec.h> // IWICImagingFactory の宣言を追加

extern ID2D1DCRenderTarget* g_pDCRenderTarget;
extern HBITMAP g_hBitmap;
extern HDC g_hMemDC;
extern int g_canvasWidth;
extern int g_canvasHeight;

extern IWICImagingFactory* g_pWICFactory;


// Direct2D 初期化
void CreateD2DResources(HWND hwnd);

void CreateDCRenderTarget();

void CreateMemorySurface(HWND hwnd);

void DrawAllSlots(ID2D1RenderTarget* pRenderTarget);

void DrawToLayeredWindow(HWND hwnd);

// PNG 読み込み
// 全パーツ読み込み
ID2D1Bitmap* LoadBitmapForTarget(ID2D1RenderTarget* pRT, const wchar_t* filename);
void LoadHeadBitmapsForWindow();
void LoadHeadBitmapsForLayered();



// 描画
void DrawScene(HWND hwnd);

// 物理シミュレーション
float physics_spring(float target, float& pos, float& vel, float stiffness, float damping);

// 表情更新
void UpdateStatesFromParams(const FaceParams& p);


ID2D1Bitmap* LoadBitmapForTarget(ID2D1RenderTarget* pRT, const wchar_t* filename);
