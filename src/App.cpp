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

    // 新的狀態切換方式 (測試大型態破壞方塊用)
    m_Mario->ChangeState(std::make_unique<BigMarioState>());

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
    m_Mario->SetPosition({ -300.0f, 1500.0f });

    for (auto& item : m_Items) { m_Root.RemoveChild(item); }
    m_Items.clear();
    LOG_INFO("已載入關卡: {}", level);
}

void App::Update() {
    // 1. 計算時間差與防禦性限制
    float deltaTime = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    // 2. 擷取輸入
    float inputDirection = 0.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) inputDirection = 1.0f;
    else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) inputDirection = -1.0f;
    bool isSprinting = Util::Input::IsKeyPressed(Util::Keycode::Z);
    bool wantsJump = Util::Input::IsKeyDown(Util::Keycode::SPACE);

    // 3. 實體物理更新 (各自處理與地形的阻擋)
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, m_CurrentMapBlocks);

    for (auto& block : m_CurrentMapBlocks) {
        block->Update(deltaTime);
        if (auto newItem = block->PopSpawnedItem()) {
            m_Items.push_back(newItem);
            m_Root.AddChild(newItem);
        }
    }

    for (auto& item : m_Items) {
        if (item->IsActive()) {
            item->Update(deltaTime, m_CurrentMapBlocks);
        }
    }
    for (auto& enemy : m_Enemies) {
        if (enemy->IsActive()) {
            enemy->UpdateAI(deltaTime, m_CurrentMapBlocks);
        }
    }

    // 呼叫 CollisionManager 時傳入 m_Enemies
    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items, m_Enemies);

    // 4. 處理實體間的互動邏輯 (收集、頂飛)
    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items);

    // 5. 清理已死亡的道具
    for (auto it = m_Items.begin(); it != m_Items.end(); ) {
        if (!(*it)->IsActive()) {
            m_Root.RemoveChild(*it);
            it = m_Items.erase(it);
        }
        else {
            ++it;
        }
    }

    // 6. 更新攝影機與渲染
    UpdateCamera();
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
    float marioWorldX = m_Mario->GetPosition().x;
    float triggerX = 0.0f; // 螢幕中線觸發點

    // 計算攝影機的世界座標 (只允許向右推進，不可後退)
    if (marioWorldX > m_CameraX + triggerX) {
        m_CameraX = marioWorldX - triggerX;
    }

    // 左邊界限制 (基於攝影機當前位置計算絕對物理邊界)
    float leftScreenBoundary = m_CameraX - 400.0f + 25.0f;
    if (m_Mario->GetPosition().x < leftScreenBoundary) {
        m_Mario->SetPosition({ leftScreenBoundary, m_Mario->GetPosition().y });
    }

    // ==========================================
    // 視圖矩陣轉換 (World Space -> Screen Space)
    // ==========================================
    m_Mario->UpdateRenderPosition(m_CameraX);

    for (auto& block : m_CurrentMapBlocks) {
        block->UpdateRenderPosition(m_CameraX);
    }

    for (auto& item : m_Items) {
        item->UpdateRenderPosition(m_CameraX);
    }
}

void App::End() {
    LOG_TRACE("End");
}