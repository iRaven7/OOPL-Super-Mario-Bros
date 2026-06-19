#ifndef UNBREAKABLE_BLOCK_HPP
#define UNBREAKABLE_BLOCK_HPP

#include "Block.hpp"
#include "SFXManager.hpp"

class UnbreakableBlock : public Block {
public:
    explicit UnbreakableBlock(const std::string& imagePath) : Block(imagePath) {}

    // �мg OnHit�A�Ϩ䤣���ͥ��󪫲z���X�P�I���аO
    void OnHit(Character* hitter) override {
        // No item interaction — just the solid "bump" feedback.
        SFXManager::GetInstance().Play(SFXManager::Sound::Bump);
    }
};

#endif // UNBREAKABLE_BLOCK_HPP