#pragma once
#include <string>
#include <vector>

struct LayerSlot {
    std::wstring name;
    std::wstring filePath;
    bool enabled = true;
};

class LayerManager {
public:
    std::vector<LayerSlot> layers;

    void LoadConfig();   // JSON or INI
    void SaveConfig();

    void AddLayer(const std::wstring& name);
    void RemoveLayer(int index);
    void MoveLayerUp(int index);
    void MoveLayerDown(int index);

};
