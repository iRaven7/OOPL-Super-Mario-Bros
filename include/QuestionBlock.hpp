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

class QuestionBlock : public Block {
public:
    enum class ItemType { MUSHROOM, COIN, FIREFLOWER, ONEUP, STAR, SUPERFLOWER };

    QuestionBlock(const std::string& imagePath, ItemType type) : Block(imagePath), m_ItemType(type) {}

    // �b QuestionBlock ���O���ק惡�q�G
    glm::vec2 GetCollisionPosition() const override {
        // �ץ��G�����ϥε��諸 m_WorldPosition.x
        return { m_WorldPosition.x, m_OriginalY };
    }

    // �P�˽T�{ SetPosition ���[�W override
    void SetPosition(const glm::vec2& Position) override {
        Block::SetPosition(Position);
        if (!m_IsBouncing) m_OriginalY = Position.y;
    }

    void OnHit(Character* hitter) override {
        if (m_IsEmpty || m_IsBouncing) return;

        m_IsBouncing = true;
        m_BounceVelocity = 250.0f;
        m_IsEmpty = true;
        SetImage(RESOURCE_DIR"/Blocks/empty_question_block.png");

        if (m_ItemType == ItemType::MUSHROOM) {
            m_SpawnedItem = std::make_shared<Mushroom>(GetPosition());
        }
        else if (m_ItemType == ItemType::COIN) {
            m_SpawnedItem = std::make_shared<Coin>(GetPosition());
        }
        else if (m_ItemType == ItemType::FIREFLOWER) {
            Mario* mario = dynamic_cast<Mario*>(hitter);
            bool isSmall = mario && dynamic_cast<SmallMarioState*>(mario->GetState());
            m_SpawnedItem = isSmall
                ? std::static_pointer_cast<Item>(std::make_shared<Mushroom>(GetPosition()))
                : std::static_pointer_cast<Item>(std::make_shared<FireFlower>(GetPosition()));
        }
        else if (m_ItemType == ItemType::ONEUP) {
            m_SpawnedItem = std::make_shared<OneUp>(GetPosition());
        }
        else if (m_ItemType == ItemType::STAR) {
            m_SpawnedItem = std::make_shared<SuperStar>(GetPosition());
        }
        else if (m_ItemType == ItemType::SUPERFLOWER) {
            m_SpawnedItem = std::make_shared<SuperFlower>(GetPosition());
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
        m_SpawnedItem = nullptr; // ������M�šA�T�O�u�ͦ��@��
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