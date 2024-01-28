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
#include <unordered_map>
#include <cstdint>

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
    uint32_t nextHookId = 1;
    template <typename H> struct RegisteredGameHooks { inline static std::unordered_map<uint32_t, typename H::fn> functions; };
    template <typename H> struct HooksToUnregister { inline static std::vector<uint32_t> hooks; };
    template <typename H> uint32_t RegisterGameHook(typename H::fn h) {
        // Ensure hook id is unique and not 0, which is reserved for invalid hooks
        if (this->nextHookId == 0 || this->nextHookId >= UINT32_MAX) this->nextHookId = 1;
        while (RegisteredGameHooks<H>::functions.find(this->nextHookId) != RegisteredGameHooks<H>::functions.end()) {
            this->nextHookId++;
        }

        RegisteredGameHooks<H>::functions[this->nextHookId] = h;
        return this->nextHookId++;
    }
    template <typename H> void UnregisterGameHook(uint32_t id) {
        HooksToUnregister<H>::hooks.push_back(id);
    }

    template <typename H, typename... Args> void ExecuteHooks(Args&&... args) {
        for (auto& hookId : HooksToUnregister<H>::hooks) {
            RegisteredGameHooks<H>::functions.erase(hookId);
        }
        HooksToUnregister<H>::hooks.clear();
        for (auto& hook : RegisteredGameHooks<H>::functions) {
            hook.second(std::forward<Args>(args)...);
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