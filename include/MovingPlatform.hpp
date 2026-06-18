#ifndef MOVING_PLATFORM_HPP
#define MOVING_PLATFORM_HPP

#include "Block.hpp"

// Platform that moves continuously UP or DOWN (wrapping) or oscillates FLOAT (bouncing).
// One 64×16 sprite matches floating_floor.png exactly — no duplicate tiles.
class MovingPlatform : public Block {
public:
    enum class Direction { UP, DOWN, FLOAT };

    static constexpr float SPEED           = 60.0f;
    static constexpr float FLOAT_SPEED     = 25.0f;
    static constexpr float FLOAT_AMPLITUDE = 32.0f;   // ±2 blocks around placed position
    static constexpr float PLATFORM_WIDTH  = 64.0f;   // matches sprite pixel width
    static constexpr float DEFAULT_MIN_Y   = -240.0f;
    static constexpr float DEFAULT_MAX_Y   =  240.0f;

    explicit MovingPlatform(Direction dir,
                            float minY = DEFAULT_MIN_Y,
                            float maxY = DEFAULT_MAX_Y);

    // Collision: 64×16 AABB centered on the platform
    glm::vec2 GetSize() const override { return { PLATFORM_WIDTH, 16.0f }; }
    glm::vec2 GetCollisionPosition() const override;
    float GetVelocityY() const override { return m_VelocityY; }
    bool IsXCollidable() const override { return false; }

    void SetPosition(const glm::vec2& pos) override;
    void Update(float deltaTime) override;
    void UpdateRenderPosition(float cameraX, float cameraZoom) override;

private:
    Direction m_Direction;
    float     m_VelocityY;
    float     m_MinY, m_MaxY;
};

#endif // MOVING_PLATFORM_HPP
