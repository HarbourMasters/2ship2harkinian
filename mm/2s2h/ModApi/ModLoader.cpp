/**
 * Finds code mods among the mounted archives and loads them. A mod is an .o2r whose manifest
 * names a binary for this platform; it is extracted, loaded, handed the API table, and its
 * ModInit is run. Self-registers through ShipInit, so nothing else has to call in here.
 */

#include "ModApiHost.h"

#include <spdlog/spdlog.h>
#include <exception>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <ship/Context.h>
#include <ship/resource/archive/Archive.h>
#include <ship/resource/archive/ArchiveManager.h>

#include "2s2h/ShipInit.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

// Must match the keys ScriptLoader::GetPlatform() writes, since both read the same manifest.
static constexpr const char* GetPlatformKey() {
#if defined(_WIN32) || defined(_WIN64)
#if defined(_M_ARM64) || defined(__aarch64__)
    return "windows_arm64";
#else
    return "windows_x64";
#endif
#elif defined(__APPLE__)
    return "darwin";
#elif defined(__ANDROID__)
    return "android";
#elif defined(__linux__)
#if defined(__x86_64__) || defined(_M_X64)
    return "linux_x64";
#elif defined(__i386__) || defined(_M_IX86)
    return "linux_x86";
#elif defined(__aarch64__)
    return "linux_arm64";
#else
    return "linux_generic";
#endif
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    return "bsd";
#else
    return "";
#endif
}

// A library has to exist as a file to be loaded, so the archive's copy is written to a temporary
// one first. POSIX unlinks it right after loading, since the mapping keeps it alive; Windows
// cannot delete a loaded library, so it waits for Unload().
class ModLibrary {
  public:
    bool Load(const std::vector<char>& image) {
        std::string path;
        if (!WriteTemporary(image, path)) {
            return false;
        }

#if defined(_WIN32)
        mHandle = LoadLibraryA(path.c_str());
        mPath = path;
#else
        mHandle = dlopen(path.c_str(), RTLD_NOW);
        unlink(path.c_str());
#endif
        return mHandle != nullptr;
    }

    void* GetFunction(const char* name) const {
        if (mHandle == nullptr) {
            return nullptr;
        }
#if defined(_WIN32)
        return (void*)GetProcAddress((HMODULE)mHandle, name);
#else
        return dlsym(mHandle, name);
#endif
    }

    void Unload() {
        if (mHandle == nullptr) {
            return;
        }
#if defined(_WIN32)
        FreeLibrary((HMODULE)mHandle);
        DeleteFileA(mPath.c_str());
#else
        dlclose(mHandle);
#endif
        mHandle = nullptr;
    }

  private:
    static bool WriteTemporary(const std::vector<char>& image, std::string& path) {
#if defined(_WIN32)
        char directory[MAX_PATH];
        char file[MAX_PATH];
        if (GetTempPathA(MAX_PATH, directory) == 0 || GetTempFileNameA(directory, "s2h", 0, file) == 0) {
            return false;
        }
        path = file;
#else
        char pathTemplate[] = "/tmp/s2h_mod_XXXXXX";
        int descriptor = mkstemp(pathTemplate);
        if (descriptor == -1) {
            return false;
        }
        close(descriptor);
        chmod(pathTemplate, 0755);
        path = pathTemplate;
#endif

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }

        out.write(image.data(), static_cast<std::streamsize>(image.size()));
        return out.good();
    }

    void* mHandle = nullptr;
    std::string mPath;
};

// Mods are never unloaded while the game runs, so ModExit goes uncalled: their hooks would be
// left pointing into freed code. They are released at exit, which on Windows is also the only
// moment the temporary file can be deleted.
class LoadedMods {
  public:
    ~LoadedMods() {
        for (auto& library : mLibraries) {
            library.Unload();
        }
    }

    void Add(const ModLibrary& library) {
        mLibraries.push_back(library);
    }

  private:
    std::vector<ModLibrary> mLibraries;
};

static LoadedMods sLoadedMods;

static void LoadMod(const std::shared_ptr<Ship::Archive>& archive, const std::string& binaryPath,
                    const std::string& name) {
    auto file = archive->LoadFile(binaryPath);
    if (file == nullptr || !file->IsLoaded) {
        SPDLOG_ERROR("[ModApi] Could not read '{}' from mod '{}'", binaryPath, name);
        return;
    }

    ModLibrary library;
    if (!library.Load(*file->Buffer)) {
        SPDLOG_ERROR("[ModApi] Mod '{}' could not be loaded", name);
        return;
    }

    auto setApi = (S2HModSetApiFunc)library.GetFunction("ModSetApi");
    if (setApi == nullptr) {
        SPDLOG_ERROR("[ModApi] Mod '{}' exports no ModSetApi, skipping it", name);
        library.Unload();
        return;
    }

    setApi(ModApi_Get());

    auto init = (S2HModInitFunc)library.GetFunction("ModInit");
    if (init != nullptr) {
        init();
    }

    sLoadedMods.Add(library);
    SPDLOG_INFO("[ModApi] Loaded mod '{}'", name);
}

static void LoadMods() {
    const std::string platform = GetPlatformKey();
    if (platform.empty()) {
        return;
    }

    auto resourceManager = Ship::Context::GetRawInstance()->GetResourceManager();
    if (resourceManager == nullptr) {
        return;
    }

    auto archives = resourceManager->GetArchiveManager()->GetArchives();
    if (archives == nullptr) {
        return;
    }

    for (const auto& archive : *archives) {
        const auto& manifest = archive->GetManifest();

        auto binary = manifest.Binaries.find(platform);
        if (binary == manifest.Binaries.end()) {
            continue;
        }

        if (manifest.CodeVersion != S2H_MOD_API_VERSION) {
            SPDLOG_ERROR("[ModApi] Mod '{}' targets API {}, this build serves {}", manifest.Name, manifest.CodeVersion,
                         S2H_MOD_API_VERSION);
            continue;
        }

        try {
            LoadMod(archive, binary->second, manifest.Name);
        } catch (const std::exception& e) {
            SPDLOG_ERROR("[ModApi] Mod '{}' failed to load: {}", manifest.Name, e.what());
        }
    }
}

static RegisterShipInitFunc initFunc(LoadMods);
