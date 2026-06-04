#ifndef FLAG_HPP
#define FLAG_HPP

#include "Item.hpp"
#include "Mario.hpp"
#include "MarioState.hpp"
#include "GameStateManager.hpp"

class Flag : public Item {
public:
    Flag(glm::vec2 bottomPos) : Item(RESOURCE_DIR"/Blocks/flag.png") {
        m_PoleX = bottomPos.x;
        m_BottomY = bottomPos.y;
        m_FlagY = bottomPos.y + 9 * 32.0f; // 假設旗桿有 9 格高

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
            // 觸發瑪利歐滑旗桿，並給予分數！
            bool isBig = mario->GetSize().y > 16.0f;
            mario->ChangeState(std::make_unique<PoleSlideState>(m_PoleX, isBig), false);
            GameStateManager::GetInstance().AddScore(5000);
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
};

#endif // FLAG_HPP