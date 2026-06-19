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
    virtual std::string GetSlideImage() const { return GetIdleImage(); }

    virtual void Enter(Mario* /*mario*/) {}
};

class SmallMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 15.0f }; }
    bool CanBreakBlocks() const override { return false; }

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario_jump.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario_skid.png"; }
    std::string GetSlideImage() const override { return RESOURCE_DIR"/Entities/LittleMario/slide.png"; }
    std::vector<std::string> GetRunImages() const override {
        return { RESOURCE_DIR"/Entities/LittleMario/mario_run3.png", RESOURCE_DIR"/Entities/LittleMario/mario_run1.png" };
    }
};

class BigMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 31.0f }; }
    bool CanBreakBlocks() const override { return true; }

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario_crouch.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario_jump.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/BigMario/mario_skid.png"; }
    std::string GetSlideImage() const override { return RESOURCE_DIR"/Entities/BigMario/big_slide.png"; }
    std::vector<std::string> GetRunImages() const override {
        return { RESOURCE_DIR"/Entities/BigMario/mario_run3.png", RESOURCE_DIR"/Entities/BigMario/mario_run1.png" };
    }
};

class FireMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 31.0f }; }
    bool CanBreakBlocks() const override { return true; }
    bool CanShoot() const override { return true; }

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_jump.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_skid.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_crouch.png"; }
    std::string GetSlideImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/fire_slide.png"; }
    std::vector<std::string> GetRunImages() const override {
        return { RESOURCE_DIR"/Entities/FireflowerMario/mario_run3.png", RESOURCE_DIR"/Entities/FireflowerMario/mario_run1.png" };
    }
};

class PoleSlideState : public MarioState {
public:
    PoleSlideState(float poleX, bool isBig, bool isFire = false, float stopX = 3188.0f)
        : m_PoleX(poleX), m_IsBig(isBig), m_IsFire(isFire), m_StopX(stopX) {}

    float GetPoleX() const { return m_PoleX; }
    glm::vec2 GetHitboxSize() const override { return m_IsBig ? glm::vec2{ 16.0f, 31.0f } : glm::vec2{ 16.0f, 15.0f }; }
    bool CanBreakBlocks() const override { return false; }
    std::string GetIdleImage() const override { return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/mario_jump.png" : RESOURCE_DIR"/Entities/LittleMario/mario_jump.png"; }
    std::string GetCrouchImage() const override { return GetIdleImage(); }
    std::string GetJumpImage() const override { return GetIdleImage(); }
    std::string GetSkidImage() const override { return GetIdleImage(); }
    std::vector<std::string> GetRunImages() const override {
        if (m_IsBig)
            return { RESOURCE_DIR"/Entities/BigMario/mario_run3.png", RESOURCE_DIR"/Entities/BigMario/mario_run1.png" };
        return { RESOURCE_DIR"/Entities/LittleMario/mario_run3.png", RESOURCE_DIR"/Entities/LittleMario/mario_run1.png" };
    }
    std::string GetSlideImage() const override {
        if (m_IsFire) return RESOURCE_DIR"/Entities/FireflowerMario/fire_slide.png";
        return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/big_slide.png"
                       : RESOURCE_DIR"/Entities/LittleMario/slide.png";
    }

    void Enter(Mario* mario) override;
    float GetSlideSpeed() const { return m_SlideSpeed; }
    float GetWalkSpeed() const { return m_WalkSpeed; }
    bool IsBottomReached() const { return m_IsBottomReached; }
    void SetBottomReached(bool val) { m_IsBottomReached = val; }
    bool IsPauseCompleted() const { return m_PauseCompleted; }
    void TickPause(float dt) {
        m_PauseTimer -= dt;
        if (m_PauseTimer <= 0.0f) m_PauseCompleted = true;
    }
    // world-X at which Mario stops walking, hides (he "enters" the castle), and
    // the level is marked complete. Per-level: e.g. level 1 col 219 = 3188,
    // level2_end col 31 = 180, level 3 col 170 = 2404.
    float GetStopX() const { return m_StopX; }

private:
    float m_PoleX;
    bool m_IsBig;
    bool m_IsFire;
    float m_SlideSpeed = 200.0f;
    float m_WalkSpeed = 150.0f;
    bool m_IsBottomReached = false;
    float m_PauseTimer = 0.8f;
    bool m_PauseCompleted = false;
    float m_StopX = 3188.0f;
};

class PipeSlideState : public MarioState {
public:
    enum class SlideDir { Down, Right, Left };

    // Vertical (Down) pipe — original signature kept for compatibility
    PipeSlideState(bool isBig, float targetY, int targetLevel, float spawnX = -300.0f)
        : m_IsBig(isBig), m_SlideDir(SlideDir::Down), m_Target(targetY)
        , m_TargetLevel(targetLevel), m_SpawnX(spawnX) {}

    // Directional pipe — explicit direction + target on that axis.
    // riseOut: when true, Mario emerges at the destination with a PipeExitState
    // rise (used when the destination spawn sits on top of a vertical pipe);
    // when false he simply spawns standing.
    // hasSpawnY: when true, spawnY pins the destination landing height exactly
    // (skipping ground-snap, which would otherwise latch onto blocks stacked
    // above the landing pipe).
    PipeSlideState(bool isBig, SlideDir dir, float target, int targetLevel, float spawnX = -300.0f, bool riseOut = false,
                   float spawnY = 0.0f, bool hasSpawnY = false)
        : m_IsBig(isBig), m_SlideDir(dir), m_Target(target)
        , m_TargetLevel(targetLevel), m_SpawnX(spawnX), m_RiseOut(riseOut)
        , m_SpawnY(spawnY), m_HasSpawnY(hasSpawnY) {}

    glm::vec2 GetHitboxSize() const override { return m_IsBig ? glm::vec2{ 16.0f, 31.0f } : glm::vec2{ 16.0f, 15.0f }; }
    bool CanBreakBlocks() const override { return false; }

    std::string GetIdleImage() const override { return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/mario.png" : RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetCrouchImage() const override { return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/mario.png" : RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetJumpImage() const override { return GetIdleImage(); }
    std::string GetSkidImage() const override { return GetIdleImage(); }
    std::vector<std::string> GetRunImages() const override { return { GetIdleImage() }; }

    void Enter(Mario* mario) override;

    SlideDir GetSlideDir() const { return m_SlideDir; }
    float GetTarget() const { return m_Target; }
    float GetTargetY() const { return m_Target; }  // kept for compatibility
    float GetSlideSpeed() const { return 50.0f; }
    bool IsDownReached() const { return m_IsDownReached; }
    void SetDownReached(bool val) { m_IsDownReached = val; }
    int GetTargetLevel() const { return m_TargetLevel; }
    float GetSpawnX() const { return m_SpawnX; }
    bool RisesOut() const { return m_RiseOut; }
    float GetSpawnY() const { return m_SpawnY; }
    bool HasExactSpawnY() const { return m_HasSpawnY; }

private:
    bool     m_IsBig;
    SlideDir m_SlideDir;
    float    m_Target;
    int      m_TargetLevel;
    float m_SpawnX;
    bool m_RiseOut = false;
    float m_SpawnY = 0.0f;
    bool m_HasSpawnY = false;
    bool m_IsDownReached = false;
};

class PipeExitState : public MarioState {
public:
    PipeExitState(bool isBig, float targetY)
        : m_IsBig(isBig), m_TargetY(targetY) {}

    glm::vec2 GetHitboxSize() const override { return m_IsBig ? glm::vec2{ 16.0f, 31.0f } : glm::vec2{ 16.0f, 15.0f }; }
    bool CanBreakBlocks() const override { return false; }
    std::string GetIdleImage() const override {
        return m_IsBig ? RESOURCE_DIR"/Entities/BigMario/mario.png"
                       : RESOURCE_DIR"/Entities/LittleMario/mario.png";
    }
    std::string GetCrouchImage() const override { return GetIdleImage(); }
    std::string GetJumpImage() const override { return GetIdleImage(); }
    std::string GetSkidImage() const override { return GetIdleImage(); }
    std::vector<std::string> GetRunImages() const override { return { GetIdleImage() }; }

    void Enter(Mario* mario) override;
    float GetTargetY() const { return m_TargetY; }
    float GetExitSpeed() const { return 50.0f; }
    bool IsExitDone() const { return m_IsExitDone; }
    void SetExitDone(bool val) { m_IsExitDone = val; }

private:
    bool  m_IsBig;
    float m_TargetY;
    bool  m_IsExitDone = false;
};

#endif // MARIO_STATE_HPP