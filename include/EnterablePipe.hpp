#ifndef ENTERABLE_PIPE_HPP
#define ENTERABLE_PIPE_HPP

#include "UnbreakableBlock.hpp"

class EnterablePipe : public UnbreakableBlock {
public:
    EnterablePipe(const std::string& imagePath, int targetLevel,
                  float spawnX = -300.0f,
                  PipeEntryDir dir = PipeEntryDir::Down,
                  bool flipX = false)
        : UnbreakableBlock(imagePath)
        , m_TargetLevel(targetLevel)
        , m_SpawnX(spawnX)
        , m_EntryDir(dir)
        , m_FlipX(flipX) {}

    bool IsPipeEntrance() const override { return true; }
    int GetTargetLevel() const override { return m_TargetLevel; }
    float GetSpawnX() const override { return m_SpawnX; }
    PipeEntryDir GetPipeEntryDir() const override { return m_EntryDir; }

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = m_WorldPosition.y * cameraZoom;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * (m_FlipX ? -1.0f : 1.0f);
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

private:
    int          m_TargetLevel;
    float        m_SpawnX;
    PipeEntryDir m_EntryDir;
    bool         m_FlipX;
};

#endif // ENTERABLE_PIPE_HPP
