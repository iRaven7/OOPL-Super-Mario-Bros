#ifndef KOOPA_HPP
#define KOOPA_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "Util/Logger.hpp"
#include "Util/Image.hpp"

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

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        float yOffset = 0.0f;

        // 假設慢慢龜站立圖片比龜殼高 16 像素，中心點會高 8 像素。
        // 請依據你實際放入的圖片尺寸調整此數值 (例如 8.0f 或 16.0f)
        if (m_State == State::Walking) {
            yOffset = 8.0f;
        }

        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = (m_WorldPosition.y + yOffset) * cameraZoom;

        // 維持左右翻轉邏輯
        float direction = (m_Velocity.x > 0.0f) ? -1.0f : 1.0f;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
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

            // 切換為龜殼圖片 (請確保檔名與路徑正確)
            SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Entities/Koopa/shell.png"));

            // 由於龜殼高度變矮，碰撞箱(Hitbox)的高度若能調整會更精確
            // m_BaseScale.y = 0.5f; // 視你的實作需求而定

            LOG_INFO("Koopa 進入靜止龜殼狀態");
        }
        else if (m_State == State::ShellMoving) {
            m_State = State::ShellIdle;
            m_Velocity.x = 0.0f;
            LOG_INFO("滑行龜殼停止");
        }
        else if (m_State == State::ShellIdle) {
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