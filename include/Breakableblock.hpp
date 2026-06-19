#ifndef BREAKABLE_BLOCK_HPP
#define BREAKABLE_BLOCK_HPP

#include "Block.hpp"
#include "Character.hpp"
#include "GameStateManager.hpp"

class BreakableBlock : public Block {
public:
    explicit BreakableBlock(const std::string& imagePath) : Block(imagePath) {
        SetDestructible(true);
    }

    bool IsActive() const override {
        return m_IsActive;
    }

    glm::vec2 GetCollisionPosition() const override {
        return { m_WorldPosition.x, m_OriginalY };
    }

    void SetPosition(const glm::vec2& Position) override {
        Block::SetPosition(Position);
        if (!m_IsBouncing) m_OriginalY = Position.y;
    }

    void OnHit(Character* hitter) override {
        if (!m_IsActive) return;

        m_JustHit = true;

        if (IsDestructible() && hitter->CanBreakBlocks()) {
            m_IsActive = false;
            SetVisible(false);
            GameStateManager::GetInstance().AddScore(50);   // brick broken
        }
        else {
            if (!m_IsBouncing) {
                m_IsBouncing = true;
                m_BounceVelocity = 180.0f;
            }
        }
    }

    void Update(float deltaTime) override {
        if (!m_IsBouncing) return;

        m_BounceVelocity -= 2500.0f * deltaTime;
        glm::vec2 currentPos = GetPosition();
        currentPos.y += m_BounceVelocity * deltaTime;

        if (currentPos.y <= m_OriginalY) {
            currentPos.y = m_OriginalY;
            m_IsBouncing = false;
            m_BounceVelocity = 0.0f;
        }
        Block::SetPosition(currentPos);
    }

private:
    bool  m_IsActive      = true;
    bool  m_IsBouncing    = false;
    float m_OriginalY     = 0.0f;
    float m_BounceVelocity = 0.0f;
};
#endif
