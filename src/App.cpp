#include "App.hpp"
#include "Mario.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp" // 假設 PTSD 有提供時間獲取

void App::Start() {
    LOG_TRACE("Start");

    // 1. 初始化瑪利歐
    m_Mario = std::make_shared<Mario>();
    m_Mario->SetPosition({ 0.0f, 0.0f }); // 起始位置
    m_Root.AddChild(m_Mario); // 掛載到渲染根節點

    LoadLevel(0);

    m_CurrentState = State::UPDATE;
}

void App::LoadLevel(int level) {
    m_CurrentLevel = level;

    // 清除舊的地圖節點
    for (auto& block : m_CurrentMapBlocks) {
        m_Root.RemoveChild(block);
    }
    m_CurrentMapBlocks.clear();

    // 判斷讀取路徑
    std::string mapPath;
    if (level == 0) {
        mapPath = RESOURCE_DIR"/Map/test_place.txt";
    }
    else {
        mapPath = RESOURCE_DIR"/Map/level" + std::to_string(level) + ".txt";
    }

    // 重新生成方塊並掛載至 Renderer
    m_CurrentMapBlocks = m_MapManager.LoadMap(mapPath);
    for (auto& block : m_CurrentMapBlocks) {
        m_Root.AddChild(block);
    }

    LOG_INFO("已載入關卡: {}", level);

    // 重設攝影機
    m_CameraX = 0.0f;
    m_Root.SetPosition({ 0.0f, 0.0f }); // 重設根節點位置

    // 重設瑪利歐位置 (依據關卡設定)
    m_Mario->SetPosition({ -300.0f, -150.0f }); // 臨時地板高度
}

void App::Update() {
    // 計算 Delta Time
    float deltaTime = static_cast<float>(Util::Time::GetDeltaTime());

    //TODO: do your things here and delete this line <3

    // 渲染所有子節點
    m_Root.Update();

    // 1. 擷取輸入
    float inputDirection = 0.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) inputDirection = 1.0f;
    else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) inputDirection = -1.0f;

    bool isSprinting = Util::Input::IsKeyPressed(Util::Keycode::Z);
    bool wantsJump = Util::Input::IsKeyPressed(Util::Keycode::SPACE); // 空白鍵跳躍

    // 2. 更新瑪利歐物理
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump);

    // 3. 更新攝影機 (關鍵！)
    UpdateCamera();

    // 4. 渲染
    m_Root.Update();

    // 實作快捷鍵以便測試關卡切換 (0 為測試，1-5 為正式關卡)
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_0)) LoadLevel(0);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_1)) LoadLevel(1);


    float deltaTime = 1.0f / 60.0f; // 若 PTSD 框架未提供時間獲取函數，暫以固定 60 FPS 計算

    float inputDirection = 0.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {
        inputDirection = 1.0f;
    }
    else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) {
        inputDirection = -1.0f;
    }

    // 假設 Z 鍵為衝刺
    bool isSprinting = Util::Input::IsKeyPressed(Util::Keycode::Z);

    // 假設 m_Mario 是 Character 類別的實體
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting);
    /*
     * Do not touch the code below as they serve the purpose for
     * closing the window.
     */
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::UpdateCamera() {
    // 瑪利歐在世界座標中的 X
    float marioWorldX = m_Mario->GetPosition().x;

    // 觸發攝影機?動的閾值 (螢幕中心的世界座標)
    float triggerX = m_CameraX + (SCREEN_WIDTH * CAMERA_DEADZONE_RATIO);

    // 原版瑪利歐機制：只有向右超過中線時攝影機才前進
    if (marioWorldX > triggerX) {
        m_CameraX = marioWorldX - (SCREEN_WIDTH * CAMERA_DEADZONE_RATIO);
    }

    // 將渲染根節點向左移動 CameraX 的距離，模擬攝影機右移
    m_Root.SetPosition({ -m_CameraX, 0.0f });

    // 限制瑪利歐不能回到螢幕左邊界 (這不算物理碰撞，是鏡頭限制)
    float leftBoundary = m_CameraX - (SCREEN_WIDTH * 0.5f); // 假設原點在螢幕中心
    // 註：上方的 Screen 座標系假設可能需要調整，取決於 PTSD 的 Viewport 設定。
    // 如果原點在螢幕左下角，leftBoundary = m_CameraX;

    // 這裡我們假設 PTSD 原點在螢幕中心，螢幕範圍是 -400 到 400
    leftBoundary = m_CameraX - 400.0f;

    if (m_Mario->GetPosition().x < leftBoundary + 25.0f) { // 25 為瑪利歐半寬
        m_Mario->SetPosition({ leftBoundary + 25.0f, m_Mario->GetPosition().y });
    }
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}