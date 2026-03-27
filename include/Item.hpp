#ifndef ITEM_HPP
#define ITEM_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

class Item : public Util::GameObject {
public:
    explicit Item(const std::string& imagePath) {
        m_ImagePath = imagePath;
        m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
        SetZIndex(40); // 確保層級在背景之上，通常在方塊後方或前方
    }

    virtual void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) = 0;
    virtual void OnCollect() = 0; // 被瑪利歐碰到時觸發

    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    glm::vec2 GetPosition() const { return m_Transform.translation; }
    glm::vec2 GetSize() const { return { 16.0f, 16.0f }; } // 預設尺寸

protected:
    std::string m_ImagePath;
    glm::vec2 m_Velocity = { 0.0f, 0.0f };
    bool m_IsActive = true;
};
#endif