#include "App.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "GameStateManager.hpp"
#include "SFXManager.hpp"
#include "BGMManager.hpp"
#include "MarioState.hpp"
#include "PiranhaPlant.hpp"
#include "config.hpp"   // WINDOW_WIDTH / WINDOW_HEIGHT
#include <iomanip>
#include <sstream>

// Half the window in pixels — render translations are centred at (0,0), so the
// view spans ±kHalfW x ±kHalfH. Derived from the window size so they stay correct
// if WINDOW_WIDTH / WINDOW_HEIGHT change.
static constexpr float kHalfW = WINDOW_WIDTH / 2.0f;
static constexpr float kHalfH = WINDOW_HEIGHT / 2.0f;

namespace {
// level2_end is a continuation segment, not a numbered main level — give it an
// id outside the 0-9 selector / sub-map (11,12) / cutscene (20) ranges so the
// number keys and level progression still map to the real levels. Its flagpole
// leads on to level 3.
constexpr int kLevel2End = 21;

// Falling below row 28 (worldY = 200 - Row*16) means Mario has dropped into a
// pit — instantly fatal regardless of size.
constexpr float kPitDeathY = 200.0f - 28.0f * 16.0f;   // = -248

// --- Pipe transport table -------------------------------------------------
// Mario enters a pipe by pressing the matching button while standing at a
// registered location. Coordinates are derived from the map grid (1-based Col,
// 0-based Row) the same way MapManager lays tiles out:
//     worldX = -300 + (Col-1) * 16      worldY = 200 - Row * 16
// Vertical (Down) pipes are 2 tiles wide; their trigger/spawn X carries the
// +8 centre offset ("shift right by 8f") so Mario lines up with the mouth.
struct PipeWarp {
    enum class Entry { Down, Right, Left };
    int       fromLevel;   // active only while this level is loaded
    glm::vec2 trigger;     // world position Mario triggers from
    Entry     entry;       // button + slide direction
    int       toLevel;     // destination level id
    float     spawnX;      // destination spawn X (world)
    bool      riseOut;     // emerge with a vertical PipeExitState rise
    bool      exactSpawnY; // pin spawnY exactly (skip ground-snap)
    float     spawnY;      // destination spawn Y (world), used when exactSpawnY
};

// Spec (Col, Row). Destinations marked "@ Col,Row" pin the landing exactly via
// spawnY = 200 - Row*16 + 16 (Mario standing on the row's pipe top), because the
// landing pipe has solid blocks stacked above it that ground-snap would catch.
//   L1 (64,21)  -> Pipe1 (vertical)
//   Pipe1 (14,19) -> L1   (horizontal)
//   L2 (104,22) -> Pipe2 (vertical)
//   Pipe2 (13,19) -> L2 @ (116,23) (horizontal)   -> spawnY = 200-23*16+16 = -152
//   L2 (168,22) -> level2_end @ (4,23) (horizontal) -> spawnY = -152
const PipeWarp kPipeWarps[] = {
    { 1,  { 716.0f,  -136.0f }, PipeWarp::Entry::Down,  11, -284.0f, false, false, 0.0f    },
    { 11, { -92.0f,  -104.0f }, PipeWarp::Entry::Right,  1, 2412.0f, true,  false, 0.0f    },
    { 2,  { 1356.0f, -152.0f }, PipeWarp::Entry::Down,  12, -284.0f, false, false, 0.0f    },
    { 12, { -108.0f, -104.0f }, PipeWarp::Entry::Right,  2, 1548.0f, true,  true,  -152.0f },
    { 2,  { 2372.0f, -152.0f }, PipeWarp::Entry::Right,  kLevel2End, -244.0f, true, true, -152.0f },
};
} // namespace

void App::Start() {
    LOG_TRACE("Start");

    // Screen shows 18 tiles across: 1280px / (18 tiles * 16px) ≈ 4.44 zoom.
    m_CameraZoom = 1280.0f / (18 * 16.0f);
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

    LoadLevel(1);
}

void App::LoadLevel(int level, float spawnX, bool fromPipe, float spawnY) {
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

    m_Background = nullptr;
    m_Root.AddChild(m_Mario);

    // Per-level configuration.
    // Sub-maps are numbered 10+N (level 1's sub-map = 11, level 2's = 12, …).
    // Add a new case here whenever a new main level or sub-map is created.
    struct LevelConfig {
        std::string    mapPath;
        LevelPipeConfig pipes;
        // 18 tiles wide: 1280px / (18 tiles * 16px) ≈ 4.44
        float          cameraZoom   = 1280.0f / (18 * 18.0f);
        float          initCameraX  = 0.0f;
        bool           cameraLocked = false;
        float          cameraY      = 0.0f;
        // Empty string = no background object; the renderer clear color (black) shows through.
        std::string    backgroundPath = RESOURCE_DIR"/Blocks/sky.png";
        // Default spawn X used when LoadLevel is called without an explicit spawnX.
        // Corresponds to the leftmost column of the map (-300) unless overridden.
        float          defaultSpawnX = -300.0f;
        // Adjusts the auto-resolved (ground-snapped) spawn height. Negative
        // lowers it — e.g. level 3's start column has a castle tile that
        // ground-snap latches onto, so the spawn is dropped to the real floor.
        float          defaultSpawnYOffset = 0.0f;
        // Looping background theme for this map.
        BGMManager::Track bgmTrack = BGMManager::Track::Ground;
    };

    LevelConfig cfg;
    switch (level) {
        case 0:   // test / debug map
            cfg.mapPath = RESOURCE_DIR"/Map/test_place.txt";
            break;

        case 1:   // World 1-1
            cfg.mapPath      = RESOURCE_DIR"/Map/level1.txt";
            cfg.pipes        = { 11, -284.0f, -1, 0.0f };  // pipe1 spawn: col 2
            break;

        case 11:  // Sub-map for level 1 (pipe1) — underground, solid black background
            cfg.mapPath      = RESOURCE_DIR"/Map/pipe1.txt";
            cfg.pipes        = { -1, 0.0f, 1, 2412.0f };
            cfg.cameraZoom   = 1280.0f / (17 * 16.0f);
            cfg.initCameraX  = -310.0f + (17 * 16.0f / 2.0f);
            cfg.cameraLocked = true;
            cfg.cameraY      = -30.0f;
            cfg.backgroundPath = "";
            cfg.bgmTrack     = BGMManager::Track::Underground;
            break;

        case 2:   // World 1-2 (underground) — solid black background
            cfg.mapPath      = RESOURCE_DIR"/Map/level2.txt";
            cfg.pipes        = { 12, -284.0f, -1, 0.0f };  // pipe2 spawn: col 2
            cfg.backgroundPath = "";
            cfg.defaultSpawnX  = -284.0f;  // level2 normal spawn: col 2, row 13
            cfg.bgmTrack       = BGMManager::Track::Underground;
            break;

        case 12:  // Sub-map for level 2 (pipe2) — underground, solid black background
            cfg.mapPath      = RESOURCE_DIR"/Map/pipe2.txt";
            cfg.pipes        = { -1, 0.0f, 2, 0.0f };
            cfg.cameraZoom   = 1280.0f / (17 * 16.0f);
            cfg.initCameraX  = -310.0f + (17 * 16.0f / 2.0f);
            cfg.cameraLocked = true;
            cfg.cameraY      = -30.0f;
            cfg.backgroundPath = "";
            cfg.bgmTrack     = BGMManager::Track::Underground;
            break;

        case 20:  // Level-2 intro cutscene: Mario enters a pipe, then walks to
                  // the next pipe. Driven by Update/StartLevel2Cutscene, not by
                  // normal gameplay. Uses the default sky background + castle prop.
            cfg.mapPath = RESOURCE_DIR"/Map/level2_animation.txt";
            break;

        case kLevel2End:   // level2_end — surface ending reached via level 2's far pipe
            cfg.mapPath          = RESOURCE_DIR"/Map/level2_end.txt";
            cfg.pipes.flagStopX  = 180.0f;   // end walk stops/hides at col 31
            break;

        case 3:   // World 1-3
            cfg.mapPath          = RESOURCE_DIR"/Map/level3.txt";
            cfg.defaultSpawnX    = -252.0f;  // spawn at col 4
            cfg.defaultSpawnYOffset = -48.0f; // drop 3 tiles off the start castle onto the floor
            cfg.pipes.flagStopX  = 2404.0f;  // end walk stops/hides at col 170
            break;

        // Add case 13 … here for future levels
        default:
            cfg.mapPath = RESOURCE_DIR"/Map/level" + std::to_string(level) + ".txt";
            break;
    }

    if (!cfg.backgroundPath.empty()) {
        m_Background = std::make_shared<Background>(cfg.backgroundPath);
        m_Root.AddChild(m_Background);
    }

    m_MapManager.LoadMap(cfg.mapPath, m_CurrentMapBlocks, m_Enemies, m_Items, cfg.pipes);

    for (auto& block : m_CurrentMapBlocks) { m_Root.AddChild(block); }
    for (auto& enemy : m_Enemies) { m_Root.AddChild(enemy); }
    for (auto& item : m_Items) { m_Root.AddChild(item); }

    m_CameraZoom   = cfg.cameraZoom;
    m_CameraX      = cfg.initCameraX;
    m_CameraLocked = cfg.cameraLocked;
    m_CameraY      = cfg.cameraY;

    // Apply level-specific default spawn X when the caller used the generic default.
    if (spawnX == -300.0f) spawnX = cfg.defaultSpawnX;

    // Resolve the landing height. By default, find the highest collidable floor
    // surface at spawnX and land Mario on it; an explicit spawnY overrides this
    // (pipe exits whose landing pipe has solid blocks stacked overhead, which
    // ground-snap would wrongly latch onto).
    if (spawnY == kAutoSpawnY) {
        float groundTop = -10000.0f;
        for (const auto& block : m_CurrentMapBlocks) {
            if (!block->IsActive() || !block->IsCollidable()) continue;
            auto bPos  = block->GetCollisionPosition();
            auto bSize = block->GetSize();
            if (std::abs(bPos.x - spawnX) < bSize.x) {
                float top = bPos.y + bSize.y * 0.5f;
                if (top > groundTop) groundTop = top;
            }
        }
        spawnY = (groundTop > -9999.0f) ? (groundTop + 8.0f) : 1500.0f;
        spawnY += cfg.defaultSpawnYOffset;   // per-level nudge (e.g. level 3)
    }

    // At this zoom the view is only ~16x9 tiles, so frame the camera on the spawn
    // point: Mario's spawn height sits in the lower third, and he starts in the
    // left half of the view (camera then scrolls right as he advances). Locked
    // sub-maps keep their hand-tuned camera placement.
    if (!cfg.cameraLocked) {
        float viewHalfW = kHalfW / m_CameraZoom;
        float viewHalfH = kHalfH / m_CameraZoom;
        m_CameraX = spawnX + viewHalfW * 0.5f;
        m_CameraY = spawnY + viewHalfH * 0.45f;
    }

    // Raise the viewport by one tile (positive cameraY shifts the view up).
    m_CameraY += 16.0f;

    m_Mario->SetVelocity({ 0.0f, 0.0f });
    m_Mario->SetZIndex(50);
    m_Mario->SetPoleWalkInvisible(false);
    m_Mario->SetVisible(true);

    if (fromPipe) {
        // A big/fire Mario rises with the taller hitbox; the exact tier is restored
        // once he surfaces (see the PipeExitState handler in Update).
        bool isBig = (m_Mario->GetPowerTier() != Mario::PowerTier::Small);
        // Place Mario inside the pipe (3 tiles below standing pos) then animate up
        m_Mario->SetPosition({ spawnX, spawnY - 48.0f });
        m_Mario->ChangeState(std::make_unique<PipeExitState>(isBig, spawnY), false);
    }
    else {
        m_Mario->SetPosition({ spawnX, spawnY });
        if (m_Mario->IsControlLocked()) {
            // Arriving still in a control-locked transitional state (pole / pipe
            // slide): restore his retained power tier rather than collapsing
            // Fire down to Big from the hitbox size.
            m_Mario->ApplyPowerState();
        }
    }

    // The level-2 intro scene immediately hands control to the scripted cutscene.
    if (level == 20) {
        StartLevel2Cutscene();
    }
    else {
        m_CutscenePhase = CutscenePhase::None;
    }

    // Kick off this map's looping background theme.
    BGMManager::GetInstance().PlayLevel(cfg.bgmTrack);

    LOG_INFO("Level loaded: {}", level);
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
            LoadLevel(1);
        }
        UpdateUI();
        m_Root.Update();
        return;
    }

    if (m_IsTransitioning) {
        if (m_IsDeadTransition) {
            // Death: play out the bounce, then either game over or respawn.
            m_LevelTransitionTimer -= deltaTime;
            m_Mario->UpdateDeathAnimation(deltaTime);

            if (m_LevelTransitionTimer <= 0.0f) {
                m_IsTransitioning = false;
                m_IsDeadTransition = false;
                if (stateManager.GetLives() <= 0) {
                    m_IsGameOver = true;
                    m_GameOverTimer = 4.0f;
                    m_GameOverUI->SetVisible(true);
                    SFXManager::GetInstance().Play(SFXManager::Sound::GameOver);
                }
                else {
                    m_Root.RemoveChild(m_Mario);
                    m_Mario = std::make_shared<Mario>();
                    m_Root.AddChild(m_Mario);
                    stateManager.ResetTime();   // respawn starts the clock over
                    LoadLevel(m_CurrentLevel);
                }
            }
        }
        else {
            // Stage clear: wait for the course-clear fanfare to finish before
            // loading the next level.
            if (!BGMManager::GetInstance().IsPlaying()) {
                m_IsTransitioning = false;
                stateManager.ResetTime();   // each new level starts on a full clock
                if (m_CurrentLevel == kLevel2End) {
                    // level2_end's flagpole leads on to the real level 3.
                    LoadLevel(3);
                }
                else {
                    // NOTE: the level-2 intro cutscene (level 20 /
                    // StartLevel2Cutscene) is on hold — clearing 1-1 loads the
                    // real level 2 directly.
                    LoadLevel(m_CurrentLevel + 1);
                }
            }
        }
        UpdateUI();
        UpdateCamera();
        m_Root.Update();
        return;
    }

    // Scripted level-2 intro cutscene takes over the whole frame while active.
    if (m_CutscenePhase != CutscenePhase::None) {
        UpdateCutscene(deltaTime);
        return;
    }

    stateManager.UpdateTime(deltaTime);

    if (m_Mario->IsTransforming()) {
        m_Mario->UpdateTransformation(deltaTime);
        m_Root.Update();
        return;
    }

    if (stateManager.IsLevelComplete()) {
        stateManager.ApplyTimeBonus();   // 50 pts per remaining second
        // The stage-clear fanfare already started when Mario stepped off the
        // pole; the transition below holds until it finishes.
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
    bool isJumpHeld = Util::Input::IsKeyPressed(Util::Keycode::SPACE);
    bool wantsCrouch = Util::Input::IsKeyPressed(Util::Keycode::DOWN);
    bool wantsFire = Util::Input::IsKeyDown(Util::Keycode::X);

    // Resolve crouch state BEFORE animation/physics so both act on it this same
    // frame. Doing it afterwards lagged the crouch by a frame: on a fast run the
    // slide pose showed late, and on a slow run friction dragged velocity under
    // the slide threshold during the lagged frame, skipping the slide entirely.
    //
    // Sticky crouch: can't stand while a solid block is directly overhead, or
    // Mario would clip into it.
    if (m_Mario->IsCrouching() && !wantsCrouch && !m_Mario->CanStandUp(m_CurrentMapBlocks)) {
        wantsCrouch = true;
    }
    m_Mario->SetCrouching(wantsCrouch);

    // While crouched on the ground Mario can't actively steer; whatever
    // horizontal momentum he carried in bleeds off through friction — that decay
    // is the crouch-slide.
    if (m_Mario->IsCrouching() && m_Mario->IsGrounded()) {
        inputDirection = 0.0f;
    }

    // Pipe entrance — coordinate-driven. Enter a pipe by pressing the matching
    // button while standing at a registered warp location (see kPipeWarps).
    if (!m_Mario->IsControlLocked()) {
        glm::vec2 marioPos  = m_Mario->GetPosition();
        glm::vec2 marioSize = m_Mario->GetSize();
        bool      isBig     = marioSize.y > 16.0f;

        for (const auto& warp : kPipeWarps) {
            if (warp.fromLevel != m_CurrentLevel) continue;

            if (warp.entry == PipeWarp::Entry::Down) {
                // Vertical pipe: stand on top (grounded), centred on the mouth,
                // press Down. trigger.x already carries the +8 centre offset.
                float pipeTopFace = warp.trigger.y + 8.0f;
                bool  feetNearTop = std::abs((marioPos.y - marioSize.y / 2.0f) - pipeTopFace) < 12.0f;
                if (wantsCrouch && m_Mario->IsGrounded() &&
                    std::abs(marioPos.x - warp.trigger.x) < 12.0f && feetNearTop) {
                    m_Mario->SetPosition({ warp.trigger.x, marioPos.y });
                    m_Mario->ChangeState(std::make_unique<PipeSlideState>(
                        isBig, PipeSlideState::SlideDir::Down,
                        marioPos.y - 64.0f, warp.toLevel, warp.spawnX, warp.riseOut,
                        warp.spawnY, warp.exactSpawnY), false);
                    SFXManager::GetInstance().Play(SFXManager::Sound::Pipe);
                    break;
                }
            }
            else {
                // Horizontal pipe: stand at the mouth, press toward it.
                float dir = (warp.entry == PipeWarp::Entry::Right) ? 1.0f : -1.0f;
                bool  pressingInto = (dir > 0.0f) ? (inputDirection > 0.0f) : (inputDirection < 0.0f);
                if (pressingInto &&
                    std::abs(marioPos.x - warp.trigger.x) < 14.0f &&
                    std::abs(marioPos.y - warp.trigger.y) < marioSize.y / 2.0f + 16.0f) {
                    auto sdir = (dir > 0.0f) ? PipeSlideState::SlideDir::Right
                                             : PipeSlideState::SlideDir::Left;
                    m_Mario->ChangeState(std::make_unique<PipeSlideState>(
                        isBig, sdir, marioPos.x + dir * 64.0f,
                        warp.toLevel, warp.spawnX, warp.riseOut,
                        warp.spawnY, warp.exactSpawnY), false);
                    SFXManager::GetInstance().Play(SFXManager::Sound::Pipe);
                    break;
                }
            }
        }
    }

    // Pipe slide finished — load the destination. The warp's riseOut flag (carried
    // by the slide state) decides whether Mario emerges with a vertical rise.
    if (auto pipeState = dynamic_cast<PipeSlideState*>(m_Mario->GetState())) {
        if (pipeState->IsDownReached()) {
            // Sound the pipe SFX as these specific warps complete: Pipe1 -> Level 1,
            // Pipe2 -> Level 2, and Level 2 -> level2_end.
            int target = pipeState->GetTargetLevel();
            if (target == 1 || target == 2 || target == kLevel2End) {
                SFXManager::GetInstance().Play(SFXManager::Sound::Pipe);
            }
            float spawnY = pipeState->HasExactSpawnY() ? pipeState->GetSpawnY() : kAutoSpawnY;
            LoadLevel(pipeState->GetTargetLevel(), pipeState->GetSpawnX(),
                      pipeState->RisesOut(), spawnY);
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

    // Jump SFX: the physics jump fires only when grounded; pick the pitch by the
    // state's true height (GetSize halves while crouched, so don't use it here).
    if (wantsJump && m_Mario->IsGrounded() && !m_Mario->IsControlLocked()) {
        bool isBig = m_Mario->GetState() && m_Mario->GetState()->GetHitboxSize().y > 16.0f;
        SFXManager::GetInstance().Play(isBig ? SFXManager::Sound::JumpSuper
                                             : SFXManager::Sound::JumpSmall);
    }

    m_Mario->Update(deltaTime);
    m_Mario->UpdateAnimation(deltaTime, inputDirection);
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, m_CurrentMapBlocks, isJumpHeld);

    // Pit death: dropping below row 28 is fatal. Ignore while control is locked
    // (pipe/pole sequences move Mario vertically by design).
    if (!m_Mario->IsControlLocked() && !m_Mario->IsGodMode() && m_Mario->GetPosition().y < kPitDeathY) {
        m_Mario->Die();
    }

    if (wantsFire) m_Mario->Shoot();

    // Transition out of pipe exit state once Mario has surfaced
    if (auto exitState = dynamic_cast<PipeExitState*>(m_Mario->GetState())) {
        if (exitState->IsExitDone()) {
            m_Mario->ApplyPowerState();   // restore Small/Big/Fire exactly
        }
    }

    auto newFireballs = m_Mario->PopSpawnedFireballs();
    for (auto& fb : newFireballs) {
        m_Fireballs.push_back(fb);
        m_Root.AddChild(fb);
    }

    // Update fireballs, and retire any that travel beyond the viewport so they
    // don't keep simulating off-screen. (m_CameraX/Y are this point still last
    // frame's values — UpdateCamera runs later — which is plenty accurate for
    // culling, with a tile of margin.) CleanupInactiveEntities removes them below.
    const float fbViewHalfW = kHalfW / m_CameraZoom;
    const float fbViewHalfH = kHalfH / m_CameraZoom;
    const float fbCullMargin = 16.0f;
    for (auto& fb : m_Fireballs) {
        if (!fb->IsActive()) continue;
        fb->Update(deltaTime, m_CurrentMapBlocks);
        glm::vec2 fbPos = fb->GetPosition();
        if (fbPos.x < m_CameraX - fbViewHalfW - fbCullMargin ||
            fbPos.x > m_CameraX + fbViewHalfW + fbCullMargin ||
            fbPos.y < m_CameraY - fbViewHalfH - fbCullMargin ||
            fbPos.y > m_CameraY + fbViewHalfH + fbCullMargin) {
            fb->Destroy();
        }
    }

    // Off-screen elements freeze completely: anything whose world-X lies outside
    // the visible horizontal band (plus 2 tiles of slack so it wakes just before
    // entering view) skips its per-frame simulation. Without this an off-screen
    // FlyingKoopa keeps bobbing and is first seen at the bottom of its ~3-tile
    // hover arc instead of at rest, and stray enemies/items wander while unseen.
    const float viewHalfW   = kHalfW / m_CameraZoom;
    const float wakeMargin  = 32.0f;
    const float viewLeftX   = m_CameraX - viewHalfW - wakeMargin;
    const float viewRightX  = m_CameraX + viewHalfW + wakeMargin;
    auto onScreenX = [&](float x) { return x >= viewLeftX && x <= viewRightX; };

    for (auto& item : m_Items) {
        if (item->IsActive() && onScreenX(item->GetPosition().x))
            item->Update(deltaTime, m_CurrentMapBlocks);
    }

    for (auto& enemy : m_Enemies) {
        if (!enemy->IsActive()) continue;
        if (!onScreenX(enemy->GetPosition().x)) continue;
        if (auto piranha = dynamic_cast<PiranhaPlant*>(enemy.get()))
            piranha->SetPlayerPos(m_Mario->GetPosition());
        enemy->RunAI(deltaTime, m_CurrentMapBlocks);
    }

    m_CollisionManager.ProcessInteractions(m_Mario.get(), m_CurrentMapBlocks, m_Items, m_Enemies, m_Fireballs);

    CleanupInactiveEntities(m_Fireballs);
    CleanupInactiveEntities(m_Items);

    UpdateCamera();
    m_Root.Update();

    HandleCheats();

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

    if (!m_CameraLocked && marioWorldX > m_CameraX + triggerX) {
        m_CameraX = marioWorldX - triggerX;
    }

    // Camera height is locked: m_CameraY stays at the value framed on level load
    // (see LoadLevel) and does not follow Mario vertically.

    // Keep Mario from leaving the left edge of the view (zoom-aware: the visible
    // half-width in world units is kHalfW / zoom).
    float leftScreenBoundary = m_CameraX - kHalfW / m_CameraZoom + m_Mario->GetSize().x * 0.5f;
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

    // Apply Y camera offset: shift every rendered entity's translation by -cameraY * zoom.
    // Positive cameraY moves the viewport up; negative moves it down (shows lower content).
    if (m_CameraY != 0.0f) {
        const float yShift = -m_CameraY * m_CameraZoom;
        m_Mario->m_Transform.translation.y += yShift;
        for (auto& block : m_CurrentMapBlocks)
            block->m_Transform.translation.y += yShift;
        for (auto& item : m_Items)
            item->m_Transform.translation.y += yShift;
        for (auto& enemy : m_Enemies)
            if (enemy->IsActive()) enemy->m_Transform.translation.y += yShift;
        for (auto& fb : m_Fireballs)
            if (fb->IsActive()) fb->m_Transform.translation.y += yShift;
    }

    // View culling: hide anything outside the camera view so off-screen objects
    // aren't rendered. Render translations are in screen pixels centred at (0,0),
    // so the window spans ±kHalfW x ±kHalfH; allow two tiles of margin to avoid
    // popping (covers the 64px-wide moving platforms whose centre is past the edge).
    const float marginX = 32.0f * m_CameraZoom;
    const float marginY = 32.0f * m_CameraZoom;
    auto cull = [&](Util::GameObject* obj) {
        const auto& t = obj->m_Transform.translation;
        obj->SetVisible(std::abs(t.x) <= kHalfW + marginX &&
                        std::abs(t.y) <= kHalfH + marginY);
    };
    for (auto& block : m_CurrentMapBlocks) if (block->IsActive()) cull(block.get());
    for (auto& item : m_Items)            if (item->IsActive())  cull(item.get());
    for (auto& enemy : m_Enemies)         if (enemy->IsActive()) cull(enemy.get());
    for (auto& fb : m_Fireballs)          if (fb->IsActive())    cull(fb.get());
}

void App::TriggerDeath() {
    GameStateManager::GetInstance().AddLife(-1);
    BGMManager::GetInstance().Stop();   // cut the level theme immediately
    SFXManager::GetInstance().Play(SFXManager::Sound::MarioDie);
    m_Mario->StartDeathAnimation();
    m_IsTransitioning = true;
    m_IsDeadTransition = true;
    m_LevelTransitionTimer = 2.5f;
}

void App::TriggerLevelTransition() {
    m_IsTransitioning = true;
    m_LevelTransitionTimer = 2.0f;
}

// Debug cheats, edge-triggered so each press fires once. Called from Update only
// during normal gameplay (after the game-over / transition / cutscene / transform
// early-returns), so they can't fire mid-cutscene.
void App::HandleCheats() {
    auto& state = GameStateManager::GetInstance();

    // G — toggle god mode (immune to enemy contact and pit falls).
    if (Util::Input::IsKeyDown(Util::Keycode::G)) {
        m_Mario->ToggleGodMode();
        LOG_INFO("[CHEAT] God mode {}", m_Mario->IsGodMode() ? "ON" : "OFF");
    }

    // V — cycle power: Small -> Big -> Fire -> Small. Uses the normal power-up
    // path (triggerPause=true) so Mario's feet stay planted across the resize.
    // Skipped while control is locked (a pole/pipe sequence owns his state then).
    if (Util::Input::IsKeyDown(Util::Keycode::V) && !m_Mario->IsControlLocked()) {
        MarioState* s = m_Mario->GetState();
        if (dynamic_cast<SmallMarioState*>(s))
            m_Mario->ChangeState(std::make_unique<BigMarioState>());
        else if (dynamic_cast<BigMarioState*>(s))
            m_Mario->ChangeState(std::make_unique<FireMarioState>());
        else
            m_Mario->ChangeState(std::make_unique<SmallMarioState>());
        LOG_INFO("[CHEAT] Cycled power state");
    }

    // I — 10 seconds of star power.
    if (Util::Input::IsKeyDown(Util::Keycode::I)) {
        m_Mario->ActivateStarPower(10.0f);
        LOG_INFO("[CHEAT] Star power granted");
    }

    // T — top up the clock (+50s, capped).
    if (Util::Input::IsKeyDown(Util::Keycode::T)) {
        state.AddTime(50);
        LOG_INFO("[CHEAT] +50 time");
    }

    // N — extra life.
    if (Util::Input::IsKeyDown(Util::Keycode::N)) {
        state.AddLife(1);
        LOG_INFO("[CHEAT] +1 life");
    }
}

// ---------------------------------------------------------------------------
// Level-2 intro cutscene
//
// Map = level2_animation.txt (BLOCK_SIZE 16, origin startX=-300, startY=200).
// Vertical pipe occupies grid columns 13-14 (0-based) → world centre x = -84,
// pipe top face at world y = -128. The floor rows put a standing Mario centre
// at y = -184. The walk runs along row 25 (1-based) from column 4 (world x
// -252, by the castle) to column 13 (world x -108, the second pipe mouth).
//
// Phase 1 (EnterPipe): Mario sits on top of the vertical pipe and slides down
//   into it (reusing PipeSlideState's downward slide + behind-pipe z-index).
// Phase 2 (Walk): Mario is repositioned to the castle side and walks right to
//   the pipe; reaching it loads the real level 2.
// ---------------------------------------------------------------------------
void App::StartLevel2Cutscene() {
    m_CutscenePhase = CutscenePhase::EnterPipe;

    bool isBig = m_Mario->GetSize().y > 16.0f;

    const float pipeCenterX     = -84.0f;   // columns 13-14 centre
    const float pipeTopStandY   = -120.0f;  // pipe top face (-128) + 8

    m_Mario->SetVisible(true);
    m_Mario->SetVelocity({ 0.0f, 0.0f });
    m_Mario->SetPosition({ pipeCenterX, pipeTopStandY });
    // Slide ~3 tiles down so Mario disappears behind the pipe. The target level
    // argument is unused — UpdateCutscene drives the next load itself.
    m_Mario->ChangeState(std::make_unique<PipeSlideState>(
        isBig, PipeSlideState::SlideDir::Down, pipeTopStandY - 48.0f, 2), false);
}

void App::UpdateCutscene(float deltaTime) {
    for (auto& block : m_CurrentMapBlocks) {
        block->Update(deltaTime);
    }
    m_Mario->Update(deltaTime);

    if (m_CutscenePhase == CutscenePhase::EnterPipe) {
        m_Mario->UpdateAnimation(deltaTime, 0.0f);
        m_Mario->UpdatePhysics(deltaTime, 0.0f, false, false, m_CurrentMapBlocks);

        if (auto pipe = dynamic_cast<PipeSlideState*>(m_Mario->GetState())) {
            if (pipe->IsDownReached()) {
                // Pipe entry done — reposition to the castle side and start walking.
                const float walkStartX = -252.0f;  // column 4  (1-based)
                m_CutsceneWalkEndX     = -108.0f;  // column 13 (1-based)
                const float standY     = -184.0f;  // floor top (-192) + 8

                m_Mario->ApplyPowerState();   // keep his power tier across the scene
                m_Mario->SetZIndex(50);
                m_Mario->SetVisible(true);
                m_Mario->SetPosition({ walkStartX, standY });
                m_Mario->SetVelocity({ 0.0f, 0.0f });
                m_CutscenePhase = CutscenePhase::Walk;
            }
        }
    }
    else if (m_CutscenePhase == CutscenePhase::Walk) {
        // Scripted constant-speed walk (bypasses collision so Mario reaches the
        // pipe mouth exactly, undisturbed by the pipe's own collision blocks).
        const float walkSpeed = 150.0f;
        glm::vec2 pos = m_Mario->GetPosition();
        pos.x += walkSpeed * deltaTime;

        bool done = false;
        if (pos.x >= m_CutsceneWalkEndX) {
            pos.x = m_CutsceneWalkEndX;
            done = true;
        }

        m_Mario->SetPosition(pos);
        m_Mario->SetVelocity({ walkSpeed, 0.0f });
        m_Mario->SetGrounded(true);
        m_Mario->UpdateAnimation(deltaTime, 1.0f);

        if (done) {
            m_CutscenePhase = CutscenePhase::None;
            LoadLevel(2);
            return;
        }
    }

    UpdateUI();
    UpdateCamera();
    m_Root.Update();
}

void App::End() {
    LOG_TRACE("End");
}