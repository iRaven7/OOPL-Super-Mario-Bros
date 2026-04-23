#ifndef UNBREAKABLE_BLOCK_HPP
#define UNBREAKABLE_BLOCK_HPP

#include "Block.hpp"

class UnbreakableBlock : public Block {
public:
    explicit UnbreakableBlock(const std::string& imagePath) : Block(imagePath) {}

    // 覆寫 OnHit，使其不產生任何物理反饋與碰撞標記
    void OnHit(Character* hitter) override {
        // 刻意留空，阻斷預設的 m_JustHit = true
    }
};

#endif // UNBREAKABLE_BLOCK_HPP