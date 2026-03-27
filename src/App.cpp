#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

void App::Start() {
    LOG_TRACE("Start");

    m_Mario = std::make_shared<Mario>();
    m_Root.AddChild(m_Mario);

    LoadLevel(0);

    m_Mario->SetMarioState(MarioState::BIG);
    m_CurrentState = State::UPDATE;
}

void App::LoadLevel(int level) {
    m_CurrentLevel = level;

    for (auto& block : m_CurrentMapBlocks) {
        m_Root.RemoveChild(block);
    }
    m_CurrentMapBlocks.clear();

    std::string mapPath;
    if (level == 0) {
        mapPath = RESOURCE_DIR"/Map/test_place.txt";
    }
    else {
        mapPath = RESOURCE_DIR"/Map/level" + std::to_string(level) + ".txt";
    }

    m_CurrentMapBlocks = m_MapManager.LoadMap(mapPath);
    for (auto& block : m_CurrentMapBlocks) {
        m_Root.AddChild(block);
    }

    // 重設攝影機與角色起始位置
    m_CameraX = 0.0f;
    m_Mario->SetPosition({ -300.0f, 10000.0f });

    LOG_INFO("已載入關卡: {}", level);
}

void App::Update() {
    // 1. 計算時間差
    float deltaTime = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;

    // 2. 擷取輸入
    float inputDirection = 0.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {
        inputDirection = 1.0f;
    }
    else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) {
        inputDirection = -1.0f;
    }

    bool isSprinting = Util::Input::IsKeyPressed(Util::Keycode::Z);
    bool wantsJump = Util::Input::IsKeyPressed(Util::Keycode::SPACE);

    // ==========================================
    // 3. 更新物理系統 (修改這行！)
    // 傳入 m_CurrentMapBlocks 讓瑪利歐進行 AABB 碰撞偵測
    // ==========================================
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, m_CurrentMapBlocks);

    // 4. 更新攝影機與地圖位置
    UpdateCamera();

    for (auto& block : m_CurrentMapBlocks) {
        block->Update(deltaTime);
    }

    // 5. 渲染
    m_Root.Update();

    // 除錯日誌
    LOG_DEBUG("Mario X: {:.2f}, Y: {:.2f}, IsGrounded: {}",
        m_Mario->GetPosition().x, m_Mario->GetPosition().y, m_Mario->IsGrounded());

    // 關卡控制與程式退出
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_0)) LoadLevel(0);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_1)) LoadLevel(1);

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::UpdateCamera() {
    float marioX = m_Mario->GetPosition().x;
    float triggerX = 0.0f; // 螢幕中線觸發點

    if (marioX > triggerX) {
        float deltaX = marioX - triggerX;
        m_CameraX += deltaX;

        m_Mario->SetPosition({ triggerX, m_Mario->GetPosition().y });

        for (auto& block : m_CurrentMapBlocks) {
            glm::vec2 pos = block->GetPosition();
            pos.x -= deltaX;
            block->SetPosition(pos);
        }
    }

    // 左邊界限制
    float leftScreenBoundary = -400.0f + 25.0f;
    if (m_Mario->GetPosition().x < leftScreenBoundary) {
        m_Mario->SetPosition({ leftScreenBoundary, m_Mario->GetPosition().y });
    }
}

void App::End() {
    LOG_TRACE("End");
}