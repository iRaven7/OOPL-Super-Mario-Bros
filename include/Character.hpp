// include/Character.hpp
#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>
#include <vector>
#include <memory>
#include "Util/GameObject.hpp"

// 用前置宣告代替 include
class Block;

class Character : public Util::GameObject {
public:
    explicit Character(const std::string& ImagePath);

    Character(const Character&) = delete;
    Character(Character&&) = delete;
    Character& operator=(const Character&) = delete;
    Character& operator=(Character&&) = delete;

    glm::vec2 UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump, const std::vector<std::shared_ptr<Block>>& blocks);

    [[nodiscard]] const std::string& GetImagePath() const { return m_ImagePath; }
    [[nodiscard]] bool GetVisibility() const { return m_Visible; }
    [[nodiscard]] bool IsGrounded() const { return m_IsGrounded; }

    glm::vec2 GetVelocity() const { return m_Velocity; }

    [[nodiscard]] virtual glm::vec2 GetSize() const {
        return { 16.0f, 16.0f };
    }

    void SetImage(const std::string& ImagePath);
    void SetGrounded(bool grounded) { m_IsGrounded = grounded; }
    void SetVelocity(glm::vec2 v) { m_Velocity = v; }
    virtual bool CanBreakBlocks() const { return false; }

    void SetPosition(const glm::vec2& Position) {
        m_WorldPosition = Position;
        m_Transform.translation = Position;
    }

    const glm::vec2& GetPosition() const {
        return m_WorldPosition;
    }

    virtual void UpdateRenderPosition(float cameraX, float cameraZoom) {
        float yOffset = 0.0f;
        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = (m_WorldPosition.y + yOffset) * cameraZoom;

        float direction = (m_Transform.scale.x < 0.0f) ? -1.0f : 1.0f;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

protected:
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
    glm::vec2 m_Velocity = { 0.0f, 0.0f };
    bool m_IsGrounded = false;
    glm::vec2 m_BaseScale = { 1.0f, 1.0f };

private:
    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
        return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
            std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
    }

    std::string m_ImagePath;
};

#endif // CHARACTER_HPP