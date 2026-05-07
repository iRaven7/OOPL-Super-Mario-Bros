#ifndef QUESTION_BLOCK_HPP
#define QUESTION_BLOCK_HPP

#include "Block.hpp"
#include "Item.hpp"
#include "Mushroom.hpp" 
#include "Coin.hpp"
#include "FireFlower.hpp"

class QuestionBlock : public Block {
public:
    enum class ItemType { MUSHROOM, COIN, FIREFLOWER }; // 未來可擴充金幣

    QuestionBlock(const std::string& imagePath, ItemType type) : Block(imagePath), m_ItemType(type) {}

    // 在 QuestionBlock 類別中修改此段：
    glm::vec2 GetCollisionPosition() const override {
        // 修正：必須使用絕對的 m_WorldPosition.x
        return { m_WorldPosition.x, m_OriginalY };
    }

    // 同樣確認 SetPosition 有加上 override
    void SetPosition(const glm::vec2& Position) override {
        Block::SetPosition(Position);
        if (!m_IsBouncing) m_OriginalY = Position.y;
    }

    void OnHit(Character*) override {
        if (m_IsEmpty || m_IsBouncing) return; // 已經空了或正在動畫中，不反應

        m_IsBouncing = true;
        m_BounceVelocity = 250.0f;
        m_IsEmpty = true;
        SetImage(RESOURCE_DIR"/Blocks/empty_question_block.png");

        // 實例化道具
        if (m_ItemType == ItemType::MUSHROOM) {
            m_SpawnedItem = std::make_shared<Mushroom>(GetPosition());
        }
        else if (m_ItemType == ItemType::COIN) {
            m_SpawnedItem = std::make_shared<Coin>(GetPosition());
        }
        else if (m_ItemType == ItemType::FIREFLOWER) {
            m_SpawnedItem = std::make_shared<FireFlower>(GetPosition());
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
        m_SpawnedItem = nullptr; // 提取後清空，確保只生成一次
        return item;
    }

private:
    bool m_IsEmpty = false;
    bool m_IsBouncing = false;
    float m_OriginalY = 0.0f;
    float m_BounceVelocity = 0.0f;
    ItemType m_ItemType;
    std::shared_ptr<Item> m_SpawnedItem = nullptr;
};

#endif