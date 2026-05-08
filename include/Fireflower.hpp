#ifndef FIREFLOWER_HPP
#define FIREFLOWER_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"

class FireFlower : public Item {
public:
    explicit FireFlower(glm::vec2 startPos) : Item(RESOURCE_DIR"/Items/fireflower.png") {
        SetPosition(startPos);
        m_SpawnStartY = startPos.y;
        SetZIndex(5);
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>&) override {
        // 火焰花只會緩慢上升，升到頂部後就停在原地
        if (m_State == State::SPAWNING) {
            glm::vec2 pos = GetPosition();
            pos.y += 50.0f * deltaTime;
            SetPosition(pos);

            if (pos.y >= m_SpawnStartY + 16.0f) {
                m_State = State::IDLE;
                SetZIndex(40);
            }
        }
    }

    void OnCollect(Mario* mario) override {
        if (m_IsActive) {
            m_IsActive = false;
            m_Visible = false; // 確保收集後圖片消失
            GameStateManager::GetInstance().AddScore(1000);
            if (mario) {
                // 修正類別名稱為 FireMarioState
                mario->ChangeState(std::make_unique<FireMarioState>());

                // 暫時將加分功能註解掉，等到進入第二階段實作計分系統後再補上
                // mario->AddScore(1000); 
            }
        }
    }

private:
    enum class State { SPAWNING, IDLE };
    State m_State = State::SPAWNING;
    float m_SpawnStartY = 0.0f;
};

#endif // FIREFLOWER_HPP