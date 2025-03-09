#include "AudioEngine.h"

int main()
{
    // Initialize
    AudioEngine::Get().Initialize();

    // Load SFX bank (created in FMOD Studio)
    AudioEngine::Get().LoadBank("C:/Aswin_Game_DEV/OPENGL_TUTORIAL/FMOD/FMOD_TUTORIAL/EXTERNAL/SOUNDS/SFX.bank");

    // Create explosion event (from FMOD Studio project)
    auto explosionEvent = AudioEngine::Get().CreateEventInstance("event:/Explosion");

    // Play with parameters
    AudioEngine::Get().SetEventParameter(explosionEvent, "Size", 1.5f);
    //AudioEngine::Get().PlayEvent(explosionEvent);
    AudioEngine::Get().PlayEvent(explosionEvent);

    // In game loop
    while (1) {
        AudioEngine::Get().Update();
        // ...
    }

    // Cleanup
    AudioEngine::Get().Shutdown();
}

