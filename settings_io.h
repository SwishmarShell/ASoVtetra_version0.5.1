#pragma once
#include <string>

// INI float “Ç‚İ‚İ
float GetPrivateProfileFloat(const wchar_t* section, const wchar_t* key, float def, const wchar_t* ini);

// settings.ini ‚Ì•Û‘¶
void SaveSettings();

// settings.ini ‚Ì“Ç‚İ‚İ
void LoadSettings();
