#ifndef GOOMBA_HPP
#define GOOMBA_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "GameStateManager.hpp"
#include "Util/Logger.hpp"

class Goomba : public Enemy {
public:
    Goomba(glm::vec2 startPos) : Enemy(RESOURCE_DIR"/Entities/Goomba/goomba.png") {
        SetPosition(startPos);
        m_Velocity.x = -m_WalkSpeed;
        SetZIndex(40);
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        float inputDirection = (m_Velocity.x > 0.0f) ? 0.5f : -0.5f;

        UpdatePhysics(deltaTime, inputDirection, false, false, blocks);

        if (m_Velocity.x == 0.0f) {
            m_Velocity.x = -inputDirection * m_WalkSpeed;
        }
        else {
            m_Velocity.x = inputDirection * m_WalkSpeed;
        }
    }

    void OnStomped(Character* hitter) override {
        m_IsActive = false;
        m_Visible = false;
        GameStateManager::GetInstance().AddScore(100);
        LOG_INFO("Goomba stomped!");
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;

        Mario* mario = dynamic_cast<Mario*>(hitter);
        if (mario) {
            mario->TakeDamage();
        }
    }

private:
    float m_WalkSpeed = 70.0f;
};

#endif // GOOMBA_HPP
