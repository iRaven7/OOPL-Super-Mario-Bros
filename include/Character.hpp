#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>
#include <vector>
#include <memory>
#include "Util/GameObject.hpp"
#include "Block.hpp"

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

    glm::vec2 GetVelocity() const { return m_Velocity;}
    

    // 設為 virtual，供後續大型瑪利歐覆寫碰撞邊界
    [[nodiscard]] virtual glm::vec2 GetSize() const {
        return { 16.0f, 16.0f };
    }

    void SetImage(const std::string& ImagePath);
    void SetGrounded(bool grounded) { m_IsGrounded = grounded; }
    void SetVelocity(glm::vec2 v) { m_Velocity = v;}
    virtual bool CanBreakBlocks() const { return false; }

    // ==========================================
    // 座標與算繪邏輯 (World Space vs Screen Space)
    // ==========================================
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

        // 修正：保留 UpdateAnimation 設定的左右翻轉狀態 (擷取 X 軸的正負號)
        float direction = (m_Transform.scale.x < 0.0f) ? -1.0f : 1.0f;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

protected:
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
    glm::vec2 m_Velocity = { 0.0f, 0.0f }; // 將速度移至 protected，允許子類別直接修改
    bool m_IsGrounded = false;             // 將接地狀態移至 protected
    glm::vec2 m_BaseScale = { 1.0f, 1.0f };

private:
    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
        return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
            std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
    }

    std::string m_ImagePath;

    // 物理常數
    static constexpr float GRAVITY = -2400.0f;
    static constexpr float MAX_FALL_SPEED = -900.0f;
    static constexpr float JUMP_FORCE = 850.0f;
    static constexpr float WALK_ACCEL = 800.0f;
    static constexpr float SPRINT_ACCEL = 1200.0f;
    static constexpr float MAX_WALK_SPEED = 300.0f;
    static constexpr float MAX_SPRINT_SPEED = 550.0f;
    static constexpr float FRICTION = 600.0f;
    static constexpr float SKID_DECEL = 1800.0f;
};

#endif // CHARACTER_HPP