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
        // 把深度調到 -50，確保它在所有方塊的最底層，但又不會被引擎吃掉
        SetZIndex(1);
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) {
        // 座標固定在 0.0f，這樣它就會像貼紙一樣黏在鏡頭上了
        m_Transform.translation.x = 0.0f;
        m_Transform.translation.y = 0.0f;

        // 把放大倍率直接開到 20 倍！保證把整個鏡頭塞得滿滿的啊
        m_Transform.scale = glm::vec2(200.0f, 200.0f) * cameraZoom;
    }
};

#endif // BACKGROUND_HPP