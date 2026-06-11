#ifndef MOVING_PLATFORM_HPP
#define MOVING_PLATFORM_HPP

#include "Block.hpp"
#include <vector>
#include <memory>

// Non-collidable tile used purely for rendering the platform visuals.
class VisualTile : public Block {
public:
    explicit VisualTile(const std::string& imagePath) : Block(imagePath) {}
    bool IsCollidable() const override { return false; }
};

// A 7-tile-wide platform that moves continuously up or down and wraps at bounds.
// Collision is one unified AABB (112 x 16). Rendering is 7 separate VisualTile objects.
class MovingPlatform : public Block {
public:
    enum class Direction { UP, DOWN };

    static constexpr int   TILE_COUNT   = 7;
    static constexpr float SPEED        = 60.0f;
    static constexpr float DEFAULT_MIN_Y = -240.0f;
    static constexpr float DEFAULT_MAX_Y =  240.0f;

    explicit MovingPlatform(Direction dir,
                            float minY = DEFAULT_MIN_Y,
                            float maxY = DEFAULT_MAX_Y);

    // --- collision (one big box) ---
    glm::vec2 GetSize() const override { return { TILE_COUNT * 16.0f, 16.0f }; }
    glm::vec2 GetCollisionPosition() const override;
    float GetVelocityY() const override { return m_VelocityY; }
    bool IsXCollidable() const override { return false; }

    // --- lifecycle ---
    void SetPosition(const glm::vec2& pos) override;
    void Update(float deltaTime) override;

    // The platform's own GameObject is invisible; it moves off-screen.
    void UpdateRenderPosition(float /*cameraX*/, float /*cameraZoom*/) override {
        m_Transform.translation = { -99999.0f, -99999.0f };
    }

    // App/MapManager should also push these into m_CurrentMapBlocks for rendering.
    const std::vector<std::shared_ptr<VisualTile>>& GetVisualTiles() const {
        return m_VisualTiles;
    }

private:
    float m_VelocityY;
    float m_MinY, m_MaxY;
    std::vector<std::shared_ptr<VisualTile>> m_VisualTiles;

    void SyncVisualTiles();
};

#endif // MOVING_PLATFORM_HPP
