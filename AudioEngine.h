#pragma once
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <fmod_errors.h>  // Required for FMOD_ErrorString

#define FMOD_CHECK(result) \
    if (result != FMOD_OK) { \
        const char* errorStr = FMOD_ErrorString(result); \
        throw std::runtime_error("FMOD error: " + std::string(errorStr)); \
    }

class AudioEngine {
public:
    static AudioEngine& Get() {
        static AudioEngine instance;
        return instance;
    }

    // Lifecycle
    void Initialize();
    void Shutdown();
    void Update(float deltaTime = 0.0f);

    // Bank Management
    void LoadBank(const std::string& bankName);
    void UnloadBank(const std::string& bankName);

    // Event Management
    FMOD::Studio::EventInstance* CreateEventInstance(const std::string& eventPath);
    void PlayEvent(FMOD::Studio::EventInstance* instance);
    void StopEvent(FMOD::Studio::EventInstance* instance, bool immediate = false);
    void SetEventParameter(FMOD::Studio::EventInstance* instance, const std::string& paramName, float value);

    // Listener Setup (for 2D/3D positioning)
    void SetListenerAttributes(int listenerId, const FMOD_3D_ATTRIBUTES& attributes);

private:
    FMOD::Studio::System* m_studioSystem = nullptr;
    FMOD::System* m_coreSystem = nullptr;
    std::unordered_map<std::string, FMOD::Studio::Bank*> m_loadedBanks;
    std::vector<FMOD::Studio::EventInstance*> m_activeEvents;

    AudioEngine() = default;
    ~AudioEngine() = default;
    void LoadMasterBanks();
};