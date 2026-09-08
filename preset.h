#pragma once
#include <string>

// プリセット保存
void SavePreset(const std::wstring& name);

// プリセット読み込み
void LoadPreset(const std::wstring& name);

// プリセット削除
void DeletePreset(const std::wstring& name);

// プリセットのエクスポート
void ExportPreset(const std::wstring& name);

// プリセットのインポート
void ImportPreset();
