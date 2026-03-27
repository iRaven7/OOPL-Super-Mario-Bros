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

    //m_Mario->SetMarioState(MarioState::BIG);
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
    bool wantsJump = Util::Input::IsKeyDown(Util::Keycode::SPACE);

    // ==========================================
    // 3. 更新物理系統 (修改這行！)
    // 傳入 m_CurrentMapBlocks 讓瑪利歐進行 AABB 碰撞偵測
    // ==========================================
    m_Mario->UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, m_CurrentMapBlocks);

    // 4. 更新攝影機與地圖位置
    UpdateCamera();

    // 1. 更新方塊，並提取新生成的道具
    for (auto& block : m_CurrentMapBlocks) {
        block->Update(deltaTime);
        if (auto newItem = block->PopSpawnedItem()) {
            m_Items.push_back(newItem);
            m_Root.AddChild(newItem);
        }
    }

    // 2. 更新道具，並處理與瑪利歐的碰撞與生命週期
    for (auto it = m_Items.begin(); it != m_Items.end(); ) {
        auto& item = *it;
        if (item->IsActive()) {
            item->Update(deltaTime, m_CurrentMapBlocks); // 香菇只在此時與方塊做物理碰撞

            // --- 簡易的 AABB 檢查瑪利歐是否吃到道具 (此處僅作邏輯判斷，不做物理修正) ---
            glm::vec2 marioPos = m_Mario->GetPosition();
            glm::vec2 marioSize = m_Mario->GetSize();
            glm::vec2 itemPos = item->GetPosition();
            glm::vec2 itemSize = item->GetSize();

            // 這裡使用的是嚴格小於，確保貼合時不觸發
            bool isColliding = std::abs(marioPos.x - itemPos.x) < (marioSize.x + itemSize.x) / 2.0f &&
                std::abs(marioPos.y - itemPos.y) < (marioSize.y + itemSize.y) / 2.0f;

            if (isColliding) {
                // 只要重疊，就立即觸發收集
                item->OnCollect(m_Mario.get());
                // 注意： OnCollect 應該要將 item 的 m_IsActive 設為 false
            }
            // -------------------------------------------------------------------------

            ++it;
        }
        else {
            // 道具失效 (被吃掉或掉出視窗)，將其移除
            m_Root.RemoveChild(item);
            it = m_Items.erase(it);
        }
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

        // 1. 平移所有地形方塊
        for (auto& block : m_CurrentMapBlocks) {
            glm::vec2 pos = block->GetPosition();
            pos.x -= deltaX;
            block->SetPosition(pos);
        }

        // 2. 新增：同步平移所有道具，抵銷攝影機造成的相對位移
        for (auto& item : m_Items) {
            glm::vec2 pos = item->GetPosition();
            pos.x -= deltaX;
            item->SetPosition(pos);
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