#ifndef FIREFLOWER_HPP
#define FIREFLOWER_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"
#include "Mario.hpp"
#include "MarioState.hpp"
#include "SFXManager.hpp"
#include "Util/Animation.hpp"

class FireFlower : public Item {
public:
    explicit FireFlower(glm::vec2 startPos) : Item(RESOURCE_DIR"/Items/fireflower1.png") {
        m_Drawable = std::make_shared<Util::Animation>(
            std::vector<std::string>{
                RESOURCE_DIR"/Items/fireflower1.png",
                RESOURCE_DIR"/Items/fireflower2.png",
                RESOURCE_DIR"/Items/fireflower3.png"
            },
            true, 167, true
        );
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
            SFXManager::GetInstance().Play(SFXManager::Sound::PowerUp);
            if (mario && !dynamic_cast<FireMarioState*>(mario->GetState())) {
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