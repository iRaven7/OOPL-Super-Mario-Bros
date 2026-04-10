#ifndef ITEM_HPP
#define ITEM_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Block.hpp"
#include "Mario.hpp" // 需要知道瑪利歐以觸發變身
#include <vector>
#include <memory>

class Item : public Util::GameObject {
public:
    explicit Item(const std::string& imagePath) {
        m_ImagePath = imagePath;
        m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
    }

    virtual void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) = 0;
    virtual void OnCollect(Mario* mario) = 0;
    virtual void OnBlockBumped(float blockCenterX) {}// 傳入方塊的 X 座標以判斷左右

    bool IsActive() const { return m_IsActive; }
    glm::vec2 GetPosition() const {
        return m_WorldPosition;
    }
    void SetPosition(const glm::vec2& Position) {
        m_WorldPosition = Position;
        m_Transform.translation = Position;
    }
    void UpdateRenderPosition(float cameraX) {
        m_Transform.translation = { m_WorldPosition.x - cameraX, m_WorldPosition.y };
    }
    glm::vec2 GetSize() const { return { 16.0f, 16.0f }; }

protected:
    std::string m_ImagePath;
    glm::vec2 m_Velocity = { 0.0f, 0.0f };
    bool m_IsActive = true;
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
};

#endif // ITEM_HPP