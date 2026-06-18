#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <glm/glm.hpp>
#include <string>
#include <memory>

class Character;
class Item;

class Block : public Util::GameObject {
public:
    enum class PipeEntryDir { Down, Right, Left };

    // What item this block holds; NONE means no contents.
    enum class ContentType { NONE, MUSHROOM, COIN, FIREFLOWER, ONEUP, STAR, SUPERFLOWER };

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
    virtual float GetSpawnX() const { return -300.0f; }
    virtual float GetVelocityY() const { return 0.0f; }
    virtual PipeEntryDir GetPipeEntryDir() const { return PipeEntryDir::Down; }

    // --- Contents: what item lives inside this block, and how many remain ---
    bool HasContents() const { return m_ContentType != ContentType::NONE && m_ContentCount > 0; }
    ContentType GetContentType() const { return m_ContentType; }
    int GetContentCount() const { return m_ContentCount; }
    void SetContents(ContentType type, int count = 1) { m_ContentType = type; m_ContentCount = count; }

    // --- Collision: whether physics resolves contact with this block ---
    virtual bool IsCollidable() const { return m_IsCollidable; }
    virtual bool IsXCollidable() const { return m_IsXCollidable; }
    void SetCollidable(bool col) { m_IsCollidable = col; }
    void SetXCollidable(bool col) { m_IsXCollidable = col; }

    // --- Destructibility: whether this block can be broken by players or objects ---
    bool IsDestructible() const { return m_IsDestructible; }
    void SetDestructible(bool d) { m_IsDestructible = d; }

    // --- Visibility: wraps the built-in Util::GameObject::m_Visible / SetVisible() ---
    bool IsVisible() const { return m_Visible; }
    // Use SetVisible(bool) (inherited from Util::GameObject) to show or hide the sprite.

protected:
    std::string m_ImagePath;
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
    bool m_JustHit = false;
    glm::vec2 m_BaseScale = { 1.0f, 1.0f };

    ContentType m_ContentType   = ContentType::NONE;
    int         m_ContentCount  = 0;
    bool        m_IsCollidable  = true;
    bool        m_IsXCollidable = true;
    bool        m_IsDestructible = false;
};

#endif // BLOCK_HPP
