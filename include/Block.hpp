#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <string>

class Character;
class Item;

class Block : public Util::GameObject {
public:
    explicit Block(const std::string& imagePath) {
        SetImage(imagePath);
    }

    virtual ~Block() = default; // 確保多型資源正確釋放

    virtual void OnHit(Character* hitter) {} // 傳入撞擊者

    virtual void Update(float deltaTime) {}  // 供子類別實作動畫

    virtual glm::vec2 GetCollisionPosition() const {
        return m_Transform.translation;
    }

    virtual std::shared_ptr<Item> PopSpawnedItem() {
        return nullptr;
    }

    void SetImage(const std::string& imagePath) {
        m_ImagePath = imagePath;
        m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
    }

    void SetPosition(const glm::vec2& Position) {
        m_Transform.translation = Position;
    }

    [[nodiscard]] glm::vec2 GetPosition() const {
        return m_Transform.translation;
    }

    [[nodiscard]] glm::vec2 GetSize() const {
        return { 16.0f, 16.0f };
    }

    // 新增：多型介面
    virtual void OnHit() {} // 預設一般方塊被撞擊無反應
    virtual bool IsActive() const { return true; } // 預設一般方塊永遠具備實體

private:
    std::string m_ImagePath;
};

#endif // BLOCK_HPP