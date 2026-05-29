#ifndef MARIO_STATE_HPP
#define MARIO_STATE_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>

// 絕對不能漏掉這行前置宣告！
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

    // 加上這兩個虛擬函式，並把變數名稱註解掉來消除警告
    virtual void Enter(Mario* /*mario*/) {}
    virtual void Update(Mario* /*mario*/, float /*deltaTime*/, float /*inputDirection*/, bool /*isSprinting*/, bool /*wantsJump*/) {}
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
    PoleSlideState(float poleX) : m_PoleX(poleX) {}

    glm::vec2 GetHitboxSize() const override { return { 16.0f, 16.0f }; }
    bool CanBreakBlocks() const override { return false; }
    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::vector<std::string> GetRunImages() const override { return {}; }

    // 這裡「只留宣告」，絕對不能把實作寫在這裡！
    void Enter(Mario* mario) override;
    void Update(Mario* mario, float deltaTime, float inputDirection, bool isSprinting, bool wantsJump) override;

private:
    float m_PoleX;
    float m_SlideSpeed = 200.0f;
    float m_WalkSpeed = 150.0f;
    bool m_IsBottomReached = false;
};

#endif // MARIO_STATE_HPP