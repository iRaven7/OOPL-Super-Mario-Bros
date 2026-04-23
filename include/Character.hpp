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

    void UpdateRenderPosition(float cameraX) {
        // 若碰撞框高度大於基本圖片高度 (16.0f)，計算需要下移的視覺偏移量
        float yVisualOffset = 0.0f;
        if (GetSize().y > 16.0f) {
            yVisualOffset = (GetSize().y - 16.0f) / 2.0f;
        }

        // 渲染時，扣除攝影機 X 偏移，並將 Y 座標下移以對齊腳底
        m_Transform.translation = { m_WorldPosition.x - cameraX, m_WorldPosition.y - yVisualOffset };
    }

protected:
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
    glm::vec2 m_Velocity = { 0.0f, 0.0f }; // 將速度移至 protected，允許子類別直接修改
    bool m_IsGrounded = false;             // 將接地狀態移至 protected

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