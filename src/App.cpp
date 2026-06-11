#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "GameStateManager.hpp"
#include "MarioState.hpp"
#include <iomanip>
#include <sstream>

void App::Start() {
    LOG_TRACE("Start");

    m_CameraZoom = 1.8f;
    m_CurrentState = State::UPDATE;

    m_Mario = std::make_shared<Mario>();

    m_ScoreUI = std::make_shared<Util::GameObject>();
    m_ScoreText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 24, "SCORE: 000000", Util::Color{ 255, 255, 255, 255 });
    m_ScoreUI->SetDrawable(m_ScoreText);
    m_ScoreUI->SetZIndex(100);
    m_ScoreUI->m_Transform.translation = { -300.0f, 250.0f };
    m_Root.AddChild(m_ScoreUI);

    m_CoinUI = std::make_shared<Util::GameObject>();
    m_CoinText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 24, "COINS: 00", Util::Color{ 255, 255, 255, 255 });
    m_CoinUI->SetDrawable(m_CoinText);
    m_CoinUI->SetZIndex(100);
    m_CoinUI->m_Transform.translation = { -100.0f, 250.0f };
    m_Root.AddChild(m_CoinUI);

    m_LivesUI = std::make_shared<Util::GameObject>();
    m_LivesText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 24, "LIVES: 3", Util::Color{ 255, 255, 255, 255 });
    m_LivesUI->SetDrawable(m_LivesText);
    m_LivesUI->SetZIndex(100);
    m_LivesUI->m_Transform.translation = { 100.0f, 250.0f };
    m_Root.AddChild(m_LivesUI);

    m_TimeUI = std::make_shared<Util::GameObject>();
    m_TimeText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 24, "TIME: 400", Util::Color{ 255, 255, 255, 255 });
    m_TimeUI->SetDrawable(m_TimeText);
    m_TimeUI->SetZIndex(100);
    m_TimeUI->m_Transform.translation = { 300.0f, 250.0f };
    m_Root.AddChild(m_TimeUI);

    m_GameOverUI = std::make_shared<Util::GameObject>();
    m_GameOverText = std::make_shared<Util::Text>(RESOURCE_DIR"/Fonts/SMB.ttf", 48, "GAME OVER", Util::Color{ 255, 255, 255, 255 });
    m_GameOverUI->SetDrawable(m_GameOverText);
    m_GameOverUI->SetZIndex(200);
    m_GameOverUI->m_Transform.translation = { 0.0f, 0.0f };
    m_GameOverUI->SetVisible(false);
    m_Root.AddChild(m_GameOverUI);

    LoadLevel(0);
    m_Mario->ChangeState(std::make_unique<BigMarioState>());
}

void App::LoadLevel(int level, float spawnX) {
    m_CurrentLevel = level;

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

    m_Background = std::make_shared<Background>(RESOURCE_DIR"/Blocks/sky.png");
    m_Root.AddChild(m_Background);
    m_Root.AddChild(m_Mario);

    // Per-level configuration.
    // Sub-maps are numbered 10+N (level 1's sub-map = 11, level 2's = 12, …).
    // Add a new case here whenever a new main level or sub-map is created.
    struct LevelConfig {
        std::string    mapPath;
        LevelPipeConfig pipes;
        float          cameraZoom  = 1.8f;
        float          initCameraX = 0.0f;
    };

    LevelConfig cfg;
    switch (level) {
        case 0:   // test / debug map
            cfg.mapPath = RESOURCE_DIR"/Map/test_place.txt";
            break;

        case 1:   // World 1-1
            cfg.mapPath      = RESOURCE_DIR"/Map/level1.txt";
            // 'W' pipe → sub-map 11; Mario enters at default spawn (-300)
            // 'w' pipe in sub-map returns here at x = 700 (col 62 of level1)
            cfg.pipes        = { 11, -196.0f, -1, 0.0f }; // sub-map spawn at col 6 centre
            break;

        case 11:  // Sub-map for level 1
            cfg.mapPath      = RESOURCE_DIR"/Map/pipe1.txt";
            // 'w' pipe returns to level 1; Mario exits at x = 700 (beside the W pipe)
            cfg.pipes        = { -1, 0.0f, 1, 700.0f };
            cfg.cameraZoom   = 1280.0f / (17 * 16.0f);   // exactly 17 blocks wide
            cfg.initCameraX  = -310.0f + (17 * 16.0f / 2.0f);  // centre the room
            break;

        // Add case 2, case 12, case 3, case 13 … here for future levels
        default:
            cfg.mapPath = RESOURCE_DIR"/Map/level" + std::to_string(level) + ".txt";
            break;
    }

    m_MapManager.LoadMap(cfg.mapPath, m_CurrentMapBlocks, m_Enemies, m_Items, cfg.pipes);

    for (auto& block : m_CurrentMapBlocks) { m_Root.AddChild(block); }
    for (auto& enemy : m_Enemies) { m_Root.AddChild(enemy); }
    for (auto& item : m_Items) { m_Root.AddChild(item); }

    m_CameraZoom = cfg.cameraZoom;
    m_CameraX    = cfg.initCameraX;

    m_Mario->SetPosition({ spawnX, 1500.0f });
    m_Mario->SetVelocity({ 0.0f, 0.0f });
    m_Mario->SetZIndex(50);

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

void App::UpdateUI() {
    auto& stateManager = GameStateManager::GetInstance();

    std::ostringstream scoreSs;
    scoreSs << "SCORE: " << std::setw(6) << std::setfill('0') << stateManager.GetScore();
    m_ScoreText->SetText(scoreSs.str());

    std::ostringstream coinSs;
    coinSs << "COINS: " << std::setw(2) << std::setfill('0') << stateManager.GetCoins();
    m_CoinText->SetText(coinSs.str());

    std::ostringstream livesSs;
    livesSs << "LIVES: " << stateManager.GetLives();
    m_LivesText->SetText(livesSs.str());

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

    if (m_IsGameOver) {
        m_GameOverTimer -= deltaTime;
        if (m_GameOverTimer <= 0.0f) {
            m_IsGameOver = false;
            m_GameOverUI->SetVisible(false);
            stateManager.Reset();
            m_Root.RemoveChild(m_Mario);
            m_Mario = std::make_shared<Mario>();
            m_Root.AddChild(m_Mario);
            LoadLevel(0);
        }
        UpdateUI();
        m_Root.Update();
        return;
    }

    if (m_IsTransitioning) {
        m_LevelTransitionTimer -= deltaTime;

        if (m_IsDeadTransition) {
            m_Mario->UpdateDeathAnimation(deltaTime);
        }

        if (m_LevelTransitionTimer <= 0.0f) {
            m_IsTransitioning = false;
            if (m_IsDeadTransition) {
                m_IsDeadTransition = false;
                if (stateManager.GetLives() <= 0) {
                    m_IsGameOver = true;
                    m_GameOverTimer = 4.0f;
                    m_GameOverUI->SetVisible(true);
                }
                else {
                    m_Root.RemoveChild(m_Mario);
                    m_Mario = std::make_shared<Mario>();
                    m_Root.AddChild(m_Mario);
                    LoadLevel(m_CurrentLevel);
                }
            }
            else {
                LoadLevel(m_CurrentLevel + 1);
            }
        }
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

    if (m_Mario->IsDead()) {
        TriggerDeath();
        return;
    }

    // 🌟 先把按鍵讀取拉到上面，這樣程式就認識 wantsCrouch 了！
    float inputDirection = 0.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) inputDirection = 1.0f;
    else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) inputDirection = -1.0f;

    bool isSprinting = Util::Input::IsKeyPressed(Util::Keycode::Z);
    bool wantsJump = Util::Input::IsKeyDown(Util::Keycode::SPACE);
    bool wantsCrouch = Util::Input::IsKeyPressed(Util::Keycode::DOWN);
    bool wantsFire = Util::Input::IsKeyDown(Util::Keycode::X);

    if (m_Mario->IsCrouching() && m_Mario->IsGrounded()) {
        inputDirection = 0.0f;
    }

    // 檢查鑽水管邏輯
    if (wantsCrouch && m_Mario->IsGrounded() && !m_Mario->IsControlLocked()) {
        auto marioPos = m_Mario->GetPosition();
        auto marioSize = m_Mario->GetSize();

        for (auto& block : m_CurrentMapBlocks) {
            if (block->IsPipeEntrance()) {
                auto blockPos = block->GetPosition();

                if (std::abs(marioPos.x - (blockPos.x + 8.0f)) < 20.0f &&
                    std::abs((marioPos.y - marioSize.y / 2.0f) - (blockPos.y + 8.0f)) < 8.0f) {

                    int target = block->GetTargetLevel();
                    float spawnX = block->GetSpawnX();
                    m_Mario->ChangeState(std::make_unique<PipeSlideState>(marioSize.y > 16.0f, marioPos.y - 64.0f, target, spawnX), false);
                    m_Mario->SetPosition({ blockPos.x + 8.0f, marioPos.y });
                    break;
                }
            }
        }
    }

    // 檢查是不是鑽到底了
    if (auto pipeState = dynamic_cast<PipeSlideState*>(m_Mario->GetState())) {
        if (pipeState->IsDownReached()) {
            LoadLevel(pipeState->GetTargetLevel(), pipeState->GetSpawnX());
        }
    }

    UpdateUI();

    // Blocks (including moving platforms) update first so Mario physics
    // snaps to their new positions this same frame — no 1-frame lag.
    for (auto& block : m_CurrentMapBlocks) {
        block->Update(deltaTime);
        if (auto newItem = block->PopSpawnedItem()) {
            m_Items.push_back(newItem);
            m_Root.AddChild(newItem);
        }
    }

    m_Mario->Update(deltaTime);
    m_Mario->UpdateAnimation(deltaTime, inputDirection);
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, m_CurrentMapBlocks);
    m_Mario->SetCrouching(wantsCrouch);

    if (wantsFire) m_Mario->Shoot();

    auto newFireballs = m_Mario->PopSpawnedFireballs();
    for (auto& fb : newFireballs) {
        m_Fireballs.push_back(fb);
        m_Root.AddChild(fb);
    }

    for (auto& fb : m_Fireballs) {
        if (fb->IsActive()) fb->Update(deltaTime, m_CurrentMapBlocks);
    }

    for (auto& item : m_Items) {
        if (item->IsActive()) item->Update(deltaTime, m_CurrentMapBlocks);
    }

    for (auto& enemy : m_Enemies) {
        if (enemy->IsActive()) enemy->UpdateAI(deltaTime, m_CurrentMapBlocks);
    }

    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items, m_Enemies, m_Fireballs);

    CleanupInactiveEntities(m_Fireballs);
    CleanupInactiveEntities(m_Items);

    UpdateCamera();
    m_Root.Update();

    // Keys 0-9: main-level selector (sub-maps 11+ are only reachable via pipes)
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_0)) LoadLevel(0);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_1)) LoadLevel(1);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_2)) LoadLevel(2);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_3)) LoadLevel(3);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_4)) LoadLevel(4);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_5)) LoadLevel(5);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_6)) LoadLevel(6);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_7)) LoadLevel(7);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_8)) LoadLevel(8);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_9)) LoadLevel(9);

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::UpdateCamera() {
    float marioWorldX = m_Mario->GetPosition().x;
    float triggerX = 0.0f;

    if (m_Background) {
        // 背景不吃 cameraX 參數，這樣才會釘死在畫面上
        m_Background->UpdateRenderPosition(0.0f, m_CameraZoom);
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
    GameStateManager::GetInstance().AddLife(-1);
    m_Mario->StartDeathAnimation();
    m_IsTransitioning = true;
    m_IsDeadTransition = true;
    m_LevelTransitionTimer = 2.5f;
}

void App::TriggerLevelTransition() {
    m_IsTransitioning = true;
    m_LevelTransitionTimer = 2.0f;
}

void App::End() {
    LOG_TRACE("End");
}