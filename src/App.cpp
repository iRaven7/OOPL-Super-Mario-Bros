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
    m_CurrentState = State::UPDATE;

    m_Mario = std::make_shared<Mario>();

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

    // 第一次載入關卡 (這會自動把瑪利歐跟背景加進去)
    LoadLevel(0);

    // 測試用：讓瑪利歐一開始就是大隻的
    m_Mario->ChangeState(std::make_unique<BigMarioState>());
}

void App::LoadLevel(int level) {
    m_CurrentLevel = level;

    // 先把舊地圖的垃圾通通掃掉
    for (auto& block : m_CurrentMapBlocks) { m_Root.RemoveChild(block); }
    m_CurrentMapBlocks.clear();

    for (auto& enemy : m_Enemies) { m_Root.RemoveChild(enemy); }
    m_Enemies.clear();

    for (auto& item : m_Items) { m_Root.RemoveChild(item); }
    m_Items.clear();

    for (auto& fb : m_Fireballs) { m_Root.RemoveChild(fb); }
    m_Fireballs.clear();

    if (m_Background) { m_Root.RemoveChild(m_Background); }
    m_Root.RemoveChild(m_Mario);

    // 然後換上新的藍天背景，並把瑪利歐放回畫面
    m_Background = std::make_shared<Background>(RESOURCE_DIR"/Blocks/sky.png");
    m_Root.AddChild(m_Background);
    m_Root.AddChild(m_Mario);

    // 再來讀取新地圖
    std::string mapPath;
    if (level == 0) {
        mapPath = RESOURCE_DIR"/Map/test_place.txt";
    }
    else {
        mapPath = RESOURCE_DIR"/Map/level" + std::to_string(level) + ".txt";
    }

    m_MapManager.LoadMap(mapPath, m_CurrentMapBlocks, m_Enemies, m_Items);

    // 把地圖物件加進渲染樹 (這段迴圈絕對不能重複出現喔！)
    for (auto& block : m_CurrentMapBlocks) { m_Root.AddChild(block); }
    for (auto& enemy : m_Enemies) { m_Root.AddChild(enemy); }
    for (auto& item : m_Items) { m_Root.AddChild(item); }

    // 把攝影機跟瑪利歐歸位
    m_CameraX = 0.0f;
    m_Mario->SetPosition({ -300.0f, 1500.0f });
    m_Mario->SetVelocity({ 0.0f, 0.0f });

    // 如果瑪利歐還在滑旗桿的狀態，幫他解除封印
    if (m_Mario->IsControlLocked()) {
        if (m_Mario->GetSize().y > 16.0f) {
            m_Mario->ChangeState(std::make_unique<BigMarioState>(), false);
        }
        else {
            m_Mario->ChangeState(std::make_unique<SmallMarioState>(), false);
        }
    }

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
    float deltaTime = static_cast<float>(Util::Time::GetDeltaTimeMs()) / 1000.0f;
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    auto& stateManager = GameStateManager::GetInstance();

    // 🌟 升級版：過場狀態攔截 (同時處理過關與死亡)
    if (m_IsTransitioning) {
        m_LevelTransitionTimer -= deltaTime;
        if (m_LevelTransitionTimer <= 0.0f) {
            m_IsTransitioning = false;

            if (m_IsDeadTransition) {
                // 死亡重來：砍掉原本的瑪利歐，生一個新的，並重新載入本關
                m_IsDeadTransition = false;
                m_Root.RemoveChild(m_Mario);
                m_Mario = std::make_shared<Mario>();
                m_Root.AddChild(m_Mario);
                LoadLevel(m_CurrentLevel);
            }
            else {
                // 正常過關：載入下一關
                LoadLevel(m_CurrentLevel + 1);
            }
        }

        // 過場期間只更新畫面，達成時間暫停效果
        UpdateUI();
        UpdateCamera();
        m_Root.Update();
        return;
    }

    stateManager.UpdateTime(deltaTime);

    if (m_Mario->IsTransforming()) {
        m_Mario->UpdateTransformation(deltaTime);
        m_Root.Update();
        return;
    }

    if (stateManager.IsLevelComplete()) {
        TriggerLevelTransition();
        stateManager.SetLevelComplete(false);
        return;
    }

    // 🌟 檢查死亡條件
    if (m_Mario->IsDead()) {
        TriggerDeath();
        return;
    }

    // 檢查鑽水管邏輯
    if (wantsCrouch && m_Mario->IsGrounded() && !m_Mario->IsControlLocked()) {
        auto marioPos = m_Mario->GetPosition();
        auto marioSize = m_Mario->GetSize();

        for (auto& block : m_CurrentMapBlocks) {
            if (block->IsPipeEntrance()) {
                auto blockPos = block->GetPosition();

                // 算一下瑪利歐是不是剛好站在這根水管的上面
                if (std::abs(marioPos.x - (blockPos.x + 16.0f)) < 20.0f &&
                    std::abs((marioPos.y - marioSize.y / 2.0f) - (blockPos.y + 16.0f)) < 8.0f) {

                    int target = block->GetTargetLevel();
                    // 讓瑪利歐往下鑽 64 像素 (剛好是兩格水管深)
                    m_Mario->ChangeState(std::make_unique<PipeSlideState>(marioSize.y > 16.0f, marioPos.y - 64.0f, target), false);
                    m_Mario->SetPosition({ blockPos.x + 16.0f, marioPos.y }); // 自動幫瑪利歐對齊水管正中央
                    break;
                }
            }
        }
    }

    // 檢查是不是鑽到底了，準備換地圖
    if (auto pipeState = dynamic_cast<PipeSlideState*>(m_Mario->GetState())) {
        if (pipeState->IsDownReached()) {
            LoadLevel(pipeState->GetTargetLevel());
        }
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

    if (m_Background) {
        m_Background->UpdateRenderPosition(m_CameraX, m_CameraZoom);
    }

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

void App::TriggerDeath() {
    m_IsTransitioning = true;
    m_IsDeadTransition = true;
    m_LevelTransitionTimer = 2.0f; // 死亡時畫面凍結兩秒
}

void App::TriggerLevelTransition() {
    m_IsTransitioning = true;
    m_LevelTransitionTimer = 2.0f; // 讓畫面停住 2 秒鐘

    // 以後如果你想加過關音樂，或是讓瑪利歐播個勝利動畫，都可以寫在這裡喔！
}

void App::End() {
    LOG_TRACE("End");
}