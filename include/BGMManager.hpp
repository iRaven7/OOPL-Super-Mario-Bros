#ifndef BGM_MANAGER_HPP
#define BGM_MANAGER_HPP

#include "Util/BGM.hpp"
#include <SDL_mixer.h>
#include <memory>
#include <string>

// Owns the single SDL_mixer music channel. Level themes loop forever; the
// stage-clear jingle is a one-shot that takes over the music channel (which
// inherently stops the level theme). Mario death and flag-grab stop the music
// outright.
class BGMManager {
public:
    enum class Track { Ground, Underground };

    static BGMManager& GetInstance() {
        static BGMManager instance;
        return instance;
    }

    // Start (and loop) a level theme. If the requested theme is already the one
    // playing, leave it running so pipe trips between two same-theme maps don't
    // restart the music mid-stride.
    void PlayLevel(Track t) {
        if (m_Current == t && Mix_PlayingMusic()) return;
        m_Current = t;
        Get(t).Play(-1);   // -1 = loop indefinitely
    }

    // Course-clear fanfare, once. It plays on the music channel, so it also
    // stops whatever level theme was playing. Poll IsPlaying() to know when the
    // fanfare (and therefore the level-end hold) is done.
    void PlayStageClear() {
        if (m_StageClear) m_StageClear->Play(0);   // 0 = play through once
    }

    bool IsPlaying() const { return Mix_PlayingMusic() != 0; }

    void Stop() { Mix_HaltMusic(); }

private:
    BGMManager() {
        m_Ground      = std::make_unique<Util::BGM>(std::string(RESOURCE_DIR "/SFX/01. Ground Theme.mp3"));
        m_Underground = std::make_unique<Util::BGM>(std::string(RESOURCE_DIR "/SFX/02. Underground Theme.mp3"));
        m_StageClear  = std::make_unique<Util::BGM>(std::string(RESOURCE_DIR "/SFX/smb_stage_clear.wav"));
    }

    Util::BGM& Get(Track t) { return (t == Track::Ground) ? *m_Ground : *m_Underground; }

    std::unique_ptr<Util::BGM> m_Ground;
    std::unique_ptr<Util::BGM> m_Underground;
    std::unique_ptr<Util::BGM> m_StageClear;
    Track m_Current = Track::Ground;
};

#endif // BGM_MANAGER_HPP
