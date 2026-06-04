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
        SetZIndex(-100); // 絕對要在最底層
    }

    // 覆寫渲染邏輯，不要扣掉 cameraX，它就會永遠跟著螢幕了！
    void UpdateRenderPosition(float cameraX, float cameraZoom) {
        // 直接鎖死在螢幕中央附近，並把 scale 放大來塞滿畫面
        m_Transform.translation.x = 0.0f;
        m_Transform.translation.y = 0.0f;
        m_Transform.scale = glm::vec2(3.0f, 3.0f) * cameraZoom; // 依你的圖片大小調整這個放大倍率
    }
};

#endif // BACKGROUND_HPP