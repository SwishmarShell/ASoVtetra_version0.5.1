#pragma once

#include <mutex>

// ------------------------------
// 表情データ
// ------------------------------
struct FaceParams {
    float jawOpen = 0.0f;
    float mouthOpen = 0.0f;
    float browUp = 0.0f;
    float eyeBlinkLeft = 0.0f;
    float eyeBlinkRight = 0.0f;

    float yaw = 0.0f;
    float browL = 0.0f;
    float browR = 0.0f;
};

// ------------------------------
// 口の状態
// ------------------------------
enum class MouthState { Closed, Open };

// ------------------------------
// Transform 設定
// ------------------------------
struct TransformSettings {
    float earMoveScale;
    float browUpY;
    float browUpRotL;
    float browUpRotR;
    float earringTopRotScale;
    float earringBottomRotScale;
    float faceMoveScale;

    float pivotEarringX;
    float pivotEarringY;

    float pivotBrowLX;
    float pivotBrowLY;

    float pivotBrowRX;
    float pivotBrowRY;

    float pivotEarX;
    float pivotEarY;
};
