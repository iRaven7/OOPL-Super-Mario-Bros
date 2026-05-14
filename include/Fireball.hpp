#ifndef FIREBALL_HPP
#define FIREBALL_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Block.hpp"
#include <vector>
#include <memory>
#include <cmath>

class Fireball : public Util::GameObject {
public:
    Fireball(glm::vec2 startPos, float direction) {
        m_ImagePath = RESOURCE_DIR"/Entities/FireflowerMario/fire.png";
        m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
        SetPosition(startPos);
        SetZIndex(45);
        m_Velocity.x = direction * 500.0f; // 依面向決定射擊方向
        m_Velocity.y = -100.0f;            // 初始稍微給予向下的速度
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) {
        if (!m_IsActive) return;

        m_Velocity.y += GRAVITY * deltaTime;
        glm::vec2 currentPos = GetPosition();
        glm::vec2 mySize = GetSize();

        // X 軸移動與撞牆判定
        currentPos.x += m_Velocity.x * deltaTime;
        for (const auto& block : blocks) {
            if (!block->IsActive()) continue;
            if (CheckAABB(currentPos, mySize, block->GetCollisionPosition(), block->GetSize())) {
                Destroy(); // 撞牆直接消失
                return;
            }
        }

        // Y 軸移動與反彈判定
        currentPos.y += m_Velocity.y * deltaTime;
        for (const auto& block : blocks) {
            if (!block->IsActive()) continue;
            if (CheckAABB(currentPos, mySize, block->GetCollisionPosition(), block->GetSize())) {
                if (m_Velocity.y < 0.0f) {
                    // 落地向上反彈
                    currentPos.y = block->GetCollisionPosition().y + (block->GetSize().y / 2.0f) + (mySize.y / 2.0f);
                    m_Velocity.y = 400.0f;
                }
                else if (m_Velocity.y > 0.0f) {
                    // 撞到天花板反彈向下
                    currentPos.y = block->GetCollisionPosition().y - (block->GetSize().y / 2.0f) - (mySize.y / 2.0f);
                    m_Velocity.y = -100.0f;
                }
            }
        }
        SetPosition(currentPos);
    }



    void SetActive(bool active) { m_IsActive = active; }
    bool IsActive() const { return m_IsActive; }
    void Destroy() { m_IsActive = false; m_Visible = false; }
    glm::vec2 GetPosition() const { return m_WorldPosition; }
    glm::vec2 GetSize() const { return { 16.0f, 16.0f }; }

    void SetPosition(const glm::vec2& Position) {
        m_WorldPosition = Position;
        m_Transform.translation = Position;
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) {
        // 保留原有的火球旋轉邏輯
        m_Transform.rotation += 10.0f * (m_Velocity.x > 0 ? -deltaTime() : deltaTime());

        // 套用縮放與座標轉換公式
        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = m_WorldPosition.y * cameraZoom;
        m_Transform.scale = m_BaseScale * cameraZoom;
    }

private:
    float deltaTime() const { return 0.016f; } // 簡化旋轉速度參數

    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
        return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
            std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
    }

    std::string m_ImagePath;
    glm::vec2 m_WorldPosition = { 0.0f, 0.0f };
    glm::vec2 m_BaseScale = { 1.0f, 1.0f };
    glm::vec2 m_Velocity = { 0.0f, 0.0f };
    bool m_IsActive = true;
    static constexpr float GRAVITY = -1500.0f;
};

#endif // FIREBALL_HPP