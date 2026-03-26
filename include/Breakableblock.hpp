#ifndef BREAKABLE_BLOCK_HPP
#define BREAKABLE_BLOCK_HPP

#include "Block.hpp"

class BreakableBlock : public Block {
public:
    explicit BreakableBlock(const std::string& imagePath) : Block(imagePath) {}

    // 覆寫撞擊行為
    void OnHit() override {
        if (m_IsActive) {
            m_IsActive = false;
            m_Visible = false; // 從畫面上隱藏
        }
    }

    // 覆寫實體狀態
    bool IsActive() const override {
        return m_IsActive;
    }

private:
    bool m_IsActive = true;
};

#endif // BREAKABLE_BLOCK_HPP