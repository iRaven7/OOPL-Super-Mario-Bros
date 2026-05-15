#ifndef KOOPA_HPP
#define KOOPA_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "Util/Logger.hpp"
#include "Util/Image.hpp"

class Koopa : public Enemy {
public:
    enum class State {
        Walking,
        ShellIdle,
        ShellMoving
    };

    Koopa(glm::vec2 startPos) : Enemy(RESOURCE_DIR"/Entities/Koopa/koopa1.png") {
        // 【關鍵修正 1】：強制初始化縮放矩陣，阻絕記憶體亂數造成的無限大或隱形
        m_BaseScale = { 1.0f, 1.0f };
        m_Transform.scale = m_BaseScale;

        // 提升初始生成高度，防止較高的慢慢龜一生成就卡進地底磚塊內
        startPos.y += 12.0f;
        SetPosition(startPos);

        m_Velocity.x = -m_WalkSpeed;
        m_State = State::Walking;
        SetZIndex(35); // 將層級微調，以利區分與其他敵人的算繪順序
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        float yOffset = 0.0f;

        if (m_State == State::Walking) {
            yOffset = 8.0f; // 請依據實際 koopa.png 的高度落差微調此常數
        }
        else {
            // 龜殼狀態 (shell.png) 通常是 16x16，與基礎圖塊等高，通常不需要補償
            yOffset = 0.0f;
        }

        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;

        // 將 yOffset 加入最終的 Y 軸算繪座標中 (不影響真實的 m_WorldPosition)
        m_Transform.translation.y = (m_WorldPosition.y + yOffset) * cameraZoom;

        float direction = (m_Velocity.x > 0.0f) ? -1.0f : 1.0f;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        if (m_State == State::ShellIdle) {
            UpdatePhysics(deltaTime, 0.0f, false, false, blocks);
            m_Velocity.x = 0.0f;
            return;
        }

        float currentSpeed = (m_State == State::ShellMoving) ? m_ShellSpeed : m_WalkSpeed;
        float inputDirection = (m_Velocity.x > 0.0f) ? 0.5f : -0.5f;

        UpdatePhysics(deltaTime, inputDirection, false, false, blocks);

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

            // 【關鍵修正 2】：確保你的圖片檔名大小寫與此處完全一致 (如 shell.png 或 Shell.png)
            SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Entities/Koopa/shell.png"));

            // 移除手動的 Y 軸硬編碼位移，讓下一個 Frame 的 UpdatePhysics 重力自然將矮龜殼往下拉至地面

            LOG_INFO("Koopa 進入靜止龜殼狀態");
        }
        else if (m_State == State::ShellMoving) {
            m_State = State::ShellIdle;
            m_Velocity.x = 0.0f;
            LOG_INFO("滑行龜殼被踩踏，停止滑行");
        }
        else if (m_State == State::ShellIdle) {
            KickShell(hitter);
        }
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;

        Mario* mario = dynamic_cast<Mario*>(hitter);

        if (m_State == State::Walking || m_State == State::ShellMoving) {
            if (mario) {
                mario->TakeDamage();
            }
        }
        else if (m_State == State::ShellIdle) {
            KickShell(hitter);
        }
    }

    State GetState() const { return m_State; }

private:
    void KickShell(Character* hitter) {
        m_State = State::ShellMoving;

        // 依據碰撞來源相對位置射出龜殼
        if (hitter->GetPosition().x < GetPosition().x) {
            m_Velocity.x = m_ShellSpeed;
        }
        else {
            m_Velocity.x = -m_ShellSpeed;
        }
        LOG_INFO("龜殼被踢出了！");
    }

    State m_State;
    float m_WalkSpeed = 50.0f;
    float m_ShellSpeed = 350.0f;
};

#endif // KOOPA_HPP