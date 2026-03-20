#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <string>
#include "Util/GameObject.hpp"

class Character : public Util::GameObject {
public:
    explicit Character(const std::string& ImagePath);

    // 禁用拷貝與搬移
    Character(const Character&) = delete;
    Character(Character&&) = delete;
    Character& operator=(const Character&) = delete;
    Character& operator=(Character&&) = delete;

    // --- 修改：加入 jump 輸入，並回傳當前速度 (供攝影機判斷) ---
    glm::vec2 UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump);

    [[nodiscard]] const std::string& GetImagePath() const { return m_ImagePath; }
    [[nodiscard]] const glm::vec2& GetPosition() const { return m_Transform.translation; }
    [[nodiscard]] bool GetVisibility() const { return m_Visible; }

    void SetImage(const std::string& ImagePath);
    void SetPosition(const glm::vec2& Position) { m_Transform.translation = Position; }
    // 用於 Week 5 碰撞偵測設定接地狀態
    void SetGrounded(bool grounded) { m_IsGrounded = grounded; }


    // 新增：供子類別判斷是否能跳躍
    bool IsGrounded() const { return m_IsGrounded; }

    // 物理更新邏輯，需傳入幀時間差 (Delta Time) 以確保不同硬體上的移動距離一致


private:
    std::string m_ImagePath;

    // --- 物理狀態變數 ---
    glm::vec2 m_Velocity = { 0.0f, 0.0f };
    bool m_IsGrounded = false; // 判斷是否在地面

    // --- 物理系統常數 (數值需依遊戲視窗與方塊比例微調) ---
    // 假設 Y 軸向下為負 (Cartesian 座標系)
    static constexpr float GRAVITY = -2400.0f;
    static constexpr float MAX_FALL_SPEED = -900.0f;
    static constexpr float JUMP_FORCE = 850.0f;

    static constexpr float WALK_ACCEL = 800.0f;      // 一般步行加速度
    static constexpr float SPRINT_ACCEL = 1200.0f;   // 衝刺加速度
    static constexpr float MAX_WALK_SPEED = 300.0f;  // 步行最高速
    static constexpr float MAX_SPRINT_SPEED = 550.0f;// 衝刺最高速

    static constexpr float FRICTION = 600.0f;        // 無輸入時的自然摩擦力
    static constexpr float SKID_DECEL = 1800.0f;     // 轉向時的強烈煞車阻力 (Skidding)
};

#endif //CHARACTER_HPP