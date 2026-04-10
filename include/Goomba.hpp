#ifndef GOOMBA_HPP
#define GOOMBA_HPP

#include "Enemy.hpp"
#include "Util/Logger.hpp"

class Goomba : public Enemy {
public:
    Goomba(glm::vec2 startPos) : Enemy(RESOURCE_DIR"/Entities/Enemies/goomba.png") {
        SetPosition(startPos);
        m_Velocity.x = -m_WalkSpeed; // 預設向左走
        SetZIndex(40);
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        // 決定虛擬的輸入方向
        float inputDirection = (m_Velocity.x > 0.0f) ? 1.0f : -1.0f;

        // 利用基底類別的 UpdatePhysics 處理重力與地形阻擋
        // 傳入 false (不衝刺) 與 false (不跳躍)
        UpdatePhysics(deltaTime, inputDirection, false, false, blocks);

        // 強制鎖定步速，避免被 Character 內部的加速度邏輯無限疊加
        m_Velocity.x = inputDirection * m_WalkSpeed;

        // 偵測撞牆：如果實體被阻擋，UpdatePhysics 會將 m_Velocity.x 設為 0
        if (m_Velocity.x == 0.0f) {
            m_Velocity.x = -inputDirection * m_WalkSpeed; // 反轉方向
        }
    }

    void OnStomped(Character* hitter) override {
        m_IsActive = false;
        m_Visible = false; // 未來可改為切換至「被踩扁」的圖片並停留 0.5 秒
        LOG_INFO("Goomba 被踩死了！");
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;
        LOG_INFO("瑪利歐受到傷害！");
        // 未來在此處呼叫 hitter(瑪利歐) 的受擊降級邏輯
    }

private:
    float m_WalkSpeed = 100.0f;
};

#endif // GOOMBA_HPP