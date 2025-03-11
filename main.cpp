#include "AudioEngine.h"

int main()
{
    // Initialize
    AudioEngine::Get().Initialize();

    // Load SFX bank (created in FMOD Studio)
    AudioEngine::Get().LoadBank("EXTERNAL/SOUNDS/sample/Build/Desktop/SFX.bank");

    // Create explosion event (from FMOD Studio project)
    auto explosionEvent = AudioEngine::Get().CreateEventInstance("event:/Explosion");
    auto explosionEvent2 = AudioEngine::Get().CreateEventInstance("event:/Explosion2");

    // Play with parameters
    AudioEngine::Get().SetEventParameter(explosionEvent, "Size", 1.5f);
    //AudioEngine::Get().PlayEvent(explosionEvent);
    //AudioEngine::Get().PlayEvent(explosionEvent);
    AudioEngine::Get().PlayEvent(explosionEvent2);

    // In game loop
    while (1) {
        AudioEngine::Get().Update();
        // ...
    }

    // Cleanup
    AudioEngine::Get().Shutdown();
}

