#pragma once
#include <string>

bool SaveProjectZip(const std::wstring& zipPath);
bool LoadProjectZip(const std::wstring& zipPath);

// temp フォルダ削除用
bool RemoveDirectoryRecursive(const std::wstring& dir);
