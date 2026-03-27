#ifndef BREAKABLE_BLOCK_HPP
#define BREAKABLE_BLOCK_HPP

#include "Block.hpp"
#include "Character.hpp" // 需要知道 Character 的介面

class BreakableBlock : public Block {
public:
    explicit BreakableBlock(const std::string& imagePath) : Block(imagePath) {}

    void SetPosition(const glm::vec2& Position) {
        Block::SetPosition(Position);
        if (!m_IsBouncing) m_OriginalY = Position.y; // 記錄原始高度
    }

    void OnHit(Character* hitter) override {
        if (!m_IsActive) return;

        if (hitter->CanBreakBlocks()) {
            // 大型瑪利歐：直接破壞
            m_IsActive = false;
            m_Visible = false;
        }
        else {
            // 小型瑪利歐：觸發彈跳動畫 (避免重複觸發)
            if (!m_IsBouncing) {
                m_IsBouncing = true;
                m_BounceVelocity = 150.0f; // 設定向上初速度
            }
        }
    }

    void Update(float deltaTime) override {
        if (!m_IsBouncing) return;

        // 簡易物理運算：速度受重力遞減，更新 Y 座標
        m_BounceVelocity -= 600.0f * deltaTime; // 600.0f 為方塊專用重力常數
        glm::vec2 currentPos = GetPosition();
        currentPos.y += m_BounceVelocity * deltaTime;

        // 若落回原點或更低，則停止動畫並重置位置
        if (currentPos.y <= m_OriginalY) {
            currentPos.y = m_OriginalY;
            m_IsBouncing = false;
            m_BounceVelocity = 0.0f;
        }

        // 注意：需使用基底的 SetPosition 避免覆寫 m_OriginalY
        Block::SetPosition(currentPos);
    }

private:
    bool m_IsActive = true;
    bool m_IsBouncing = false;
    float m_OriginalY = 0.0f;
    float m_BounceVelocity = 0.0f;
};
#endif