#ifndef ENTERABLE_PIPE_HPP
#define ENTERABLE_PIPE_HPP

#include "UnbreakableBlock.hpp"

class EnterablePipe : public UnbreakableBlock {
public:
    EnterablePipe(const std::string& imagePath, int targetLevel,
                  float spawnX = -300.0f,
                  PipeEntryDir dir = PipeEntryDir::Down)
        : UnbreakableBlock(imagePath)
        , m_TargetLevel(targetLevel)
        , m_SpawnX(spawnX)
        , m_EntryDir(dir) {}

    bool IsPipeEntrance() const override { return true; }
    int GetTargetLevel() const override { return m_TargetLevel; }
    float GetSpawnX() const override { return m_SpawnX; }
    PipeEntryDir GetPipeEntryDir() const override { return m_EntryDir; }

private:
    int          m_TargetLevel;
    float        m_SpawnX;
    PipeEntryDir m_EntryDir;
};

#endif // ENTERABLE_PIPE_HPP
