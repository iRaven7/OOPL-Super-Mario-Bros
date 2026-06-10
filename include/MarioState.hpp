#ifndef MARIO_STATE_HPP
#define MARIO_STATE_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>

class Mario;

class MarioState {
public:
    virtual ~MarioState() = default;

    virtual glm::vec2 GetHitboxSize() const = 0;
    virtual bool CanBreakBlocks() const = 0;
    virtual bool CanShoot() const { return false; }

    virtual std::string GetIdleImage() const = 0;
    virtual std::string GetJumpImage() const = 0;
    virtual std::string GetSkidImage() const = 0;
    virtual std::string GetCrouchImage() const = 0;
    virtual std::vector<std::string> GetRunImages() const = 0;

    virtual void Enter(Mario* /*mario*/) {}
};

class SmallMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 16.0f }; }
    bool CanBreakBlocks() const override { return false; }

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario_jump.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario_skid.png"; }
    std::vector<std::string> GetRunImages() const override {
        return { RESOURCE_DIR"/Entities/LittleMario/mario_run3.png", RESOURCE_DIR"/Entities/LittleMario/mario_run1.png" };
    }
};

class BigMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 32.0f }; }
    bool CanBreakBlocks() const override { return true; }

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario_crouch.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario_jump.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario_skid.png"; }
    std::vector<std::string> GetRunImages() const override {
        return { RESOURCE_DIR"/Entities/BigMario/mario_run3.png", RESOURCE_DIR"/Entities/BigMario/mario_run1.png" };
    }
};

class FireMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 32.0f }; }
    bool CanBreakBlocks() const override { return true; }
    bool CanShoot() const override { return true; }

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_jump.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_skid.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_crouch.png"; }
    std::vector<std::string> GetRunImages() const override {
        return { RESOURCE_DIR"/Entities/FireflowerMario/mario_run3.png", RESOURCE_DIR"/Entities/FireflowerMario/mario_run1.png" };
    }
};

class PoleSlideState : public MarioState {
public:
    PoleSlideState(float poleX, bool isBig) : m_PoleX(poleX), m_IsBig(isBig) {}

    float GetPoleX() const { return m_PoleX; }
    glm::vec2 GetHitboxSize() const override { return m_IsBig ? glm::vec2{ 16.0f, 32.0f } : glm::vec2{ 16.0f, 16.0f }; }
    bool CanBreakBlocks() const override { return false; }
    std::string GetIdleImage() const override { return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/mario.png" : RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::vector<std::string> GetRunImages() const override { return {}; }

    void Enter(Mario* mario) override;
    float GetSlideSpeed() const { return m_SlideSpeed; }
    float GetWalkSpeed() const { return m_WalkSpeed; }
    bool IsBottomReached() const { return m_IsBottomReached; }
    void SetBottomReached(bool val) { m_IsBottomReached = val; }

private:
    float m_PoleX;
    bool m_IsBig;
    float m_SlideSpeed = 200.0f;
    float m_WalkSpeed = 150.0f;
    bool m_IsBottomReached = false;
};

class PipeSlideState : public MarioState {
public:
    PipeSlideState(bool isBig, float targetY, int targetLevel)
        : m_IsBig(isBig), m_TargetY(targetY), m_TargetLevel(targetLevel) {
    }

    glm::vec2 GetHitboxSize() const override { return m_IsBig ? glm::vec2{ 16.0f, 32.0f } : glm::vec2{ 16.0f, 16.0f }; }
    bool CanBreakBlocks() const override { return false; }

    std::string GetIdleImage() const override { return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/mario.png" : RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetCrouchImage() const override { return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/mario.png" : RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetJumpImage() const override { return GetIdleImage(); }
    std::string GetSkidImage() const override { return GetIdleImage(); }
    std::vector<std::string> GetRunImages() const override { return { GetIdleImage() }; }

    void Enter(Mario* mario) override;

    float GetTargetY() const { return m_TargetY; }
    float GetSlideSpeed() const { return 50.0f; }
    bool IsDownReached() const { return m_IsDownReached; }
    void SetDownReached(bool val) { m_IsDownReached = val; }
    int GetTargetLevel() const { return m_TargetLevel; }

private:
    bool m_IsBig;
    float m_TargetY;
    int m_TargetLevel;
    bool m_IsDownReached = false;
};

#endif // MARIO_STATE_HPP