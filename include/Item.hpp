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

    bool IsActive() const { return m_IsActive; }
    glm::vec2 GetPosition() const { return m_Transform.translation; }
    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    glm::vec2 GetSize() const { return { 16.0f, 16.0f }; }

protected:
    std::string m_ImagePath;
    glm::vec2 m_Velocity = { 0.0f, 0.0f };
    bool m_IsActive = true;
};

#endif // ITEM_HPP