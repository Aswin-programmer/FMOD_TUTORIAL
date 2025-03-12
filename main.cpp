// Example engine integration (in your game loop)
/*
int main()
{
    // Initialize audio
    if (!GameAudioManager::GetInstance().Initialize())
    {
        std::cerr << "Failed to initialize audio system!" << std::endl;
        return 1;
    }

    // Load banks
    GameAudioManager::GetInstance().LoadBanks("./FMODBanks");

    // Game loop
    bool running = true;
    float deltaTime = 0.016f; // 60 FPS

    while (running)
    {
        // Update game logic
        // ...

        // Update audio
        GameAudioManager::GetInstance().Update(deltaTime);

        // Example: Play a sound when a key is pressed
        if (keyPressed)
        {
            GameAudioManager::GetInstance().PlayOneShot("event:/SFX/PlayerJump", playerX, playerY);
        }

        // Example: Update listener position to follow camera
        GameAudioManager::GetInstance().SetListenerPosition(cameraX, cameraY);

        // Example: Set a parameter based on game state
        GameAudioManager::GetInstance().SetGlobalParameter("GameIntensity", currentIntensity);
    }

    // Shutdown audio
    GameAudioManager::GetInstance().Shutdown();

    return 0;
}
*/

#include <iostream>        // For cout/cerr output
#include "GameAudioManager.h" // Your audio manager class
#include <thread>
#include <chrono>

int main()
{
    // Initialize audio
    if (!GameAudioManager::GetInstance().Initialize())
    {
        std::cerr << "Failed to initialize audio system!" << std::endl;
        return 1;
    }

    // Load banks and check result
    bool banksLoaded = GameAudioManager::GetInstance().LoadBanks("C:/Aswin_Game_DEV/OPENGL_TUTORIAL/FMOD/FMOD_TUTORIAL/EXTERNAL/SOUNDS/sounds/Build/Desktop");
    if (!banksLoaded) {
        std::cerr << "Failed to load audio banks!" << std::endl;
        return 1;
    }

    // Try playing the event, check result
   // Example variables for positions
    float playerX = 30; // Player's x position
    float playerY = 0;  // Player's y position
                    
    float sourceX = 0; // Sound source x position
    float sourceY = 0;  // Sound source y position

    // 1. Set the listener's position (e.g., player's position)
    GameAudioManager::GetInstance().SetListenerPosition(playerX, playerY);

    // 2. Create an AudioEvent for the spatialized sound
    std::shared_ptr<AudioEvent> spatialEvent = GameAudioManager::GetInstance().CreateEvent("event:/MUSIC/TRAFF");
    // In your main function
    if (spatialEvent)
    {
        std::cout << "Event created successfully" << std::endl;

        // Check if event is valid after setting position
        spatialEvent->SetPosition(sourceX, sourceY);
        std::cout << "Is event valid after setting position: " << (spatialEvent->IsValid() ? "Yes" : "No") << std::endl;

        // Play and check result
        bool playResult = spatialEvent->Play();
        std::cout << "Play result: " << (playResult ? "Success" : "Failed") << std::endl;
        std::cout << "Is event playing: " << (spatialEvent->IsPlaying() ? "Yes" : "No") << std::endl;
    }



    //auto musicTrack = GameAudioManager::GetInstance().PlayOneShot("event:/MUSIC/TRAFFICMUSIC",210,50);

    // Game loop
    bool running = true;
    float deltaTime = 0.016f; // 60 FPS


    // Update your game loop to include a way to exit and allow time for audio to play
    int loopCount = 0;
    const int maxLoops = 3000; // Run for a few seconds at 60 FPS

    while (running && loopCount < maxLoops)
    {
        // Update FMOD system
        GameAudioManager::GetInstance().Update(deltaTime);

        // Optionally update sound position to test
        if (loopCount % 60 == 0) { // every second
            sourceX += 5.0f; // Move sound 5 units to the right
            spatialEvent->SetPosition(sourceX, sourceY);
            std::cout << "Sound position: " << sourceX << ", " << sourceY << std::endl;
        }

        // Add a small sleep to avoid maxing CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
        loopCount++;
    }

    // Shutdown audio
    GameAudioManager::GetInstance().Shutdown();
    return 0;
}

