#ifndef FLAG_HPP
#define FLAG_HPP

#include "Item.hpp"
#include "Mario.hpp"
#include "MarioState.hpp"
#include "GameStateManager.hpp"
#include "SFXManager.hpp"
#include "BGMManager.hpp"
#include <algorithm>

class Flag : public Item {
public:
    Flag(glm::vec2 bottomPos, float stopX = 3188.0f) : Item(RESOURCE_DIR"/Blocks/flag.png") {
        m_PoleX = bottomPos.x;
        m_BottomY = bottomPos.y;
        m_StopX = stopX;
        m_FlagY = bottomPos.y + 9 * 16.0f; // 9-segment pole, top at +144

        SetPosition({ m_PoleX - 16.0f, m_FlagY});
        SetZIndex(4);
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>&) override {
        // 降旗動畫
        if (m_IsTriggered && m_FlagY > m_BottomY + 16.0f) {
            m_FlagY -= 200.0f * deltaTime;
        }
    }

    void OnCollect(Mario* mario) override {
        if (!m_IsTriggered) {
            m_IsTriggered = true;
            bool isBig  = mario->GetSize().y > 16.0f;
            bool isFire = dynamic_cast<FireMarioState*>(mario->GetState()) != nullptr;
            mario->ChangeState(std::make_unique<PoleSlideState>(m_PoleX, isBig, isFire, m_StopX), false);
            // Reaching the flag ends the stage: cut the level theme now; the
            // course-clear fanfare follows once Mario steps off the pole.
            BGMManager::GetInstance().Stop();
            SFXManager::GetInstance().Play(SFXManager::Sound::Flagpole);

            // Flagpole score scales with grab height: 5000 for the apex (top
            // tile), otherwise linear from 100 at the base up to 2000 just below
            // the apex.
            float poleTop = m_BottomY + 9 * 16.0f;
            float apexY   = poleTop - 16.0f;
            float grabY   = std::clamp(mario->GetPosition().y, m_BottomY, poleTop);
            int score;
            if (grabY >= apexY) {
                score = 5000;
            } else {
                float frac = (grabY - m_BottomY) / (apexY - m_BottomY);
                score = std::max(100, static_cast<int>(frac * 2000.0f));
            }
            GameStateManager::GetInstance().AddScore(score);
        }
    }

    // 🌟 覆寫碰撞框大小：變成一座看不到的高牆
    glm::vec2 GetSize() const override {
        return { 16.0f, 600.0f };
    }

    // 🌟 覆寫碰撞中心點：固定在旗桿中間，不受圖片往下掉的影響
    glm::vec2 GetPosition() const override {
        return { m_PoleX, m_BottomY + 300.0f };
    }

    // 🌟 覆寫渲染：讓旗子圖片獨立往下掉
    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        m_Transform.translation.x = (m_PoleX - 8.0f - cameraX) * cameraZoom;
        m_Transform.translation.y = (m_FlagY - 16.0f) * cameraZoom;
        m_Transform.scale.x = -m_BaseScale.x * cameraZoom;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

private:
    bool m_IsTriggered = false;
    float m_PoleX;
    float m_BottomY;
    float m_FlagY;
    float m_StopX = 3188.0f;
};

#endif // FLAG_HPP