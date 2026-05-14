#ifndef KOOPA_HPP
#define KOOPA_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "Util/Logger.hpp"

class Koopa : public Enemy {
public:
    // 定義慢慢龜的有限狀態機
    enum class State {
        Walking,
        ShellIdle,
        ShellMoving
    };

    // 註：請確認 RESOURCES_DIR 內的實際圖片路徑並進行替換
    Koopa(glm::vec2 startPos) : Enemy(RESOURCE_DIR"/Entities/Koopa/koopa1.png") {
        SetPosition(startPos);
        m_Velocity.x = -m_WalkSpeed;
        m_State = State::Walking;
        SetZIndex(40);
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        // 靜止龜殼狀態：僅套用重力，不給予水平推力
        if (m_State == State::ShellIdle) {
            UpdatePhysics(deltaTime, 0.0f, false, false, blocks);
            m_Velocity.x = 0.0f; // 確保摩擦力使其完全停止
            return;
        }

        // 根據狀態決定移動速度
        float currentSpeed = (m_State == State::ShellMoving) ? m_ShellSpeed : m_WalkSpeed;
        float inputDirection = (m_Velocity.x > 0.0f) ? 0.5f : -0.5f;

        // 套用物理運算
        UpdatePhysics(deltaTime, inputDirection, false, false, blocks);

        // 撞牆偵測與反彈邏輯
        if (m_Velocity.x == 0.0f) {
            m_Velocity.x = -inputDirection * currentSpeed;
        }
        else {
            m_Velocity.x = inputDirection * currentSpeed;
        }
    }

    void OnStomped(Character* hitter) override {
        if (!m_IsActive) return;

        if (m_State == State::Walking) {
            m_State = State::ShellIdle;
            m_Velocity.x = 0.0f;
            // TODO: 此處應呼叫 SetDrawable 切換為龜殼的圖片
            LOG_INFO("Koopa 被踩踏，進入靜止龜殼狀態");
        }
        else if (m_State == State::ShellMoving) {
            m_State = State::ShellIdle;
            m_Velocity.x = 0.0f;
            LOG_INFO("滑行龜殼被踩踏，停止滑行");
        }
        else if (m_State == State::ShellIdle) {
            // 踩踏靜止的龜殼，會將其踢出
            KickShell(hitter);
        }
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;

        Mario* mario = dynamic_cast<Mario*>(hitter);

        if (m_State == State::Walking || m_State == State::ShellMoving) {
            // 行走或滑行狀態下，側面碰到瑪利歐會造成傷害
            if (mario) {
                mario->TakeDamage();
            }
        }
        else if (m_State == State::ShellIdle) {
            // 側面碰到靜止龜殼，將其踢出
            KickShell(hitter);
        }
    }

    State GetState() const { return m_State; }

private:
    // 輔助函式：處理踢龜殼的物理向量
    void KickShell(Character* hitter) {
        m_State = State::ShellMoving;

        // 根據碰撞者的相對 X 座標，決定龜殼的射出方向
        if (hitter->GetPosition().x < GetPosition().x) {
            m_Velocity.x = m_ShellSpeed;  // 從左側撞擊，向右踢出
        }
        else {
            m_Velocity.x = -m_ShellSpeed; // 從右側撞擊，向左踢出
        }
        LOG_INFO("龜殼被踢出了！");
    }

    State m_State;
    float m_WalkSpeed = 100.0f;
    float m_ShellSpeed = 350.0f; // 滑行速度應顯著高於行走速度
};

#endif // KOOPA_HPP