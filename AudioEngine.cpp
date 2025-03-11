#include "AudioEngine.h"
#include <iostream>

void AudioEngine::Initialize() {
    FMOD_RESULT result;

    // Create Studio System
    result = FMOD::Studio::System::create(&m_studioSystem);
    FMOD_CHECK(result);

    // Initialize with live update support
    result = m_studioSystem->initialize(512, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, nullptr);
    FMOD_CHECK(result);

    // Get core system for low-level operations
    result = m_studioSystem->getCoreSystem(&m_coreSystem);
    FMOD_CHECK(result);

    // Load essential banks
    LoadMasterBanks();
}

void AudioEngine::Shutdown() {
    // Release all events first
    for (auto& event : m_activeEvents) {
        event->stop(FMOD_STUDIO_STOP_IMMEDIATE);
        event->release();
    }
    m_activeEvents.clear();

    // Unload banks
    for (auto& [name, bank] : m_loadedBanks) {
        bank->unload();
    }
    m_loadedBanks.clear();

    // Shutdown systems
    if (m_studioSystem) {
        m_studioSystem->release();
        m_studioSystem = nullptr;
    }
}

void AudioEngine::Update(float deltaTime) {
    if (m_studioSystem) {
        m_studioSystem->update();
    }
}

void AudioEngine::LoadMasterBanks() {
    LoadBank("EXTERNAL/SOUNDS/sample/Build/Desktop/Master.bank");
    LoadBank("EXTERNAL/SOUNDS/sample/Build/Desktop/Master.strings.bank");  // Required for event path resolution
    LoadBank("EXTERNAL/SOUNDS/sample/Build/Desktop/SFX.bank");  // Required for event path resolution
}

void AudioEngine::LoadBank(const std::string& bankName) {
    if (m_loadedBanks.find(bankName) != m_loadedBanks.end()) return;

    FMOD::Studio::Bank* bank = nullptr;
    FMOD_RESULT result = m_studioSystem->loadBankFile(
        bankName.c_str(),
        FMOD_STUDIO_LOAD_BANK_NORMAL,
        &bank
    );
    FMOD_CHECK(result);

    if (bank) {
        m_loadedBanks[bankName] = bank;

        // Load sample data if needed
        bank->loadSampleData();
    }
}

FMOD::Studio::EventInstance* AudioEngine::CreateEventInstance(const std::string& eventPath) {
    FMOD::Studio::EventDescription* eventDesc = nullptr;
    FMOD_RESULT result = m_studioSystem->getEvent(eventPath.c_str(), &eventDesc);

    // Add error logging before checking result
    if (result != FMOD_OK) {
        const char* errorStr = FMOD_ErrorString(result);
        std::cout << "Failed to get event '" << eventPath << "': " << errorStr << std::endl;

        // Let's check if banks are loaded correctly
        int bankCount = 0;
        m_studioSystem->getBankCount(&bankCount);
        std::cout << "Loaded bank count: " << bankCount << std::endl;

        // Try to list all events to see what's available
        FMOD::Studio::Bank** banks = new FMOD::Studio::Bank * [bankCount];
        m_studioSystem->getBankList(banks, bankCount, &bankCount);

        for (int i = 0; i < bankCount; i++) {
            int eventCount = 0;
            banks[i]->getEventCount(&eventCount);
            std::cout << "Bank " << i << " has " << eventCount << " events" << std::endl;

            if (eventCount > 0) {
                FMOD::Studio::EventDescription** events = new FMOD::Studio::EventDescription * [eventCount];
                banks[i]->getEventList(events, eventCount, &eventCount);

                for (int j = 0; j < eventCount; j++) {
                    char path[256];
                    events[j]->getPath(path, 256, nullptr);
                    std::cout << "  Event: " << path << std::endl;
                }

                delete[] events;
            }
        }

        delete[] banks;
    }

    FMOD_CHECK(result);

    FMOD::Studio::EventInstance* instance = nullptr;
    result = eventDesc->createInstance(&instance);
    FMOD_CHECK(result);
    m_activeEvents.push_back(instance);
    return instance;
}

void AudioEngine::PlayEvent(FMOD::Studio::EventInstance* instance) {
    if (instance) {
        instance->start();
    }
}

void AudioEngine::StopEvent(FMOD::Studio::EventInstance* instance, bool immediate) {
    if (instance) {
        FMOD_STUDIO_STOP_MODE mode = immediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
        instance->stop(mode);
        instance->release();

        // Remove from active events
        auto it = std::find(m_activeEvents.begin(), m_activeEvents.end(), instance);
        if (it != m_activeEvents.end()) {
            m_activeEvents.erase(it);
        }
    }
}

void AudioEngine::SetEventParameter(FMOD::Studio::EventInstance* instance, const std::string& paramName, float value) {
    if (instance) {
        instance->setParameterByName(paramName.c_str(), value);
    }
}

void AudioEngine::SetListenerAttributes(int listenerId, const FMOD_3D_ATTRIBUTES& attributes) {
    m_studioSystem->setListenerAttributes(listenerId, &attributes);
}