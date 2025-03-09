#include "AudioSystem.h"
#include <iostream>

bool AudioSystem::Initialize() {
    if (m_Initialized) {
        std::cout << "Audio system already initialized!" << std::endl;
        return true;
    }

    // Create the Studio System
    FMOD_RESULT result = FMOD::Studio::System::create(&m_StudioSystem);
    CheckError(result, "FMOD::Studio::System::create");
    if (result != FMOD_OK) return false;

    // Get the Core System from the Studio System
    result = m_StudioSystem->getCoreSystem(&m_CoreSystem);
    CheckError(result, "m_StudioSystem->getCoreSystem");
    if (result != FMOD_OK) return false;

    // Initialize the Studio System
    result = m_StudioSystem->initialize(1024, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    CheckError(result, "m_StudioSystem->initialize");
    if (result != FMOD_OK) return false;

    // Create master channel group
    result = m_CoreSystem->createChannelGroup("Master", &m_MasterChannelGroup);
    CheckError(result, "m_CoreSystem->createChannelGroup");
    if (result != FMOD_OK) return false;

    m_Initialized = true;
    std::cout << "FMOD Audio System initialized successfully." << std::endl;
    return true;
}

void AudioSystem::Shutdown() {
    if (!m_Initialized) return;

    // Unload all banks
    for (auto& bankPair : m_Banks) {
        if (bankPair.second) {
            bankPair.second->unload();
            bankPair.second = nullptr;
        }
    }
    m_Banks.clear();

    // Release all event instances
    for (auto& event : m_Events) {
        if (event.instance) {
            event.instance->release();
            event.instance = nullptr;
        }
    }
    m_Events.clear();

    // Release all sounds
    for (auto& sound : m_Sounds) {
        if (sound.sound) {
            sound.sound->release();
            sound.sound = nullptr;
        }
    }
    m_Sounds.clear();

    // Release channel group
    if (m_MasterChannelGroup) {
        m_MasterChannelGroup->release();
        m_MasterChannelGroup = nullptr;
    }

    // Release Studio System (this will also release the Core System)
    if (m_StudioSystem) {
        m_StudioSystem->release();
        m_StudioSystem = nullptr;
        m_CoreSystem = nullptr;
    }

    m_Initialized = false;
    std::cout << "FMOD Audio System shut down." << std::endl;
}

void AudioSystem::Update(float deltaTime) {
    if (!m_Initialized) return;

    // Update FMOD systems
    if (m_StudioSystem) {
        m_StudioSystem->update();
    }

    // Update playing state of events
    for (auto& event : m_Events) {
        if (event.instance) {
            FMOD_STUDIO_PLAYBACK_STATE state;
            event.instance->getPlaybackState(&state);
            event.isPlaying = (state != FMOD_STUDIO_PLAYBACK_STOPPED);
        }
    }
}

int AudioSystem::LoadSound(const std::string& path, bool isLooping, bool is3D) {
    if (!m_Initialized) return -1;

    SoundInstance newSound;
    newSound.path = path;
    newSound.isLooping = isLooping;

    FMOD_MODE mode = FMOD_DEFAULT;
    if (isLooping) mode |= FMOD_LOOP_NORMAL;
    if (is3D) mode |= FMOD_3D;

    FMOD_RESULT result = m_CoreSystem->createSound(path.c_str(), mode, nullptr, &newSound.sound);
    CheckError(result, "m_CoreSystem->createSound");
    if (result != FMOD_OK) return -1;

    m_Sounds.push_back(newSound);
    return static_cast<int>(m_Sounds.size() - 1);
}

void AudioSystem::PlaySound(int soundId) {
    if (!m_Initialized || soundId < 0 || soundId >= m_Sounds.size()) return;

    auto& sound = m_Sounds[soundId];
    if (sound.sound) {
        FMOD_RESULT result = m_CoreSystem->playSound(sound.sound, m_MasterChannelGroup, false, &sound.channel);
        CheckError(result, "m_CoreSystem->playSound");

        if (result == FMOD_OK && sound.channel) {
            sound.channel->setVolume(sound.volume);
            sound.channel->setPitch(sound.pitch);
        }
    }
}

void AudioSystem::StopSound(int soundId) {
    if (!m_Initialized || soundId < 0 || soundId >= m_Sounds.size()) return;

    auto& sound = m_Sounds[soundId];
    if (sound.channel) {
        sound.channel->stop();
    }
}

void AudioSystem::SetSoundVolume(int soundId, float volume) {
    if (!m_Initialized || soundId < 0 || soundId >= m_Sounds.size()) return;

    auto& sound = m_Sounds[soundId];
    sound.volume = volume;
    if (sound.channel) {
        sound.channel->setVolume(volume);
    }
}

void AudioSystem::SetSoundPitch(int soundId, float pitch) {
    if (!m_Initialized || soundId < 0 || soundId >= m_Sounds.size()) return;

    auto& sound = m_Sounds[soundId];
    sound.pitch = pitch;
    if (sound.channel) {
        sound.channel->setPitch(pitch);
    }
}

void AudioSystem::SetSoundPan(int soundId, float pan) {
    if (!m_Initialized || soundId < 0 || soundId >= m_Sounds.size()) return;

    auto& sound = m_Sounds[soundId];
    if (sound.channel) {
        sound.channel->setPan(pan);
    }
}

bool AudioSystem::LoadBank(const std::string& bankPath, bool loadSamples) {
    if (!m_Initialized) return false;

    // Check if already loaded
    auto it = m_Banks.find(bankPath);
    if (it != m_Banks.end()) {
        return true;  // Already loaded
    }

    FMOD::Studio::Bank* bank = nullptr;
    FMOD_STUDIO_LOAD_BANK_FLAGS flags = loadSamples ? FMOD_STUDIO_LOAD_BANK_NORMAL : FMOD_STUDIO_LOAD_BANK_NONBLOCKING;

    FMOD_RESULT result = m_StudioSystem->loadBankFile(bankPath.c_str(), flags, &bank);
    CheckError(result, "m_StudioSystem->loadBankFile");

    if (result != FMOD_OK || !bank) {
        std::cerr << "Failed to load bank: " << bankPath << std::endl;
        return false;
    }

    m_Banks[bankPath] = bank;

    // If we're not loading samples immediately, we need to check bank load status in Update
    if (!loadSamples) {
        std::cout << "Bank loading samples asynchronously: " << bankPath << std::endl;
    }

    return true;
}

void AudioSystem::UnloadBank(const std::string& bankPath) {
    if (!m_Initialized) return;

    auto it = m_Banks.find(bankPath);
    if (it != m_Banks.end()) {
        if (it->second) {
            it->second->unload();
        }
        m_Banks.erase(it);
    }
}

int AudioSystem::CreateEventInstance(const std::string& eventPath) {
    if (!m_Initialized) return -1;

    FMOD::Studio::EventDescription* eventDescription = nullptr;
    FMOD_RESULT result = m_StudioSystem->getEvent(eventPath.c_str(), &eventDescription);
    CheckError(result, "m_StudioSystem->getEvent");

    if (result != FMOD_OK || !eventDescription) {
        std::cerr << "Failed to find event: " << eventPath << std::endl;
        return -1;
    }

    FMOD::Studio::EventInstance* eventInstance = nullptr;
    result = eventDescription->createInstance(&eventInstance);
    CheckError(result, "eventDescription->createInstance");

    if (result != FMOD_OK || !eventInstance) {
        std::cerr << "Failed to create instance for event: " << eventPath << std::endl;
        return -1;
    }

    EventInstance newEvent;
    newEvent.instance = eventInstance;
    newEvent.path = eventPath;

    m_Events.push_back(newEvent);
    return static_cast<int>(m_Events.size() - 1);
}

void AudioSystem::PlayEvent(int eventId) {
    if (!m_Initialized || eventId < 0 || eventId >= m_Events.size()) return;

    auto& event = m_Events[eventId];
    if (event.instance) {
        event.instance->start();
        event.isPlaying = true;
    }
}

void AudioSystem::StopEvent(int eventId, bool immediate) {
    if (!m_Initialized || eventId < 0 || eventId >= m_Events.size()) return;

    auto& event = m_Events[eventId];
    if (event.instance) {
        FMOD_STUDIO_STOP_MODE mode = immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
        event.instance->stop(mode);
    }
}

void AudioSystem::SetEventParameter(int eventId, const std::string& paramName, float value) {
    if (!m_Initialized || eventId < 0 || eventId >= m_Events.size()) return;

    auto& event = m_Events[eventId];
    if (event.instance) {
        event.instance->setParameterByName(paramName.c_str(), value);
    }
}

float AudioSystem::GetEventParameter(int eventId, const std::string& paramName) {
    if (!m_Initialized || eventId < 0 || eventId >= m_Events.size()) return 0.0f;

    float value = 0.0f;
    auto& event = m_Events[eventId];
    if (event.instance) {
        event.instance->getParameterByName(paramName.c_str(), &value);
    }
    return value;
}

void AudioSystem::Set3DListenerPosition(const float* position, const float* forward, const float* up) {
    if (!m_Initialized || !m_CoreSystem) return;

    FMOD_VECTOR pos = { position[0], position[1], position[2] };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };  // Velocity could be calculated from previous position if needed
    FMOD_VECTOR fwd = { forward[0], forward[1], forward[2] };
    FMOD_VECTOR up_vec = { up[0], up[1], up[2] };

    m_CoreSystem->set3DListenerAttributes(0, &pos, &vel, &fwd, &up_vec);
}

void AudioSystem::SetEventPosition(int eventId, const float* position) {
    if (!m_Initialized || eventId < 0 || eventId >= m_Events.size()) return;

    auto& event = m_Events[eventId];
    if (event.instance) {
        FMOD_3D_ATTRIBUTES attributes;
        event.instance->get3DAttributes(&attributes);

        attributes.position = { position[0], position[1], position[2] };
        event.instance->set3DAttributes(&attributes);
    }
}

void AudioSystem::SetSoundPosition(int soundId, const float* position) {
    if (!m_Initialized || soundId < 0 || soundId >= m_Sounds.size()) return;

    auto& sound = m_Sounds[soundId];
    if (sound.channel) {
        FMOD_VECTOR pos = { position[0], position[1], position[2] };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };  // Velocity could be calculated if needed

        sound.channel->set3DAttributes(&pos, &vel);
    }
}

void AudioSystem::SetMasterVolume(float volume) {
    if (!m_Initialized || !m_MasterChannelGroup) return;

    m_MasterVolume = volume;
    m_MasterChannelGroup->setVolume(volume);
}

float AudioSystem::GetMasterVolume() const {
    return m_MasterVolume;
}

void AudioSystem::PauseAllSounds(bool pause) {
    if (!m_Initialized || !m_MasterChannelGroup) return;

    m_MasterChannelGroup->setPaused(pause);
}

bool AudioSystem::ConnectToStudioRemote(const char* host, int port) {
    std::cout << "Remote connection to FMOD Studio not implemented in this version." << std::endl;
    std::cout << "Start FMOD Studio with the same project loaded for live updates." << std::endl;
    return false;
}

void AudioSystem::DisconnectFromStudioRemote() {
    // Do nothing
}

bool AudioSystem::IsConnectedToStudioRemote() const {
    return false;
}

int AudioSystem::GetLoadedSoundsCount() const {
    return static_cast<int>(m_Sounds.size());
}

int AudioSystem::GetPlayingSoundsCount() const {
    int count = 0;

    if (m_Initialized && m_CoreSystem) {
        m_CoreSystem->getChannelsPlaying(&count, nullptr);
    }

    return count;
}

int AudioSystem::GetLoadedEventsCount() const {
    return static_cast<int>(m_Events.size());
}

int AudioSystem::GetPlayingEventsCount() const {
    int count = 0;

    for (const auto& event : m_Events) {
        if (event.isPlaying) count++;
    }

    return count;
}

void AudioSystem::CheckError(FMOD_RESULT result, const char* function) {
    if (result != FMOD_OK) {
        std::cerr << "FMOD Error (" << function << "): " << FMOD_ErrorString(result) << std::endl;
    }
}