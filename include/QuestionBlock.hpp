#ifndef QUESTION_BLOCK_HPP
#define QUESTION_BLOCK_HPP

#include "Block.hpp"
#include "Item.hpp"
#include "Mushroom.hpp"
#include "Coin.hpp"
#include "FireFlower.hpp"
#include "OneUp.hpp"
#include "SuperStar.hpp"
#include "SuperFlower.hpp"
#include "Mario.hpp"
#include "MarioState.hpp"

class QuestionBlock : public Block {
public:
    // count controls how many items emerge (one per hit).
    QuestionBlock(const std::string& imagePath, ContentType type, int count = 1)
        : Block(imagePath)
    {
        SetContents(type, count);
    }

    glm::vec2 GetCollisionPosition() const override {
        return { m_WorldPosition.x, m_OriginalY };
    }

    void SetPosition(const glm::vec2& Position) override {
        Block::SetPosition(Position);
        if (!m_IsBouncing) m_OriginalY = Position.y;
    }

    void OnHit(Character* hitter) override {
        if (!HasContents() || m_IsBouncing) return;

        m_IsBouncing = true;
        m_BounceVelocity = 250.0f;
        GameStateManager::GetInstance().AddScore(100);   // item/coin block hit

        // Consume one item from the block's contents.
        m_ContentCount--;

        if (!HasContents())
            SetImage(RESOURCE_DIR"/Blocks/empty_question_block.png");

        switch (GetContentType()) {
        case ContentType::MUSHROOM:
            m_SpawnedItem = std::make_shared<Mushroom>(GetPosition());
            break;
        case ContentType::COIN:
            m_SpawnedItem = std::make_shared<Coin>(GetPosition());
            break;
        case ContentType::FIREFLOWER: {
            Mario* mario = dynamic_cast<Mario*>(hitter);
            bool isSmall = mario && dynamic_cast<SmallMarioState*>(mario->GetState());
            m_SpawnedItem = isSmall
                ? std::static_pointer_cast<Item>(std::make_shared<Mushroom>(GetPosition()))
                : std::static_pointer_cast<Item>(std::make_shared<FireFlower>(GetPosition()));
            break;
        }
        case ContentType::ONEUP:
            m_SpawnedItem = std::make_shared<OneUp>(GetPosition());
            break;
        case ContentType::STAR:
            m_SpawnedItem = std::make_shared<SuperStar>(GetPosition());
            break;
        case ContentType::SUPERFLOWER:
            m_SpawnedItem = std::make_shared<SuperFlower>(GetPosition());
            break;
        default: break;
        }
    }

    void Update(float deltaTime) override {
        if (!m_IsBouncing) return;

        m_BounceVelocity -= 1500.0f * deltaTime;
        glm::vec2 currentPos = GetPosition();
        currentPos.y += m_BounceVelocity * deltaTime;

        if (currentPos.y <= m_OriginalY) {
            currentPos.y = m_OriginalY;
            m_IsBouncing = false;
            m_BounceVelocity = 0.0f;
        }
        Block::SetPosition(currentPos);
    }

    std::shared_ptr<Item> PopSpawnedItem() override {
        auto item = m_SpawnedItem;
        m_SpawnedItem = nullptr;
        return item;
    }

private:
    bool  m_IsBouncing    = false;
    float m_OriginalY     = 0.0f;
    float m_BounceVelocity = 0.0f;
    std::shared_ptr<Item> m_SpawnedItem = nullptr;
};

#endif
