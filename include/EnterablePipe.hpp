#ifndef ENTERABLE_PIPE_HPP
#define ENTERABLE_PIPE_HPP

#include "UnbreakableBlock.hpp"

class EnterablePipe : public UnbreakableBlock {
public:
    EnterablePipe(const std::string& imagePath, int targetLevel)
        : UnbreakableBlock(imagePath), m_TargetLevel(targetLevel) {
    }

    bool IsPipeEntrance() const override { return true; }
    int GetTargetLevel() const { return m_TargetLevel; }

private:
    int m_TargetLevel;
};

#endif // ENTERABLE_PIPE_HPP