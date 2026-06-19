#ifndef SFX_MANAGER_HPP
#define SFX_MANAGER_HPP

#include "Util/SFX.hpp"
#include <memory>
#include <string>
#include <unordered_map>

// Central, cached sound-effect player. Each effect maps to a wav under
// Resources/SFX and is loaded once. Events whose asset is missing are still
// registered, but Util::SFX stores a null chunk for them and Play() becomes a
// silent no-op (Mix_PlayChannel ignores a null chunk) — so "designated events
// without a matching sound asset yet" are bypassed automatically.
class SFXManager {
public:
    enum class Sound {
        Coin,
        Stomp,
        Kick,
        Bump,
        BreakBlock,
        Fireball,
        PowerUp,
        PowerUpAppears,
        OneUp,
        JumpSmall,
        JumpSuper,
        Flagpole,
        MarioDie,
        GameOver,
        Pipe,
    };

    static SFXManager& GetInstance() {
        static SFXManager instance;
        return instance;
    }

    void Play(Sound s) {
        auto it = m_Sounds.find(s);
        if (it != m_Sounds.end() && it->second) {
            it->second->Play();
        }
    }

private:
    SFXManager() {
        Load(Sound::Coin,           "smb_coin.wav");
        Load(Sound::Stomp,          "smb_stomp.wav");
        Load(Sound::Kick,           "smb_kick.wav");
        Load(Sound::Bump,           "smb_bump.wav");
        Load(Sound::BreakBlock,     "smb_breakblock.wav");
        Load(Sound::Fireball,       "smb_fireball.wav");
        Load(Sound::PowerUp,        "smb_powerup.wav");
        Load(Sound::PowerUpAppears, "smb_powerup_appears.wav");
        Load(Sound::OneUp,          "smb_1-up.wav");
        Load(Sound::JumpSmall,      "smb_jump-small.wav");
        Load(Sound::JumpSuper,      "smb_jump-super.wav");
        Load(Sound::Flagpole,       "smb_flagpole.wav");
        Load(Sound::MarioDie,       "smb_mariodie.wav");
        Load(Sound::GameOver,       "smb_gameover.wav");
        Load(Sound::Pipe,           "smb_pipe.wav");
    }

    void Load(Sound s, const std::string& file) {
        m_Sounds[s] = std::make_shared<Util::SFX>(std::string(RESOURCE_DIR "/SFX/") + file);
    }

    std::unordered_map<Sound, std::shared_ptr<Util::SFX>> m_Sounds;
};

#endif // SFX_MANAGER_HPP
