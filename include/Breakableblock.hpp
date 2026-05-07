#ifndef BREAKABLE_BLOCK_HPP
#define BREAKABLE_BLOCK_HPP

#include "Block.hpp"
#include "Character.hpp" // 需要知道 Character 的介面

class BreakableBlock : public Block {
public:
    explicit BreakableBlock(const std::string& imagePath) : Block(imagePath) {}

    bool IsActive() const override {
        return m_IsActive;
    }

    // 在 BreakableBlock 類別中新增或修改此段：
    glm::vec2 GetCollisionPosition() const override {
        return { m_WorldPosition.x, m_OriginalY };
    }

    // 注意：SetPosition 必須加上 override 以確保多型呼叫正確
    void SetPosition(const glm::vec2& Position) override {
        Block::SetPosition(Position);
        if (!m_IsBouncing) m_OriginalY = Position.y;
    }

    void OnHit(Character* hitter) override {
        if (!m_IsActive) return;

        m_JustHit = true; // 標記此方塊在此影格被撞擊

        if (hitter->CanBreakBlocks()) {
            // 大型瑪利歐：直接破壞
            m_IsActive = false;
            m_Visible = false;
        }
        else {
            // 小型瑪利歐：觸發彈跳動畫 (避免重複觸發)
            if (!m_IsBouncing) {
                m_IsBouncing = true;
                // 提高初始向上速度（從 150.0f 提升至 250.0f）
                m_BounceVelocity = 180.0f;
            }
        }
    }

    void Update(float deltaTime) override {
        if (!m_IsBouncing) return;

        // 加大專用重力（從 600.0f 提升至 1500.0f）
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
    bool m_IsActive = true;
    bool m_IsBouncing = false;
    float m_OriginalY = 0.0f;
    float m_BounceVelocity = 0.0f;
};
#endif