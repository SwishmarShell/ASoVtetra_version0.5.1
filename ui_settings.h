#pragma once
#include <windows.h>
#include "resource.h"

// ダイアログプロシージャ
INT_PTR CALLBACK SettingsProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK InputNameProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void CreateLayerRow(HWND hParent, int index);
void UpdateLayerRowPositions(HWND hwnd);

// UI ボタンハンドラ
void OnPresetSave(HWND hDlg);
void OnPresetLoad(HWND hDlg);
void OnPresetDelete(HWND hDlg);
