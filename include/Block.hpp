#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <string>
#include <memory>

class Character;
class Item; // 前置宣告，供 PopSpawnedItem 使用

class Block : public Util::GameObject {
public:
    explicit Block(const std::string& imagePath) {
        SetImage(imagePath);
    }

    virtual ~Block() = default;

    void SetImage(const std::string& imagePath) {
        m_ImagePath = imagePath;
        m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
    }

    // ==========================================
    // 座標與算繪邏輯 (World Space vs Screen Space)
    // ==========================================
    virtual void SetPosition(const glm::vec2& Position) {
        m_WorldPosition = Position;
        m_Transform.translation = Position;
    }

    virtual glm::vec2 GetPosition() const {
        return m_WorldPosition;
    }

    virtual glm::vec2 GetCollisionPosition() const {
        return m_WorldPosition;
    }

    virtual glm::vec2 GetSize() const {
        return { 16.0f, 16.0f };
    }

    virtual void UpdateRenderPosition(float cameraX, float cameraZoom) {
        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = m_WorldPosition.y * cameraZoom;
        m_Transform.scale = m_BaseScale * cameraZoom;
    }

    // ==========================================
    // 物理與互動邏輯
    // ==========================================
    virtual void OnHit(Character*) {
        m_JustHit = true; // 預設記錄受擊標記
    }

    // 讀取並重置受擊狀態
    bool PopJustHit() {
        bool val = m_JustHit;
        m_JustHit = false;
        return val;
    }

    virtual void Update(float) {}
    virtual bool IsActive() const { return true; }
    virtual std::shared_ptr<Item> PopSpawnedItem() { return nullptr; }

protected: // 使用 protected 讓衍生類別 (BreakableBlock, QuestionBlock) 可直接存取
    std::string m_ImagePath;
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
    bool m_JustHit = false;
    glm::vec2 m_BaseScale = { 1.0f, 1.0f };
};

#endif // BLOCK_HPP