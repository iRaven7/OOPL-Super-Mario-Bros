// Flagpole.hpp (草稿)
#include "Util/GameObject.hpp"

class Flagpole : public Util::GameObject {
public:
    Flagpole(float x, float y) {
        // 初始化你的旗桿和旗幟圖片
        // 旗桿可能很高，是由多個 Block 組成，或是單一張長圖

        m_Transform.translation = { x, y };

        // 假設 m_Flag 是另一個 GameObject 裝載 flag.png
        // m_Flag.translation = {x - 20, y + 200}; (旗幟掛在頂端左側)
    }

    void SlideFlagDown(float deltaTime) {
        if (!m_IsFlagDown) {
            // 讓旗幟的 Y 座標跟著下降
            // if (m_Flag.translation.y > m_BottomY) {
            //      m_Flag.translation.y -= 200.0f * deltaTime;
            // } else { m_IsFlagDown = true; }
        }
    }

    void SetTriggered(bool triggered) { m_IsTriggered = triggered; }
    bool IsTriggered() const { return m_IsTriggered; }

private:
    bool m_IsTriggered = false;
    bool m_IsFlagDown = false;
    // ... 其他圖片與座標成員 ...
};