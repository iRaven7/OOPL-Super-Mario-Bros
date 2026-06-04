#ifndef BACKGROUND_HPP
#define BACKGROUND_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <string>
#include <memory>

class Background : public Util::GameObject {
public:
    explicit Background(const std::string& imagePath) {
        m_Drawable = std::make_shared<Util::Image>(imagePath);
        // 設定成超小的負數，確保它永遠被壓在所有物件的最底層
        SetZIndex(-100);
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) {
        // 視差捲動魔法在這裡！
        // 乘以 0.1f 代表鏡頭移動 10 像素，背景只移動 1 像素，產生深遠的距離感
        m_Transform.translation.x = (-cameraX * 0.1f) * cameraZoom;

        // 假設背景圖很大，稍微調整 Y 軸讓天空比較多
        m_Transform.translation.y = 100.0f * cameraZoom;
        m_Transform.scale = glm::vec2(1.0f, 1.0f) * cameraZoom;
    }
};

#endif // BACKGROUND_HPP