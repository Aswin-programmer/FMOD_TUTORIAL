// FMOD Integration for 2D Game Engine
// This implementation provides a wrapper around FMOD Core and Studio APIs

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

// FMOD includes
#include "fmod_studio.hpp"
#include "fmod.hpp"
#include "fmod_errors.h"

class AudioSystem {
public:
    struct SoundInstance {
        FMOD::Sound* sound = nullptr;
        FMOD::Channel* channel = nullptr;
        std::string path;
        bool isLooping = false;
        float volume = 1.0f;
        float pitch = 1.0f;
    };

    struct EventInstance {
        FMOD::Studio::EventInstance* instance = nullptr;
        std::string path;
        bool isPlaying = false;
    };

    // Initialize and shutdown methods
    bool Initialize();
    void Shutdown();

    // Update method to be called every frame
    void Update(float deltaTime);

    // Core API methods (for simple sounds)
    int LoadSound(const std::string& path, bool isLooping = false, bool is3D = false);
    void PlaySound(int soundId);
    void StopSound(int soundId);
    void SetSoundVolume(int soundId, float volume);
    void SetSoundPitch(int soundId, float pitch);
    void SetSoundPan(int soundId, float pan);

    // Studio API methods (for events)
    bool LoadBank(const std::string& bankPath, bool loadSamples = true);
    void UnloadBank(const std::string& bankPath);
    int CreateEventInstance(const std::string& eventPath);
    void PlayEvent(int eventId);
    void StopEvent(int eventId, bool immediate = false);
    void SetEventParameter(int eventId, const std::string& paramName, float value);
    float GetEventParameter(int eventId, const std::string& paramName);

    // 3D positioning methods
    void Set3DListenerPosition(const float* position, const float* forward, const float* up);
    void SetEventPosition(int eventId, const float* position);
    void SetSoundPosition(int soundId, const float* position);

    // Global settings
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    void PauseAllSounds(bool pause);

    // Runtime editing support
    bool ConnectToStudioRemote(const char* host = "127.0.0.1", int port = 9264);
    void DisconnectFromStudioRemote();
    bool IsConnectedToStudioRemote() const;

    // System information
    int GetLoadedSoundsCount() const;
    int GetPlayingSoundsCount() const;
    int GetLoadedEventsCount() const;
    int GetPlayingEventsCount() const;

private:
    // FMOD system instances
    FMOD::System* m_CoreSystem = nullptr;
    FMOD::Studio::System* m_StudioSystem = nullptr;

    // Master channel group
    FMOD::ChannelGroup* m_MasterChannelGroup = nullptr;

    // Collections
    std::vector<SoundInstance> m_Sounds;
    std::vector<EventInstance> m_Events;
    std::unordered_map<std::string, FMOD::Studio::Bank*> m_Banks;

    // Internal methods
    void CheckError(FMOD_RESULT result, const char* function);
    void ReleaseResources();

    // System state
    bool m_Initialized = false;
    float m_MasterVolume = 1.0f;
    bool m_IsRemoteConnected = false;
};