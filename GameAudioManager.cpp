// GameAudioManager.cpp
#include "GameAudioManager.h"
#include <filesystem>
#include <iostream>

GameAudioManager& GameAudioManager::GetInstance()
{
    static GameAudioManager instance;
    return instance;
}

bool GameAudioManager::Initialize()
{
    return FMODAudioSystem::GetInstance().Initialize();
}

void GameAudioManager::Shutdown()
{
    // Stop and clear all active events
    m_ActiveEvents.clear();
    m_CurrentMusicTrack = nullptr;

    // Shutdown FMOD
    FMODAudioSystem::GetInstance().Shutdown();
}

bool GameAudioManager::LoadBanks(const std::string& banksFolder)
{
    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(banksFolder))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().filename().string();
                std::string extension = entry.path().extension().string();

                // Only load .bank files
                if (extension == ".bank")
                {
                    std::string bankName = entry.path().stem().string();
                    std::string bankPath = entry.path().string();

                    if (!LoadBank(bankName, bankPath))
                    {
                        std::cerr << "Failed to load bank: " << bankName << std::endl;
                    }
                }
            }
        }
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading banks: " << e.what() << std::endl;
        return false;
    }
}

bool GameAudioManager::LoadBank(const std::string& bankName, const std::string& bankPath)
{
    return FMODAudioSystem::GetInstance().LoadBank(bankName, bankPath);
}

std::shared_ptr<AudioEvent> GameAudioManager::CreateEvent(const std::string& eventPath)
{
    auto event = std::make_shared<AudioEvent>(eventPath);
    if (event->IsValid())
    {
        m_ActiveEvents.push_back(event);
        return event;
    }

    return nullptr;
}

bool GameAudioManager::PlayOneShot(const std::string& eventPath, float x, float y)
{
    FMOD_VECTOR position = { x, y, 0 };
    return FMODAudioSystem::GetInstance().PlayOneShot(eventPath, position);
}

void GameAudioManager::SetListenerPosition(float x, float y)
{
    FMODAudioSystem::GetInstance().Set3DListenerPosition(x, y);
}

bool GameAudioManager::SetGlobalParameter(const std::string& name, float value)
{
    return FMODAudioSystem::GetInstance().SetGlobalParameter(name, value);
}

float GameAudioManager::GetGlobalParameter(const std::string& name)
{
    return FMODAudioSystem::GetInstance().GetGlobalParameter(name);
}

bool GameAudioManager::SetBusVolume(const std::string& busPath, float volume)
{
    return FMODAudioSystem::GetInstance().SetBusVolume(busPath, volume);
}

bool GameAudioManager::SetVCAVolume(const std::string& vcaPath, float volume)
{
    return FMODAudioSystem::GetInstance().SetVCAVolume(vcaPath, volume);
}

bool GameAudioManager::StartSnapshot(const std::string& snapshotPath)
{
    return FMODAudioSystem::GetInstance().StartSnapshot(snapshotPath);
}

bool GameAudioManager::StopSnapshot(const std::string& snapshotPath)
{
    return FMODAudioSystem::GetInstance().StopSnapshot(snapshotPath);
}

std::shared_ptr<AudioEvent> GameAudioManager::PlayMusicTrack(const std::string& musicEventPath)
{
    // Stop current music first
    StopAllMusic();

    // Create and play the new music track
    m_CurrentMusicTrack = CreateEvent(musicEventPath);
    if (m_CurrentMusicTrack)
    {
        m_CurrentMusicTrack->Play();
    }

    return m_CurrentMusicTrack;
}

bool GameAudioManager::StopAllMusic(bool allowFadeOut)
{
    if (m_CurrentMusicTrack)
    {
        bool result = m_CurrentMusicTrack->Stop(allowFadeOut);
        m_CurrentMusicTrack = nullptr;
        return result;
    }

    return true;
}

void GameAudioManager::Update(float deltaTime)
{
    // Accumulate delta time
    m_TimeSinceLastUpdate += deltaTime;

    // Update FMOD at fixed intervals (e.g., 60Hz)
    const float updateInterval = 1.0f / 60.0f;
    if (m_TimeSinceLastUpdate >= updateInterval)
    {
        // Update FMOD
        FMODAudioSystem::GetInstance().Update();

        // Clean up finished events
        CleanupEvents();

        // Reset timer
        m_TimeSinceLastUpdate = 0.0f;
    }
}

void GameAudioManager::CleanupEvents()
{
    // Remove any events that are no longer active or valid
    m_ActiveEvents.erase(
        std::remove_if(m_ActiveEvents.begin(), m_ActiveEvents.end(),
            [](const std::shared_ptr<AudioEvent>& event) {
                return !event->IsValid() || !event->IsPlaying();
            }),
        m_ActiveEvents.end()
    );

    // Check if music track is still playing
    if (m_CurrentMusicTrack && (!m_CurrentMusicTrack->IsValid() || !m_CurrentMusicTrack->IsPlaying()))
    {
        m_CurrentMusicTrack = nullptr;
    }
}