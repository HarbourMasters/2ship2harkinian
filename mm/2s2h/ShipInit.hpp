#ifndef SHIP_INIT_HPP
#define SHIP_INIT_HPP

#ifdef __cplusplus

#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include <functional>

struct ShipInit {
    static std::unordered_map<std::string, std::vector<std::function<void()>>>& GetAll() {
        static std::unordered_map<std::string, std::vector<std::function<void()>>> shipInitFuncs;
        return shipInitFuncs;
    }

    static void InitAll() {
        ShipInit::Init("*");
    }

    static void Init(const std::string& path) {
        auto& shipInitFuncs = ShipInit::GetAll();
        for (const auto& initFunc : shipInitFuncs[path]) {
            initFunc();
        }
    }
};

struct RegisterShipInitFunc {
    RegisterShipInitFunc(std::function<void()> initFunc, const std::set<std::string>& updatePaths = {}) {
        auto& shipInitFuncs = ShipInit::GetAll();

        shipInitFuncs["*"].push_back(initFunc);

        for (const auto& path : updatePaths) {
            shipInitFuncs[path].push_back(initFunc);
        }
    }
};

// Macro to declare a self-registering init function with a unique variable name,
// safe to use in unity builds where multiple TUs share the same scope.
#define SHIP_INIT_CONCAT_(a, b) a##b
#define SHIP_INIT_CONCAT(a, b) SHIP_INIT_CONCAT_(a, b)
#define REGISTER_SHIP_INIT_FUNC(...) \
    static RegisterShipInitFunc SHIP_INIT_CONCAT(sShipInit_, __COUNTER__)(__VA_ARGS__)

#endif // __cplusplus

#endif // SHIP_INIT_HPP
