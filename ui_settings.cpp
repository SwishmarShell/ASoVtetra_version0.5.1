#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include <map>
#include <string>

#include "types.h"
#include "ui_settings.h"
#include "preset.h"
#include "settings_io.h"
#include "ui_api.h"
#include <ShlObj.h>

#include "LayerManager.h"
#include "project_io.h"

#include "render.h"   // ★ 最後にする



// main.cpp のビットマップキャッシュ
extern std::map<std::string, ID2D1Bitmap*> g_bitmapsWindow;
extern std::map<std::string, ID2D1Bitmap*> g_bitmapsLayered;

extern ID2D1Bitmap* LoadBitmapForTarget(ID2D1RenderTarget* pRT, const wchar_t* filename);
extern ID2D1HwndRenderTarget* g_pRenderTarget;
extern std::map<std::wstring, ID2D1Bitmap*> g_layerBitmapsWindow;
extern std::map<std::wstring, ID2D1Bitmap*> g_layerBitmapsLayered;

extern HWND hLayerPanel;

extern LayerManager g_layerManager;

extern HWND g_hMainWnd;

extern bool g_isLayeredMode;

extern HWND hLoad_SavingText;

void RebuildLayerPanelUI(HWND hDlg);

// スクロール量 & LayerPanel の WndProc（子ウィンドウプロシージャ）
static int scrollOffset = 0;

// PNG 選択（main.cpp 側で定義されている）
extern void SelectPngForSlot(HWND owner, const std::string& slot);

// TransformSettings（main.cpp 側のグローバル）
extern TransformSettings g_transform;

// スロット辞書（main.cpp 側）
extern std::map<std::string, std::wstring> g_bitmapFiles;

// LayerPanel のウィンドウクラス
LRESULT CALLBACK LayerPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hLoad_SavingText = NULL;


// LayerRow ( [Edit: Name]  [Load PNG]  [ON/OFF]  [↑] [↓] [X] )を管理する構造体
struct LayerRowUI {
    HWND hNameEdit;
    HWND hLoadBtn;
    HWND hEnableCheck;
    HWND hUpBtn;
    HWND hDownBtn;
    HWND hDeleteBtn;
};

// LayerRow UI の配列（全行）
static std::vector<LayerRowUI> g_layerRows;

bool SaveProjectFolder(const std::wstring& folderPath);
bool LoadProjectFolder(const std::wstring& folderPath);

// ------------------------------
// UI ハンドラ
// ------------------------------
void OnPresetSave(HWND hDlg)
{
    OutputDebugString(L"[UI] OnPresetSave\n");
}

void OnPresetLoad(HWND hDlg)
{
    OutputDebugString(L"[UI] OnPresetLoad\n");
}

void OnPresetDelete(HWND hDlg)
{
    OutputDebugString(L"[UI] OnPresetDelete\n");
}

// ------------------------------
// InputName ダイアログ
// ------------------------------
INT_PTR CALLBACK InputNameProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDOK:
            EndDialog(hDlg, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

// ------------------------------
// Settings ダイアログ
// ------------------------------
INT_PTR CALLBACK SettingsProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {

    case WM_INITDIALOG:
    {
        // LayerPanel ウィンドウクラス登録
        WNDCLASS wc = {};
        wc.lpfnWndProc = LayerPanelProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"LayerPanelClass";
        RegisterClass(&wc);

        // プリセット一覧を読み込む
        HWND hList = GetDlgItem(hDlg, IDC_LIST_PRESETS);

        wchar_t buffer[256];
        const wchar_t* ini = L"presets.ini";


        // スライダー初期化
        SendDlgItemMessage(hDlg, IDC_SLIDER_EAR_MOVE, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        SendDlgItemMessage(hDlg, IDC_SLIDER_EAR_MOVE, TBM_SETPOS, TRUE, (int)g_transform.earMoveScale);

        SendDlgItemMessage(hDlg, IDC_SLIDER_BROW_UP, TBM_SETRANGE, TRUE, MAKELONG(-50, 50));
        SendDlgItemMessage(hDlg, IDC_SLIDER_BROW_UP, TBM_SETPOS, TRUE, (int)g_transform.browUpY);

        SendDlgItemMessage(hDlg, IDC_SLIDER_FACE_MOVE, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        SendDlgItemMessage(hDlg, IDC_SLIDER_FACE_MOVE, TBM_SETPOS, TRUE, (int)g_transform.faceMoveScale);

        // EDITBOX 初期化
        SetDlgItemInt(hDlg, IDC_EDIT_EAR_MOVE, (int)g_transform.earMoveScale, TRUE);
        SetDlgItemInt(hDlg, IDC_EDIT_BROW_UP, (int)g_transform.browUpY, TRUE);
        SetDlgItemInt(hDlg, IDC_EDIT_FACE_MOVE, (int)g_transform.faceMoveScale, TRUE);

        // LayerPanel を生成
        HWND hPlaceholder = GetDlgItem(hDlg, IDC_LAYER_PANEL);
        HWND hLoad_SavingText = GetDlgItem(hDlg, IDC_LOAD_SAVING_TEXT);

        RECT rc;
        GetWindowRect(hPlaceholder, &rc);
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);


        // 子ウィンドウ（LayerPanel）を作成
        hLayerPanel = CreateWindowEx(
            0,
            L"LayerPanelClass",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER,
            rc.left, rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            hDlg,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );

        return TRUE;
    }

    case WM_HSCROLL:
    {
        HWND slider = (HWND)lParam;
        int pos = SendMessage(slider, TBM_GETPOS, 0, 0);

        if (slider == GetDlgItem(hDlg, IDC_SLIDER_EAR_MOVE)) {
            g_transform.earMoveScale = static_cast<float>(pos);
            SetDlgItemInt(hDlg, IDC_EDIT_EAR_MOVE, pos, TRUE);
        }
        else if (slider == GetDlgItem(hDlg, IDC_SLIDER_BROW_UP)) {
            g_transform.browUpY = static_cast<float>(pos);
            SetDlgItemInt(hDlg, IDC_EDIT_BROW_UP, pos, TRUE);
        }
        else if (slider == GetDlgItem(hDlg, IDC_SLIDER_FACE_MOVE)) {
            g_transform.faceMoveScale = static_cast<float>(pos);
            SetDlgItemInt(hDlg, IDC_EDIT_FACE_MOVE, pos, TRUE);
        }

        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDC_BTN_ADD_LAYER:
        {
            extern LayerManager g_layerManager;
            extern HWND hLayerPanel;

            // 1. データ追加
            g_layerManager.AddLayer(L"NewLayer");

            // 2. UI 全破棄
            for (auto& row : g_layerRows) {
                DestroyWindow(row.hNameEdit);
                DestroyWindow(row.hLoadBtn);
                DestroyWindow(row.hEnableCheck);
                DestroyWindow(row.hUpBtn);
                DestroyWindow(row.hDownBtn);
                DestroyWindow(row.hDeleteBtn);
            }
            g_layerRows.clear();

            // 3. UI 再生成
                // LayerPanel UI
            for (size_t i = 0; i < g_layerManager.layers.size(); i++) {
                CreateLayerRow(hLayerPanel, i);
            }
            UpdateLayerRowPositions(hLayerPanel);


            // ★ 追加：スクロール位置をリセット
            scrollOffset = 0;
            SetScrollPos(hLayerPanel, SB_VERT, 0, TRUE);

            // ★ パネルを再描画
            InvalidateRect(hLayerPanel, NULL, TRUE);


            // 4. メイン描画更新
            extern HWND g_hMainWnd;
            InvalidateRect(g_hMainWnd, NULL, FALSE);

            return TRUE;
        }


        case IDC_BTN_SAVE_ZIP:
        {
            BROWSEINFO bi = { 0 };
            bi.lpszTitle = L"保存先フォルダを選択してください";
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

            if (pidl) {
                wchar_t folder[MAX_PATH];
                SHGetPathFromIDList(pidl, folder);

                if (SaveProjectFolder(folder)) {
                    MessageBox(hDlg, L"フォルダ保存が完了しました。", L"Save Folder", MB_OK);
                }
                else {
                    MessageBox(hDlg, L"フォルダ保存に失敗しました。", L"Save Folder", MB_OK | MB_ICONERROR);
                }
            }
            return TRUE;
        }


        case IDC_BTN_LOAD_ZIP:
        {
            BROWSEINFO bi = { 0 };
            bi.lpszTitle = L"読み込むフォルダを選択してください";
            LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

            if (pidl) {
                wchar_t folder[MAX_PATH];
                SHGetPathFromIDList(pidl, folder);

                SetWindowText(hLoad_SavingText, L"読み込み中...");
                ShowWindow(hLoad_SavingText, SW_SHOW);

                if (LoadProjectFolder(folder)) {

                    // ★ キャッシュ完全破棄（既存コードそのまま利用）
                    for (auto& kv : g_bitmapsWindow)  if (kv.second) kv.second->Release();
                    for (auto& kv : g_bitmapsLayered) if (kv.second) kv.second->Release();
                    for (auto& kv : g_layerBitmapsWindow)  if (kv.second) kv.second->Release();
                    for (auto& kv : g_layerBitmapsLayered) if (kv.second) kv.second->Release();

                    g_bitmapsWindow.clear();
                    g_bitmapsLayered.clear();
                    g_layerBitmapsWindow.clear();
                    g_layerBitmapsLayered.clear();

                    // ★ 現在の描画モードに応じてロード
                    if (g_isLayeredMode) {
                        LoadHeadBitmapsForLayered();
                    }
                    else {
                        LoadHeadBitmapsForWindow();
                    }

                    // ★ LayerPanel UI 再構築
                    RebuildLayerPanelUI(hDlg);

                    // ★ メイン描画更新
                    InvalidateRect(g_hMainWnd, NULL, FALSE);

                    ShowWindow(hLoad_SavingText, SW_HIDE);
                    SetWindowText(hLoad_SavingText, L"");

                    MessageBox(hDlg, L"フォルダ読み込みが完了しました。", L"Load Folder", MB_OK);
                }
                else {
                    MessageBox(hDlg, L"フォルダ読み込みに失敗しました。", L"Load Folder", MB_OK | MB_ICONERROR);
                }
            }
            return TRUE;
        }

        // ------------------------------
        // 設定保存
        // ------------------------------

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}


// LayerRow 生成する関数
void CreateLayerRow(HWND hParent, int index)
{
    const int rowHeight = 30;
    const int y = index * rowHeight;

    LayerRowUI row{};

    row.hNameEdit = CreateWindowEx(
        0, L"EDIT",
        g_layerManager.layers[index].name.c_str(),
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        5, y + 5, 120, 20,
        hParent, (HMENU)(INT_PTR)(10000 + index),
        GetModuleHandle(NULL), NULL);

    row.hLoadBtn = CreateWindowEx(
        0, L"BUTTON", L"Load PNG",
        WS_CHILD | WS_VISIBLE,
        130, y + 5, 80, 20,
        hParent, (HMENU)(INT_PTR)(11000 + index),
        GetModuleHandle(NULL), NULL);

    row.hEnableCheck = CreateWindowEx(
        0, L"BUTTON", L"ON",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        215, y + 5, 40, 20,
        hParent, (HMENU)(INT_PTR)(12000 + index),
        GetModuleHandle(NULL), NULL);
    // ★ チェック状態を復元（ここに入れる）
    SendMessage(row.hEnableCheck, BM_SETCHECK,
        g_layerManager.layers[index].enabled ? BST_CHECKED : BST_UNCHECKED,
        0);

    row.hUpBtn = CreateWindowEx(
        0, L"BUTTON", L"↑",
        WS_CHILD | WS_VISIBLE,
        260, y + 5, 25, 20,
        hParent, (HMENU)(INT_PTR)(13000 + index),
        GetModuleHandle(NULL), NULL);

    row.hDownBtn = CreateWindowEx(
        0, L"BUTTON", L"↓",
        WS_CHILD | WS_VISIBLE,
        290, y + 5, 25, 20,
        hParent, (HMENU)(INT_PTR)(14000 + index),
        GetModuleHandle(NULL), NULL);

    row.hDeleteBtn = CreateWindowEx(
        0, L"BUTTON", L"X",
        WS_CHILD | WS_VISIBLE,
        320, y + 5, 25, 20,
        hParent, (HMENU)(INT_PTR)(15000 + index),
        GetModuleHandle(NULL), NULL);

    g_layerRows.push_back(row);
}


LRESULT CALLBACK LayerPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch (msg)
    {
    case WM_CREATE:
        return 0;


    case WM_VSCROLL:
    {
        int code = LOWORD(wParam);
        int pos = HIWORD(wParam);

        switch (code)
        {
        case SB_LINEUP:       scrollOffset -= 20; break;
        case SB_LINEDOWN:     scrollOffset += 20; break;
        case SB_PAGEUP:       scrollOffset -= 100; break;
        case SB_PAGEDOWN:     scrollOffset += 100; break;
        case SB_THUMBTRACK:   scrollOffset = pos; break;
        }

        if (scrollOffset < 0) scrollOffset = 0;

        // ★ maxScroll の自動計算
        int rowHeight = 30;
        RECT rc;
        GetClientRect(hwnd, &rc);
        int panelHeight = rc.bottom - rc.top;
        int totalHeight = rowHeight * g_layerRows.size();
        int maxScroll = max(0, totalHeight - panelHeight);

        if (scrollOffset > maxScroll)
            scrollOffset = maxScroll;

        UpdateLayerRowPositions(hwnd);

        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }



    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        /// 背景を白で塗りつぶす（または任意の色）
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH hbr = (HBRUSH)(COLOR_WINDOW + 1);
        FillRect(hdc, &rc, hbr);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
    {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, (HBRUSH)(COLOR_WINDOW + 1));
        return 1; // 背景描画済み
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        // ON チェックボックス（12000 + index）
        if (id >= 12000 && id < 13000 && code == BN_CLICKED)
        {
            int index = id - 12000;

            extern LayerManager g_layerManager;

            HWND hCheck = (HWND)lParam;
            BOOL checked = (SendMessage(hCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);

            g_layerManager.layers[index].enabled = (checked == TRUE);

            // メインウィンドウを再描画（F1/F2どちらでも反映させる）
            extern HWND g_hMainWnd;
            InvalidateRect(g_hMainWnd, NULL, FALSE);

            return 0;
        }


        // ↑ ボタン
        if (id >= 13000 && id < 14000)
        {
            int index = id - 13000;

            extern LayerManager g_layerManager;

            if (index > 0)
            {
                // ★ 1. 内部データの順番を入れ替える
                std::swap(g_layerManager.layers[index], g_layerManager.layers[index - 1]);
            }

            // ★ 2. UI を破壊
            for (auto& row : g_layerRows) {
                DestroyWindow(row.hNameEdit);
                DestroyWindow(row.hLoadBtn);
                DestroyWindow(row.hEnableCheck);
                DestroyWindow(row.hUpBtn);
                DestroyWindow(row.hDownBtn);
                DestroyWindow(row.hDeleteBtn);
            }
            g_layerRows.clear();

            // ★ 3. UI を再生成
            for (size_t i = 0; i < g_layerManager.layers.size(); i++) {
                CreateLayerRow(hLayerPanel, i);
            }
            UpdateLayerRowPositions(hLayerPanel);

            // ★ 追加：スクロール位置をリセット
            scrollOffset = 0;
            SetScrollPos(hLayerPanel, SB_VERT, 0, TRUE);

            // ★ パネルを再描画
            InvalidateRect(hLayerPanel, NULL, TRUE);

            // ★ PNG の再ロード（Window モード）
            if (g_pRenderTarget) {
                g_layerBitmapsWindow.clear();
                for (auto& layer : g_layerManager.layers) {
                    if (!layer.filePath.empty()) {
                        g_layerBitmapsWindow[layer.name] =
                            LoadBitmapForTarget(g_pRenderTarget, layer.filePath.c_str());
                    }
                }
            }

            // ★ PNG の再ロード（Layered モード）
            if (g_pDCRenderTarget) {
                g_layerBitmapsLayered.clear();
                for (auto& layer : g_layerManager.layers) {
                    if (!layer.filePath.empty()) {
                        g_layerBitmapsLayered[layer.name] =
                            LoadBitmapForTarget(g_pDCRenderTarget, layer.filePath.c_str());
                    }
                }
            }

            extern HWND g_hMainWnd;
            InvalidateRect(g_hMainWnd, NULL, FALSE);

            return 0;
        }


        // ↓ ボタン
        if (id >= 14000 && id < 15000)
        {
            int index = id - 14000;

            extern LayerManager g_layerManager;

            if (index < g_layerManager.layers.size() - 1)
            {
                // ★ 1. 内部データの順番を入れ替える
                std::swap(g_layerManager.layers[index], g_layerManager.layers[index + 1]);
            }

            // ★ 2. UI を破壊
            for (auto& row : g_layerRows) {
                DestroyWindow(row.hNameEdit);
                DestroyWindow(row.hLoadBtn);
                DestroyWindow(row.hEnableCheck);
                DestroyWindow(row.hUpBtn);
                DestroyWindow(row.hDownBtn);
                DestroyWindow(row.hDeleteBtn);
            }
            g_layerRows.clear();

            // ★ 3. UI を再生成
            for (size_t i = 0; i < g_layerManager.layers.size(); i++) {
                CreateLayerRow(hLayerPanel, i);
            }
            UpdateLayerRowPositions(hLayerPanel);

            // ★ 追加：スクロール位置をリセット
            scrollOffset = 0;
            SetScrollPos(hLayerPanel, SB_VERT, 0, TRUE);

            // ★ パネルを再描画
            InvalidateRect(hLayerPanel, NULL, TRUE);

            // ★ PNG の再ロード（Window モード）
            if (g_pRenderTarget) {
                g_layerBitmapsWindow.clear();
                for (auto& layer : g_layerManager.layers) {
                    if (!layer.filePath.empty()) {
                        g_layerBitmapsWindow[layer.name] =
                            LoadBitmapForTarget(g_pRenderTarget, layer.filePath.c_str());
                    }
                }
            }

            // ★ PNG の再ロード（Layered モード）
            if (g_pDCRenderTarget) {
                g_layerBitmapsLayered.clear();
                for (auto& layer : g_layerManager.layers) {
                    if (!layer.filePath.empty()) {
                        g_layerBitmapsLayered[layer.name] =
                            LoadBitmapForTarget(g_pDCRenderTarget, layer.filePath.c_str());
                    }
                }
            }

            extern HWND g_hMainWnd;
            InvalidateRect(g_hMainWnd, NULL, FALSE);

            return 0;
        }


        // X ボタン（削除）
        if (id >= 15000 && id < 16000)
        {
            int index = id - 15000;

            extern LayerManager g_layerManager;
            extern HWND hLayerPanel;

            // 1. データ削除
            g_layerManager.RemoveLayer(index);

            // 2. UI 全破棄
            for (auto& row : g_layerRows) {
                DestroyWindow(row.hNameEdit);
                DestroyWindow(row.hLoadBtn);
                DestroyWindow(row.hEnableCheck);
                DestroyWindow(row.hUpBtn);
                DestroyWindow(row.hDownBtn);
                DestroyWindow(row.hDeleteBtn);
            }
            g_layerRows.clear();

            // 3. UI 再生成
            for (size_t i = 0; i < g_layerManager.layers.size(); i++) {
                CreateLayerRow(hLayerPanel, i);
            }
            UpdateLayerRowPositions(hLayerPanel);

            // ★ 追加：スクロール位置をリセット
            scrollOffset = 0;
            SetScrollPos(hLayerPanel, SB_VERT, 0, TRUE);

            // Window モード用ビットマップ再ロード
            LoadHeadBitmapsForWindow();

            // Layered モード用ビットマップ再ロード
            LoadHeadBitmapsForLayered();

            // 4. メイン描画も更新
            extern HWND g_hMainWnd;
            InvalidateRect(g_hMainWnd, NULL, FALSE);

            return 0;

        }



        // Load PNG ボタン
        if (id >= 11000 && id < 12000)
        {
            int index = id - 11000;

            HWND hParent = GetParent(hwnd);

            extern LayerManager g_layerManager;

            // PNG 選択ダイアログ
            wchar_t fileName[MAX_PATH] = L"";
            OPENFILENAME ofn = { sizeof(ofn) };
            ofn.lpstrFilter = L"PNG Files\0*.png\0";
            ofn.lpstrFile = fileName;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn))
            {
                g_layerManager.layers[index].filePath = fileName;

                // ★ Renderer 用にビットマップを読み込む
                if (g_pRenderTarget)
                {
                    ID2D1Bitmap* bmp = LoadBitmapForTarget(g_pRenderTarget, fileName);
                    g_layerBitmapsWindow[g_layerManager.layers[index].name] = bmp;
                }
                // ★ DCRenderTarget 用（レイヤードモード）
                if (g_pDCRenderTarget)
                {
                    ID2D1Bitmap* bmp2 = LoadBitmapForTarget(g_pDCRenderTarget, fileName);
                    g_layerBitmapsLayered[g_layerManager.layers[index].name] = bmp2;
                }

                // ★ 読み込んだら画面更新
                InvalidateRect(GetParent(hParent), NULL, FALSE);
            }

            return 0;
        }

        // EditBox の変更
        if (id >= 10000 && id < 11000)
        {
            int index = id - 10000;

            extern LayerManager g_layerManager;

            // 古い名前を保存
            std::wstring oldName = g_layerManager.layers[index].name;

            // 新しい名前を取得
            wchar_t buf[256];
            GetWindowText((HWND)lParam, buf, 256);
            g_layerManager.layers[index].name = buf;

            // ★ ビットマップのキーを更新（Window）
            if (g_layerBitmapsWindow.count(oldName)) {
                g_layerBitmapsWindow[buf] = g_layerBitmapsWindow[oldName];
                g_layerBitmapsWindow.erase(oldName);
            }

            // ★ ビットマップのキーを更新（Layered）
            if (g_layerBitmapsLayered.count(oldName)) {
                g_layerBitmapsLayered[buf] = g_layerBitmapsLayered[oldName];
                g_layerBitmapsLayered.erase(oldName);
            }

            return 0;
        }




        break;
    }


    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


// LayerRow の再配置（スクロール対応）
void UpdateLayerRowPositions(HWND hwnd)
{
    const int rowHeight = 30;

    for (size_t i = 0; i < g_layerRows.size(); i++) {
        int y = i * rowHeight - scrollOffset;

        MoveWindow(g_layerRows[i].hNameEdit, 5, y + 5, 120, 20, TRUE);
        MoveWindow(g_layerRows[i].hLoadBtn, 130, y + 5, 80, 20, TRUE);
        MoveWindow(g_layerRows[i].hEnableCheck, 215, y + 5, 40, 20, TRUE);
        MoveWindow(g_layerRows[i].hUpBtn, 260, y + 5, 25, 20, TRUE);
        MoveWindow(g_layerRows[i].hDownBtn, 290, y + 5, 25, 20, TRUE);
        MoveWindow(g_layerRows[i].hDeleteBtn, 320, y + 5, 25, 20, TRUE);
    }
}

void RebuildLayerPanelUI(HWND hDlg)
{
    extern HWND hLayerPanel;
    extern std::vector<LayerRowUI> g_layerRows;

    // 既存 UI 破棄
    for (auto& row : g_layerRows) {
        DestroyWindow(row.hNameEdit);
        DestroyWindow(row.hLoadBtn);
        DestroyWindow(row.hEnableCheck);
        DestroyWindow(row.hUpBtn);
        DestroyWindow(row.hDownBtn);
        DestroyWindow(row.hDeleteBtn);
    }
    g_layerRows.clear();

    // 再生成
    for (int i = 0; i < g_layerManager.layers.size(); i++) {
        CreateLayerRow(hLayerPanel, i);
    }
    UpdateLayerRowPositions(hLayerPanel);

    // スクロール位置リセット
    scrollOffset = 0;
    SetScrollPos(hLayerPanel, SB_VERT, 0, TRUE);
}
