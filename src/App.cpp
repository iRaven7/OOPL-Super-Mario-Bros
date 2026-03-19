#include "App.hpp"

#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");

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

void App::Update() {

    //TODO: do your things here and delete this line <3

    // 渲染所有子節點
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

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}