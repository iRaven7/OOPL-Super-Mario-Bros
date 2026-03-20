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

}
    /*
    // 重設攝影機
    m_CameraX = 0.0f;
    m_Root.SetPosition({ 0.0f, 0.0f }); // 重設根節點位置

    // 重設瑪利歐位置 (依據關卡設定)
    m_Mario->SetPosition({ -300.0f, -150.0f }); // 臨時地板高度
}
*/

void App::Update() {
    // 1. 計算 Delta Time
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

    // 3. 更新物理系統
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump);

    // 4. 更新攝影機邏輯
    UpdateCamera();

    // 5. 渲染所有子節點
    m_Root.Update();

    // --- 除錯與系統控制區塊 ---
    LOG_DEBUG("Mario X: {:.2f}, Y: {:.2f}, IsGrounded: {}",
        m_Mario->GetPosition().x, m_Mario->GetPosition().y, m_Mario->IsGrounded());

    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_0)) LoadLevel(0);
    if (Util::Input::IsKeyPressed(Util::Keycode::NUM_1)) LoadLevel(1);

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}
/*
void App::UpdateCamera() {
    // 瑪利歐在世界座標中的 X
    float marioWorldX = m_Mario->GetPosition().x;

    // 觸發攝影機滚動的閾值 (螢幕中心的世界座標)
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
*/

void App::UpdateCamera() {
    // 取得瑪利歐目前的 X 座標
    float marioX = m_Mario->GetPosition().x;

    // 定義螢幕中線作為攝影機開始滾動的觸發點
    // 假設原點 (0,0) 在螢幕正中央，螢幕寬度為 800，中線大約是 0.0f
    float triggerX = 0.0f;

    if (marioX > triggerX) {
        // 計算瑪利歐超出觸發點的距離
        float deltaX = marioX - triggerX;

        // 累加至總攝影機位移量 (若後續需要計算絕對世界座標時會用到)
        m_CameraX += deltaX;

        // 1. 將瑪利歐的螢幕 X 座標固定在觸發點上
        m_Mario->SetPosition({ triggerX, m_Mario->GetPosition().y });

        // 2. 將所有地圖方塊向左平移 deltaX
        for (auto& block : m_CurrentMapBlocks) {
            glm::vec2 pos = block->GetPosition();
            pos.x -= deltaX;
            block->SetPosition(pos);
        }
    }

    // 處理左邊界限制 (瑪利歐無法向左走出螢幕)
    // 假設螢幕左邊緣為 -400.0f，瑪利歐的半寬約為 25.0f
    float leftScreenBoundary = -400.0f + 25.0f;
    if (m_Mario->GetPosition().x < leftScreenBoundary) {
        m_Mario->SetPosition({ leftScreenBoundary, m_Mario->GetPosition().y });
    }
}


void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}