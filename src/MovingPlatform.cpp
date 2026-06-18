#include "MovingPlatform.hpp"

MovingPlatform::MovingPlatform(Direction dir, float minY, float maxY)
    : Block(RESOURCE_DIR"/Blocks/floating_floor.png"),
      m_Direction(dir),
      m_VelocityY(dir == Direction::UP  ?  SPEED :
                  dir == Direction::DOWN ? -SPEED :
                                           FLOAT_SPEED),
      m_MinY(minY), m_MaxY(maxY)
{}

glm::vec2 MovingPlatform::GetCollisionPosition() const {
    return { m_WorldPosition.x + PLATFORM_WIDTH / 2.0f, m_WorldPosition.y };
}

void MovingPlatform::SetPosition(const glm::vec2& pos) {
    m_WorldPosition = pos;
    if (m_Direction == Direction::FLOAT) {
        m_MinY = pos.y - FLOAT_AMPLITUDE;
        m_MaxY = pos.y + FLOAT_AMPLITUDE;
    }
    m_Transform.translation = pos;
}

void MovingPlatform::Update(float deltaTime) {
    m_WorldPosition.y += m_VelocityY * deltaTime;

    if (m_Direction == Direction::FLOAT) {
        if (m_VelocityY > 0.0f && m_WorldPosition.y >= m_MaxY) {
            m_WorldPosition.y = m_MaxY;
            m_VelocityY = -FLOAT_SPEED;
        } else if (m_VelocityY < 0.0f && m_WorldPosition.y <= m_MinY) {
            m_WorldPosition.y = m_MinY;
            m_VelocityY = FLOAT_SPEED;
        }
    } else {
        // UP / DOWN: wrap around at bounds
        if (m_VelocityY > 0.0f && m_WorldPosition.y > m_MaxY)
            m_WorldPosition.y = m_MinY;
        else if (m_VelocityY < 0.0f && m_WorldPosition.y < m_MinY)
            m_WorldPosition.y = m_MaxY;
    }
}

void MovingPlatform::UpdateRenderPosition(float cameraX, float cameraZoom) {
    // Shift the sprite center to the middle of the 64px platform so it covers
    // exactly from m_WorldPosition.x to m_WorldPosition.x + 64.
    float centerX = m_WorldPosition.x + PLATFORM_WIDTH / 2.0f;
    m_Transform.translation.x = (centerX - cameraX) * cameraZoom;
    m_Transform.translation.y = m_WorldPosition.y * cameraZoom;
    m_Transform.scale = m_BaseScale * cameraZoom;
}
