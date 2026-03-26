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

    // 更新物理函式簽名，注入環境方塊依賴
    glm::vec2 UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump, const std::vector<std::shared_ptr<Block>>& blocks);

    [[nodiscard]] const std::string& GetImagePath() const { return m_ImagePath; }
    [[nodiscard]] const glm::vec2& GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] bool GetVisibility() const { return m_Visible; }
    [[nodiscard]] bool IsGrounded() const { return m_IsGrounded; }

    // 設定角色實體邊界大小
    [[nodiscard]] glm::vec2 GetSize() const {
        return { 16.0f, 16.0f };
    }

    void SetImage(const std::string& ImagePath);
    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    void SetGrounded(bool grounded) { m_IsGrounded = grounded; }

private:
    // AABB 碰撞幾何測試
    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
        return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
            std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
    }

    std::string m_ImagePath;

    glm::vec2 m_Velocity = { 0.0f, 0.0f };
    bool m_IsGrounded = false;

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