#ifndef FIREFLOWER_HPP
#define FIREFLOWER_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"
#include "Mario.hpp"      // 補上這個
#include "MarioState.hpp" // 補上這個

class FireFlower : public Item {
public:
    explicit FireFlower(glm::vec2 startPos) : Item(RESOURCE_DIR"/Items/fireflower.png") {
        SetPosition(startPos);
        m_SpawnStartY = startPos.y;
        SetZIndex(5);
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>&) override {
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
            m_Visible = false;
            GameStateManager::GetInstance().AddScore(1000);
            if (mario) {
                mario->ChangeState(std::make_unique<FireMarioState>());
            }
        }
    }

private:
    enum class State { SPAWNING, IDLE };
    State m_State = State::SPAWNING;
    float m_SpawnStartY = 0.0f;
};

#endif // FIREFLOWER_HPP