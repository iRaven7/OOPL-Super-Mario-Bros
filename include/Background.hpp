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
        SetZIndex(-50);
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) {
        m_Transform.translation.x = 0.0f;
        m_Transform.translation.y = 0.0f;

        // Y 座標稍微往下壓一點，確保能填滿螢幕
        m_Transform.translation.y = -200.0f * cameraZoom;

        // 放大個 5 倍通常就很夠塞滿畫面了
        m_Transform.scale = glm::vec2(1000.0f, 1000.0f) * cameraZoom;
    }
};

#endif // BACKGROUND_HPP