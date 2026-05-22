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

// 在 MarioState.hpp 或相關檔案中新增
class PoleSlideState : public MarioState {
public:
    PoleSlideState(float poleX) : m_PoleX(poleX) {}

    void Enter(Mario* mario) override {
        // 將瑪利歐對齊旗桿的 X 座標
        auto pos = mario->GetPosition();
        mario->SetPosition({ m_PoleX, pos.y });

        // 停止所有速度
        mario->SetVelocity({ 0.0f, 0.0f });

        // 這裡可以播放抓旗桿的圖片 (假設你有 mario_climb.png)
        // mario->SetImage("mario_climb.png"); 
    }

    void Update(Mario* mario, float deltaTime, float inputDirection, bool isSprinting, bool wantsJump) override {
        // 完全忽略玩家輸入

        if (!m_IsBottomReached) {
            // 階段 1：往下滑
            auto pos = mario->GetPosition();
            pos.y -= m_SlideSpeed * deltaTime;
            mario->SetPosition(pos);

            // 假設 Y 座標小於某個值 (或透過 CollisionManager 判斷著地) 就視為到底
            if (mario->IsGrounded()) {
                m_IsBottomReached = true;
                // 到底後可以換成正常走路圖，準備往右走
                // 瑪利歐通常會先翻到旗桿右邊
                mario->SetPosition({ pos.x + 30.0f, pos.y });
            }
        }
        else {
            // 階段 2：往右走到城堡 (或者走出畫面)
            auto pos = mario->GetPosition();
            pos.x += m_WalkSpeed * deltaTime;
            mario->SetPosition(pos);

            // 更新走路動畫
            mario->UpdateAnimation(deltaTime, 1.0f); // 強制給予向右的方向

            // 階段 3：走了一段距離後，觸發換關標記
            if (pos.x > m_PoleX + 300.0f) { // 假設往右走 300 單位就進城堡
                GameStateManager::GetInstance().SetLevelComplete(true);
            }
        }
    }

private:
    float m_PoleX;
    float m_SlideSpeed = 200.0f;
    float m_WalkSpeed = 150.0f;
    bool m_BottomReached = false;
};

#endif // MARIO_STATE_HPP