#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "GameStateManager.hpp"
#include <iomanip>
#include <sstream>

void App::Start() {
    LOG_TRACE("Start");

    m_CameraZoom = 1.8f;
    m_Mario = std::make_shared<Mario>();
    m_Root.AddChild(m_Mario);

    LoadLevel(0);

    m_Mario->ChangeState(std::make_unique<BigMarioState>());

    // 建立分數 UI
    m_ScoreUI = std::make_shared<Util::GameObject>();
    m_ScoreText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 24, "SCORE: 000000", Util::Color{ 255, 255, 255, 255 });
    m_ScoreUI->SetDrawable(m_ScoreText);
    m_ScoreUI->SetZIndex(100);
    m_ScoreUI->m_Transform.translation = { -300.0f, 250.0f };
    m_Root.AddChild(m_ScoreUI);

    // 建立金幣 UI
    m_CoinUI = std::make_shared<Util::GameObject>();
    m_CoinText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 24, "COINS: 00", Util::Color{ 255, 255, 255, 255 });
    m_CoinUI->SetDrawable(m_CoinText);
    m_CoinUI->SetZIndex(100);
    m_CoinUI->m_Transform.translation = { 0.0f, 250.0f };
    m_Root.AddChild(m_CoinUI);

    // 建立時間 UI
    m_TimeUI = std::make_shared<Util::GameObject>();
    m_TimeText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 24, "TIME: 400", Util::Color{ 255, 255, 255, 255 });
    m_TimeUI->SetDrawable(m_TimeText);
    m_TimeUI->SetZIndex(100);
    m_TimeUI->m_Transform.translation = { 300.0f, 250.0f };
    m_Root.AddChild(m_TimeUI);

    m_CurrentState = State::UPDATE;
}

void App::LoadLevel(int level) {
    m_CurrentLevel = level;

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

    m_MapManager.LoadMap(mapPath, m_CurrentMapBlocks, m_Enemies);

    for (auto& block : m_CurrentMapBlocks) { m_Root.AddChild(block); }
    for (auto& enemy : m_Enemies) { m_Root.AddChild(enemy); }

    m_CameraX = 0.0f;
    m_Mario->SetPosition({ -300.0f, 1500.0f });

    LOG_INFO("已載入關卡: {}", level);
}

// 整理：將原本 Update 裡的一大堆程式碼獨立拆分出來
void App::UpdateUI() {
    auto& stateManager = GameStateManager::GetInstance();

    std::ostringstream scoreSs;
    scoreSs << "SCORE: " << std::setw(6) << std::setfill('0') << stateManager.GetScore();
    m_ScoreText->SetText(scoreSs.str());

    std::ostringstream coinSs;
    coinSs << "COINS: " << std::setw(2) << std::setfill('0') << stateManager.GetCoins();
    m_CoinText->SetText(coinSs.str());

    std::ostringstream timeSs;
    timeSs << "TIME: " << std::setw(3) << std::setfill('0') << stateManager.GetTimeRemaining();
    m_TimeText->SetText(timeSs.str());
}

template <typename T>
void App::CleanupInactiveEntities(std::vector<std::shared_ptr<T>>& entities) {
    for (auto it = entities.begin(); it != entities.end(); ) {
        if (!(*it)->IsActive()) {
            m_Root.RemoveChild(*it);
            it = entities.erase(it);
        }
        else {
            ++it;
        }
    }
}

void App::Update() {
    // 1. 計算時間差與防禦性限制
    float deltaTime = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    auto& stateManager = GameStateManager::GetInstance();
    stateManager.UpdateTime(deltaTime);

    if (m_Mario->IsTransforming()) {
        m_Mario->UpdateTransformation(deltaTime);
        m_Root.Update();
        return;
    }

    // 2. 檢查過關條件
    if (m_Mario->GetPosition().x > 500.0f && !stateManager.IsLevelComplete()) {
        stateManager.SetLevelComplete(true);
    }

    if (stateManager.IsLevelComplete()) {
        LoadLevel(m_CurrentLevel + 1);
        stateManager.SetLevelComplete(false);
        return;
    }

    // 3. 擷取輸入
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

    // 4. 更新 UI
    UpdateUI();

    // 5. 更新所有實體的邏輯與物理
    m_Mario->Update(deltaTime);
    m_Mario->UpdateAnimation(deltaTime, inputDirection);
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, m_CurrentMapBlocks);
    m_Mario->SetCrouching(wantsCrouch);

    if (wantsFire) {
        m_Mario->Shoot();
    }

    auto newFireballs = m_Mario->PopSpawnedFireballs();
    for (auto& fb : newFireballs) {
        m_Fireballs.push_back(fb);
        m_Root.AddChild(fb);
    }

    for (auto& fb : m_Fireballs) {
        if (fb->IsActive()) fb->Update(deltaTime, m_CurrentMapBlocks);
    }

    for (auto& block : m_CurrentMapBlocks) {
        block->Update(deltaTime);
        if (auto newItem = block->PopSpawnedItem()) {
            m_Items.push_back(newItem);
            m_Root.AddChild(newItem);
        }
    }

    for (auto& item : m_Items) {
        if (item->IsActive()) item->Update(deltaTime, m_CurrentMapBlocks);
    }

    for (auto& enemy : m_Enemies) {
        if (enemy->IsActive()) enemy->UpdateAI(deltaTime, m_CurrentMapBlocks);
    }

    // 6. 處理實體間的互動邏輯 (只呼叫一次就好！)
    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items, m_Enemies, m_Fireballs);

    // 7. 清理已死亡或失去活性的物件
    CleanupInactiveEntities(m_Fireballs);
    CleanupInactiveEntities(m_Items);
    // 如果敵人有銷毀邏輯，這裡也可以加 CleanupInactiveEntities(m_Enemies);

    // 8. 更新攝影機與渲染
    UpdateCamera();
    m_Root.Update();

    // 除錯日誌
    LOG_DEBUG("Mario X: {:.2f}, Y: {:.2f}, IsGrounded: {}",
        m_Mario->GetPosition().x, m_Mario->GetPosition().y, m_Mario->IsGrounded());

    // 9. 關卡控制與程式退出
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

    m_Mario->UpdateRenderPosition(m_CameraX, m_CameraZoom);

    for (auto& block : m_CurrentMapBlocks) {
        block->UpdateRenderPosition(m_CameraX, m_CameraZoom);
    }

    for (auto& item : m_Items) {
        item->UpdateRenderPosition(m_CameraX, m_CameraZoom);
    }

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