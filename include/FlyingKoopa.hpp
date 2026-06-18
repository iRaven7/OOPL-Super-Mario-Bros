#ifndef FLYING_KOOPA_HPP
#define FLYING_KOOPA_HPP

#include "Koopa.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <cmath>

// A winged Koopa (Paratroopa). While it has its wings it hovers in place,
// drifting up and down. Stomping it knocks the wings off — it then drops and
// behaves exactly like an ordinary walking Koopa (which can in turn be stomped
// into a shell). A fireball kills it outright, wings and all.
class FlyingKoopa : public Koopa {
public:
    FlyingKoopa(glm::vec2 startPos) : Koopa(startPos) {
        // The base Koopa ctor already lifted startPos.y; use that as the centre
        // of the up/down hover.
        m_HoverCenterY = GetPosition().y;

        m_WingFrame1 = std::make_shared<Util::Image>(RESOURCE_DIR"/Entities/Koopa/wings1.png");
        m_WingFrame2 = std::make_shared<Util::Image>(RESOURCE_DIR"/Entities/Koopa/wings2.png");

        m_Wing = std::make_shared<Util::GameObject>();
        m_Wing->SetDrawable(m_WingFrame1);
        m_Wing->SetZIndex(36.0f);   // in front of the koopa body (35) so wings cover it
        AddChild(m_Wing);
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        if (m_HasWings) {
            m_BobTimer  += deltaTime;
            m_FlapTimer += deltaTime;

            // Hover in place, oscillating up and down around the spawn height.
            float offset = std::sin(m_BobTimer * m_BobSpeed) * m_BobAmplitude;
            SetPosition({ GetPosition().x, m_HoverCenterY + offset });

            // Flap the wings (body keeps its static koopa1.png sprite).
            int flap = static_cast<int>(m_FlapTimer * 10.0f) % 2;
            m_Wing->SetDrawable(flap == 0 ? m_WingFrame1 : m_WingFrame2);
            return;
        }

        // Wings lost — fall and walk like a normal Koopa.
        Koopa::UpdateAI(deltaTime, blocks);
    }

    void OnStomped(Character* hitter) override {
        if (!m_IsActive) return;

        if (m_HasWings) {
            // First stomp only strips the wings; Mario's bounce is applied by
            // the collision manager. The koopa drops from its hover height and
            // starts walking on the next frame.
            LoseWings();
            return;
        }
        Koopa::OnStomped(hitter);
    }

    void OnFireballHit() override {
        if (m_Wing) m_Wing->SetVisible(false);
        m_HasWings = false;
        Koopa::OnFireballHit();
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        Koopa::UpdateRenderPosition(cameraX, cameraZoom);

        if (m_HasWings && m_Wing) {
            // Wings ride on the koopa's back, slightly above centre, mirrored to
            // match the body's facing direction.
            float dir   = (m_Velocity.x > 0.0f) ? -1.0f : 1.0f;
            float wingX = GetPosition().x + 8.0f;
            float wingY = GetPosition().y + 6.0f;
            m_Wing->m_Transform.translation = { (wingX - cameraX) * cameraZoom, wingY * cameraZoom };
            m_Wing->m_Transform.scale       = { cameraZoom * dir, cameraZoom };
        }
    }

private:
    void LoseWings() {
        m_HasWings = false;
        if (m_Wing) m_Wing->SetVisible(false);
        m_Velocity.y = 0.0f;   // start the drop cleanly; gravity takes over
    }

    std::shared_ptr<Util::GameObject> m_Wing;
    std::shared_ptr<Util::Image>      m_WingFrame1;
    std::shared_ptr<Util::Image>      m_WingFrame2;

    bool  m_HasWings     = true;
    float m_BobTimer     = 0.0f;
    float m_FlapTimer    = 0.0f;
    float m_BobSpeed     = 3.0f;    // radians / sec
    float m_BobAmplitude = 40.0f;   // px above/below the hover centre
    float m_HoverCenterY = 0.0f;
};

#endif // FLYING_KOOPA_HPP
