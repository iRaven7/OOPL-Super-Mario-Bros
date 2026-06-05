#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <glm/glm.hpp> // 明確加上這個
#include <string>
#include <memory>

class Character;
class Item;

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

    virtual void OnHit(Character*) {
        m_JustHit = true;
    }

    bool PopJustHit() {
        bool val = m_JustHit;
        m_JustHit = false;
        return val;
    }

    virtual void Update(float) {}
    virtual bool IsActive() const { return true; }
    virtual std::shared_ptr<Item> PopSpawnedItem() { return nullptr; }
    virtual bool IsPipeEntrance() const { return false; }
    virtual int GetTargetLevel() const { return -1; }

protected:
    std::string m_ImagePath;
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
    bool m_JustHit = false;
    glm::vec2 m_BaseScale = { 1.0f, 1.0f };
};

#endif // BLOCK_HPP