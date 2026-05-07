#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

void App::Start() {
    LOG_TRACE("Start");

    m_CameraZoom = 1.8f;
    m_Mario = std::make_shared<Mario>();
    m_Root.AddChild(m_Mario);

    LoadLevel(0);

    // 新的狀態切換方式 (測試大型態破壞方塊用)
    m_Mario->ChangeState(std::make_unique<BigMarioState>());

    m_CurrentState = State::UPDATE;
}

void App::LoadLevel(int level) {
    m_CurrentLevel = level;

    // 清理舊有關卡的實體
    for (auto& block : m_CurrentMapBlocks) { m_Root.RemoveChild(block); }
    m_CurrentMapBlocks.clear();

    for (auto& enemy : m_Enemies) { m_Root.RemoveChild(enemy); }
    m_Enemies.clear();

    for (auto& item : m_Items) { m_Root.RemoveChild(item); }
    m_Items.clear();

    for (auto& fb : m_Fireballs) { m_Root.RemoveChild(fb); }
    m_Fireballs.clear();

    std::string mapPath;
    if (level == 0) {
        mapPath = RESOURCE_DIR"/Map/test_place.txt";
    }
    else {
        mapPath = RESOURCE_DIR"/Map/level" + std::to_string(level) + ".txt";
    }

    // 呼叫更新後的 LoadMap 介面
    m_MapManager.LoadMap(mapPath, m_CurrentMapBlocks, m_Enemies);

    // 將解析出的實體註冊至算繪樹
    for (auto& block : m_CurrentMapBlocks) { m_Root.AddChild(block); }
    for (auto& enemy : m_Enemies) { m_Root.AddChild(enemy); }

    m_CameraX = 0.0f;
    m_Mario->SetPosition({ -300.0f, 1500.0f });

    LOG_INFO("已載入關卡: {}", level);
}

void App::Update() {
    // 1. 計算時間差與防禦性限制
    float deltaTime = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    if (m_Mario->IsTransforming()) {
        m_Mario->UpdateTransformation(deltaTime);
        m_Root.Update(); // 僅渲染畫面，略過其餘物理與邏輯更新
        return;
    }


    // 2. 擷取輸入
    float inputDirection = 0.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) inputDirection = 1.0f;
    else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) inputDirection = -1.0f;
    if (m_Mario->IsCrouching() && m_Mario->IsGrounded()) {
        inputDirection = 0.0f;
    }
    bool isSprinting = Util::Input::IsKeyPressed(Util::Keycode::Z);
    bool wantsJump = Util::Input::IsKeyDown(Util::Keycode::SPACE);
    bool wantsCrouch = Util::Input::IsKeyPressed(Util::Keycode::DOWN);
    bool wantsFire = Util::Input::IsKeyDown(Util::Keycode::X);

    // 3. 實體物理更新 (各自處理與地形的阻擋)

    m_Mario->Update(deltaTime);
    m_Mario->UpdateAnimation(deltaTime, inputDirection);
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, m_CurrentMapBlocks);
    m_Mario->SetCrouching(wantsCrouch);

    auto newFireballs = m_Mario->PopSpawnedFireballs();
    for (auto& fb : newFireballs) {
        m_Fireballs.push_back(fb);
        m_Root.AddChild(fb);
    }

    // 更新火球物理
    for (auto& fb : m_Fireballs) {
        if (fb->IsActive()) fb->Update(deltaTime, m_CurrentMapBlocks);
    }

    // 將火球傳入 CollisionManager (注意：這裡等一下會修改 CollisionManager 介面)
    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items, m_Enemies, m_Fireballs);

    // 清理已撞毀的火球
    for (auto it = m_Fireballs.begin(); it != m_Fireballs.end(); ) {
        if (!(*it)->IsActive()) {
            m_Root.RemoveChild(*it);
            it = m_Fireballs.erase(it);
        }
        else {
            ++it;
        }
    }

    if (wantsFire) {
        m_Mario->Shoot();
    }

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
    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items, m_Enemies, m_Fireballs);

    // 4. 處理實體間的互動邏輯 (收集、頂飛)
    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items, m_Enemies, m_Fireballs);

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
    float triggerX = 0.0f;

    if (marioWorldX > m_CameraX + triggerX) {
        m_CameraX = marioWorldX - triggerX;
    }

    float leftScreenBoundary = m_CameraX - 400.0f + 25.0f;
    if (m_Mario->GetPosition().x < leftScreenBoundary) {
        m_Mario->SetPosition({ leftScreenBoundary, m_Mario->GetPosition().y });
    }

    // 更新所有實體的螢幕渲染座標
    m_Mario->UpdateRenderPosition(m_CameraX, m_CameraZoom);

    for (auto& block : m_CurrentMapBlocks) {
        block->UpdateRenderPosition(m_CameraX, m_CameraZoom);
    }

    for (auto& item : m_Items) {
        item->UpdateRenderPosition(m_CameraX, m_CameraZoom);
    }

    // 新增：更新敵人的渲染座標，使其跟隨攝影機偏移
    for (auto& enemy : m_Enemies) {
        if (enemy->IsActive()) {
            enemy->UpdateRenderPosition(m_CameraX, m_CameraZoom);
        }
    }

    for (auto& fb : m_Fireballs) {
        if (fb->IsActive()) fb->UpdateRenderPosition(m_CameraX, m_CameraZoom);
    }
}

void App::End() {
    LOG_TRACE("End");
}