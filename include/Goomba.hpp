#ifndef GOOMBA_HPP
#define GOOMBA_HPP

#include "Enemy.hpp"
#include "GameStateManager.hpp"
#include "Util/Logger.hpp"

class Goomba : public Enemy {
public:
    Goomba(glm::vec2 startPos) : Enemy(RESOURCE_DIR"/Entities/Goomba/goomba.png") {
        SetPosition(startPos);
        m_Velocity.x = -m_WalkSpeed; // 預設向左走
        SetZIndex(40);
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        // 決定虛擬的輸入方向
        float inputDirection = (m_Velocity.x > 0.0f) ? 0.5f : -0.5f;

        // 利用基底類別的 UpdatePhysics 處理重力與地形阻擋
        UpdatePhysics(deltaTime, inputDirection, false, false, blocks);

        // 偵測撞牆：如果實體被阻擋，UpdatePhysics 會將 m_Velocity.x 設為 0
        if (m_Velocity.x == 0.0f) {
            // 撞牆了，給予反方向的速度
            m_Velocity.x = -inputDirection * m_WalkSpeed;
        }
        else {
            // 沒撞牆，維持固定的巡邏速度 (覆蓋掉 Character 內部的加速度疊加)
            m_Velocity.x = inputDirection * m_WalkSpeed;
        }
    }

    void OnStomped(Character* hitter) override {
        m_IsActive = false;
        m_Visible = false; // 未來可改為切換至「被踩扁」的圖片並停留 0.5 秒
        GameStateManager::GetInstance().AddScore(100);
        LOG_INFO("Goomba 被踩死了！");
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;

        // 嘗試將 hitter 轉型為 Mario 以呼叫其專屬方法
        Mario* mario = dynamic_cast<Mario*>(hitter);
        if (mario) {
            mario->TakeDamage();
        }
    }

private:
    float m_WalkSpeed = 100.0f;
};

#endif // GOOMBA_HPP