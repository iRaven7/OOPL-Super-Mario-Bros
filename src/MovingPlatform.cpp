#include "MovingPlatform.hpp"

MovingPlatform::MovingPlatform(Direction dir, float minY, float maxY)
    : Block(RESOURCE_DIR"/Blocks/floating_floor.png"),
      m_VelocityY(dir == Direction::UP ? SPEED : -SPEED),
      m_MinY(minY), m_MaxY(maxY)
{
    for (int i = 0; i < TILE_COUNT; ++i) {
        auto tile = std::make_shared<VisualTile>(RESOURCE_DIR"/Blocks/floating_floor.png");
        tile->SetZIndex(10);
        m_VisualTiles.push_back(std::move(tile));
    }
}

glm::vec2 MovingPlatform::GetCollisionPosition() const {
    // m_WorldPosition is the center of tile 0 (leftmost).
    // Center of the full 7-tile span = tile0_center + (7-1)/2 * 16 = x + 48.
    return { m_WorldPosition.x + (TILE_COUNT - 1) * 8.0f, m_WorldPosition.y };
}

void MovingPlatform::SetPosition(const glm::vec2& pos) {
    m_WorldPosition = pos;
    m_Transform.translation = pos;
    SyncVisualTiles();
}

void MovingPlatform::Update(float deltaTime) {
    m_WorldPosition.y += m_VelocityY * deltaTime;

    if (m_VelocityY > 0.0f && m_WorldPosition.y > m_MaxY)
        m_WorldPosition.y = m_MinY;
    else if (m_VelocityY < 0.0f && m_WorldPosition.y < m_MinY)
        m_WorldPosition.y = m_MaxY;

    SyncVisualTiles();
}

void MovingPlatform::SyncVisualTiles() {
    for (int i = 0; i < TILE_COUNT; ++i) {
        m_VisualTiles[i]->SetPosition({ m_WorldPosition.x + i * 16.0f, m_WorldPosition.y });
    }
}
