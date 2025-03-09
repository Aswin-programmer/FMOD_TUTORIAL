#include "AudioSystem.h"
#include <iostream>

// Example Game Engine Audio Manager that uses our FMOD wrapper
class GameAudioManager {
public:
    GameAudioManager() = default;
    ~GameAudioManager() {
        m_AudioSystem.Shutdown();
    }

    bool Initialize() {
        if (!m_AudioSystem.Initialize()) {
            std::cerr << "Failed to initialize audio system!" << std::endl;
            return false;
        }

        // Connect to FMOD Studio for live editing if in development mode
#ifdef _DEBUG
        m_AudioSystem.ConnectToStudioRemote();
        std::cout << "Connected to FMOD Studio for live editing." << std::endl;
#endif

        // Load banks
        if (!LoadBanks()) {
            std::cerr << "Failed to load audio banks!" << std::endl;
            return false;
        }

        // Preload common sounds
        PreloadCommonSounds();

        return true;
    }

    void Update(float deltaTime) {
        m_AudioSystem.Update(deltaTime);
    }

    // Example method to play a sound effect
    void PlaySFX(const std::string& sfxName) {
        auto it = m_SoundMap.find(sfxName);
        if (it != m_SoundMap.end()) {
            m_AudioSystem.PlaySound(it->second);
        }
        else {
            // Load sound if not already loaded
            int soundId = m_AudioSystem.LoadSound("Assets/Audio/SFX/" + sfxName + ".wav");
            if (soundId >= 0) {
                m_SoundMap[sfxName] = soundId;
                m_AudioSystem.PlaySound(soundId);
            }
        }
    }

    // Example method to play a music track
    void PlayMusic(const std::string& trackName, bool fadeIn = true) {
        StopMusic(true);  // Stop current music

        auto it = m_MusicMap.find(trackName);
        if (it != m_MusicMap.end()) {
            int eventId = it->second;
            if (fadeIn) {
                m_AudioSystem.SetEventParameter(eventId, "Fade", 0.0f);
                m_AudioSystem.PlayEvent(eventId);
                m_AudioSystem.SetEventParameter(eventId, "Fade", 1.0f);
            }
            else {
                m_AudioSystem.PlayEvent(eventId);
            }
            m_CurrentMusicId = eventId;
        }
        else {
            // Create event instance if not already loaded
            std::string eventPath = "event:/Music/" + trackName;
            int eventId = m_AudioSystem.CreateEventInstance(eventPath);
            if (eventId >= 0) {
                m_MusicMap[trackName] = eventId;
                if (fadeIn) {
                    m_AudioSystem.SetEventParameter(eventId, "Fade", 0.0f);
                    m_AudioSystem.PlayEvent(eventId);
                    m_AudioSystem.SetEventParameter(eventId, "Fade", 1.0f);
                }
                else {
                    m_AudioSystem.PlayEvent(eventId);
                }
                m_CurrentMusicId = eventId;
            }
        }
    }

    // Stop current music
    void StopMusic(bool fadeOut = true) {
        if (m_CurrentMusicId >= 0) {
            if (fadeOut) {
                m_AudioSystem.SetEventParameter(m_CurrentMusicId, "Fade", 0.0f);
                // The event would stop automatically after fade out if set up that way in FMOD Studio
            }
            else {
                m_AudioSystem.StopEvent(m_CurrentMusicId, true);
            }
        }
    }

    // Example method to play an ambience track
    void PlayAmbience(const std::string& ambienceName) {
        auto it = m_AmbienceMap.find(ambienceName);
        if (it != m_AmbienceMap.end()) {
            m_AudioSystem.PlayEvent(it->second);
        }
        else {
            // Create event instance if not already loaded
            std::string eventPath = "event:/Ambience/" + ambienceName;
            int eventId = m_AudioSystem.CreateEventInstance(eventPath);
            if (eventId >= 0) {
                m_AmbienceMap[ambienceName] = eventId;
                m_AudioSystem.PlayEvent(eventId);
            }
        }
    }

    // Example method to trigger a one-shot event
    void TriggerEvent(const std::string& eventName) {
        std::string eventPath = "event:/Events/" + eventName;
        int eventId = m_AudioSystem.CreateEventInstance(eventPath);
        if (eventId >= 0) {
            m_AudioSystem.PlayEvent(eventId);
            // For one-shot events, we don't need to store the ID
        }
    }

    // Set master volume
    void SetMasterVolume(float volume) {
        m_AudioSystem.SetMasterVolume(volume);
    }

    // Set 3D listener position (for camera/player)
    void SetListenerPosition(float x, float y, float z) {
        float pos[3] = { x, y, z };
        float forward[3] = { 0.0f, 0.0f, 1.0f };  // Forward in 2D game is typically Z
        float up[3] = { 0.0f, 1.0f, 0.0f };       // Up is Y

        m_AudioSystem.Set3DListenerPosition(pos, forward, up);
    }

    // Set 3D position for an event
    void SetEventPosition(const std::string& eventName, float x, float y, float z) {
        auto it = m_MusicMap.find(eventName);
        if (it != m_MusicMap.end()) {
            float pos[3] = { x, y, z };
            m_AudioSystem.SetEventPosition(it->second, pos);
        }
    }

private:
    bool LoadBanks() {
        // Load master bank and master strings bank
        if (!m_AudioSystem.LoadBank("Assets/Audio/Banks/Master.bank")) {
            return false;
        }

        if (!m_AudioSystem.LoadBank("Assets/Audio/Banks/Master.strings.bank")) {
            return false;
        }

        // Load other banks
        std::vector<std::string> bankNames = { "Music", "SFX", "Ambience", "Dialogue" };
        for (const auto& bankName : bankNames) {
            if (!m_AudioSystem.LoadBank("Assets/Audio/Banks/" + bankName + ".bank")) {
                std::cerr << "Warning: Failed to load bank: " << bankName << std::endl;
                // Continue loading other banks even if one fails
            }
        }

        return true;
    }

    void PreloadCommonSounds() {
        // Preload common sound effects
        std::vector<std::string> commonSounds = {
            "button_click", "menu_open", "menu_close", "item_pickup", "jump", "land"
        };

        for (const auto& soundName : commonSounds) {
            int soundId = m_AudioSystem.LoadSound("Assets/Audio/SFX/" + soundName + ".wav");
            if (soundId >= 0) {
                m_SoundMap[soundName] = soundId;
            }
        }

        // Preload common events
        std::vector<std::pair<std::string, std::string>> commonEvents = {
            { "main_theme", "event:/Music/MainTheme" },
            { "menu_music", "event:/Music/MenuMusic" },
            { "forest_ambience", "event:/Ambience/Forest" },
            { "city_ambience", "event:/Ambience/City" }
        };

        for (const auto& event : commonEvents) {
            int eventId = m_AudioSystem.CreateEventInstance(event.second);
            if (eventId >= 0) {
                if (event.second.find("Music") != std::string::npos) {
                    m_MusicMap[event.first] = eventId;
                }
                else if (event.second.find("Ambience") != std::string::npos) {
                    m_AmbienceMap[event.first] = eventId;
                }
            }
        }
    }

    AudioSystem m_AudioSystem;

    // Maps for keeping track of loaded sounds and events
    std::unordered_map<std::string, int> m_SoundMap;    // Simple sounds
    std::unordered_map<std::string, int> m_MusicMap;    // Music events
    std::unordered_map<std::string, int> m_AmbienceMap; // Ambience events

    int m_CurrentMusicId = -1;  // Currently playing music
};

// Example of how to use the GameAudioManager in your main game loop
int main() {
    GameAudioManager audioManager;

    // Initialize the audio system
    if (!audioManager.Initialize()) {
        std::cerr << "Failed to initialize audio manager!" << std::endl;
        return -1;
    }

    // Example game loop
    bool running = true;
    float deltaTime = 0.016f;  // ~60 fps

    // Start playing menu music
    audioManager.PlayMusic("menu_music");

    while (running) {
        // Update audio system
        audioManager.Update(deltaTime);

        // Example: Play sound when player jumps
        if (playerJumped) {
            audioManager.PlaySFX("jump");
        }

        // Example: Change music when entering a new area
        if (playerEnteredNewArea) {
            audioManager.PlayMusic("area_theme", true);  // Fade in
        }

        // Example: Update listener position based on player/camera position
        audioManager.SetListenerPosition(playerX, playerY, 0.0f);

        // Rest of game loop...
    }

    return 0;
}