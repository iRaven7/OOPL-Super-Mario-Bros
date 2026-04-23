#ifndef MARIO_STATE_HPP
#define MARIO_STATE_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>

class MarioState {
public:
    virtual ~MarioState() = default;

    virtual glm::vec2 GetHitboxSize() const = 0;
    virtual bool CanBreakBlocks() const = 0;
    virtual bool CanShoot() const { return false; }

    // 定義該型態的各種動畫圖片路徑
    virtual std::string GetIdleImage() const = 0;
    virtual std::string GetJumpImage() const = 0;
    virtual std::string GetSkidImage() const = 0; // 煞車圖片
    virtual std::string GetCrouchImage() const = 0;
    virtual std::vector<std::string> GetRunImages() const = 0; // 跑步幀集合
};

// 小型態實作
class SmallMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 16.0f }; }
    bool CanBreakBlocks() const override { return false; }

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario_jump.png"; } // 替換為實際跳躍圖
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/LittleMario/mario_skid.png"; } // 替換為實際煞車圖
    std::vector<std::string> GetRunImages() const override {
        // 替換為實際的跑步 1, 2, 3 幀
        return { RESOURCE_DIR"/Entities/LittleMario/mario_run3.png", RESOURCE_DIR"/Entities/LittleMario/mario_run1.png" };
    }
};

// 大型態實作
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

// 火力態實作
class FireMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override { return { 16.0f, 32.0f }; }
    bool CanBreakBlocks() const override { return true; }
    bool CanShoot() const override { return true; } // 允許發射火球

    std::string GetIdleImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario.png"; }
    std::string GetJumpImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_jump.png"; }
    std::string GetSkidImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_skid.png"; }
    std::string GetCrouchImage() const override { return RESOURCE_DIR"/Entities/FireflowerMario/mario_crouch.png"; }
    std::vector<std::string> GetRunImages() const override {
        return { RESOURCE_DIR"/Entities/FireflowerMario/mario_run3.png", RESOURCE_DIR"/Entities/FireflowerMario/mario_run1.png" };
    }
};

#endif // MARIO_STATE_HPP