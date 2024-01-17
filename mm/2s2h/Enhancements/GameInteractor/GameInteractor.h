#ifndef GAME_INTERACTOR_H
#define GAME_INTERACTOR_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include <vector>
#include <functional>

#define DEFINE_HOOK(name, type)         \
    struct name {                       \
        typedef std::function<type> fn; \
    }

class GameInteractor {
public:
    static GameInteractor* Instance;

    // Game State
    class State {

    };

    // Game Hooks
    template <typename H> struct RegisteredGameHooks { inline static std::vector<typename H::fn> functions; };
    template <typename H> void RegisterGameHook(typename H::fn h) { RegisteredGameHooks<H>::functions.push_back(h); }
    template <typename H, typename... Args> void ExecuteHooks(Args&&... args) {
        for (auto& fn : RegisteredGameHooks<H>::functions) {
            fn(std::forward<Args>(args)...);
        }
    }

    DEFINE_HOOK(OnGameStateMainFinish, void());
    DEFINE_HOOK(OnGameStateDrawFinish, void());
    DEFINE_HOOK(OnGameStateUpdate, void());

    // Game Actions
    class RawAction {

    };

};

#endif // __cplusplus

#endif // GAME_INTERACTOR_H