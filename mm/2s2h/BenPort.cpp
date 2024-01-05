﻿#include "BenPort.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <chrono>

#include <ResourceManager.h>
#include <File.h>
#include <DisplayList.h>
#include <Window.h>
#include <GameVersions.h>

#include "z64animation.h"
#include "z64bgcheck.h"
#include <libultraship/libultra/gbi.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <time.h>
#endif
#include <Array.h>
#include <stb/stb_image.h>
#define DRMP3_IMPLEMENTATION
#include <dr_libs/mp3.h>
#define DRWAV_IMPLEMENTATION
#include <dr_libs/wav.h>
#include <AudioPlayer.h>
#include "variables.h"
#include "z64.h"
#include "macros.h"
#include <Utils/StringHelper.h>

#include <Fast3D/gfx_pc.h>
#include <Fast3D/gfx_rendering_api.h>

#ifdef __APPLE__
#include <SDL_scancode.h>
#else
#include <SDL2/SDL_scancode.h>
#endif
#include "Extractor/Extract.h"
// OTRTODO
//#include <functions.h>


#ifdef ENABLE_CROWD_CONTROL
#include "Enhancements/crowd-control/CrowdControl.h"
CrowdControl* CrowdControl::Instance;
#endif

#include <libultraship/libultraship.h>
#include <SohGui.hpp>

// Resource Types/Factories
#include "2s2h/resource/type/Animation.h"
#include "2s2h/resource/type/AudioSample.h"
#include "2s2h/resource/type/AudioSequence.h"
#include "2s2h/resource/type/AudioSoundFont.h"
#include "2s2h/resource/type/CollisionHeader.h"
#include "2s2h/resource/type/Cutscene.h"
#include "2s2h/resource/type/Path.h"
#include "2s2h/resource/type/PlayerAnimation.h"
#include "2s2h/resource/type/Scene.h"
#include "2s2h/resource/type/Skeleton.h"
#include "2s2h/resource/type/SkeletonLimb.h"
#include "2s2h/resource/type/Text.h"
#include "2s2h/resource/importer/AnimationFactory.h"
#include "2s2h/resource/importer/AudioSampleFactory.h"
#include "2s2h/resource/importer/AudioSequenceFactory.h"
#include "2s2h/resource/importer/AudioSoundFontFactory.h"
#include "2s2h/resource/importer/CollisionHeaderFactory.h"
#include "2s2h/resource/importer/CutsceneFactory.h"
#include "2s2h/resource/importer/PathFactory.h"
#include "2s2h/resource/importer/PlayerAnimationFactory.h"
#include "2s2h/resource/importer/SceneFactory.h"
#include "2s2h/resource/importer/SkeletonFactory.h"
#include "2s2h/resource/importer/SkeletonLimbFactory.h"
#include "2s2h/resource/importer/TextFactory.h"
#include "2s2h/resource/importer/TextMMFactory.h"
#include "2s2h/resource/importer/BackgroundFactory.h"
#include "2s2h/resource/importer/TextureAnimationFactory.h"

OTRGlobals* OTRGlobals::Instance;

extern "C" char** cameraStrings;
std::vector<std::shared_ptr<std::string>> cameraStdStrings;

Color_RGB8 kokiriColor = { 0x1E, 0x69, 0x1B };
Color_RGB8 goronColor = { 0x64, 0x14, 0x00 };
Color_RGB8 zoraColor = { 0x00, 0xEC, 0x64 };


OTRGlobals::OTRGlobals() {
    std::vector<std::string> OTRFiles;
    //std::string mqPath = LUS::Context::LocateFileAcrossAppDirs("oot-mq.otr", appShortName);
    //if (std::filesystem::exists(mqPath)) {
    //    OTRFiles.push_back(mqPath);
    //}
    std::string ootPath = LUS::Context::LocateFileAcrossAppDirs("mm.otr", appShortName);
    if (std::filesystem::exists(ootPath)) {
        OTRFiles.push_back(ootPath);
    }
    std::string sohOtrPath = LUS::Context::GetPathRelativeToAppBundle("soh.otr");
    if (std::filesystem::exists(sohOtrPath)) {
        OTRFiles.push_back(sohOtrPath);
    }
    std::string patchesPath = LUS::Context::LocateFileAcrossAppDirs("mods", appShortName);
    if (patchesPath.length() > 0 && std::filesystem::exists(patchesPath)) {
        if (std::filesystem::is_directory(patchesPath)) {
            for (const auto& p : std::filesystem::recursive_directory_iterator(patchesPath)) {
                if (StringHelper::IEquals(p.path().extension().string(), ".otr")) {
                    OTRFiles.push_back(p.path().generic_string());
                }
            }
        }
    }
    std::unordered_set<uint32_t> ValidHashes = { OOT_PAL_MQ,     OOT_NTSC_JP_MQ,    OOT_NTSC_US_MQ, OOT_PAL_GC_MQ_DBG,
                                                 OOT_NTSC_US_10, OOT_NTSC_US_11,    OOT_NTSC_US_12, OOT_PAL_10,
                                                 OOT_PAL_11,     OOT_NTSC_JP_GC_CE, OOT_NTSC_JP_GC, OOT_NTSC_US_GC,
                                                 OOT_PAL_GC,     OOT_PAL_GC_DBG1,   OOT_PAL_GC_DBG2 };
    // tell LUS to reserve 3 SoH specific threads (Game, Audio, Save)
    context = LUS::Context::CreateInstance("2 Ship 2 Harkinian", appShortName, "shipofharkinian.json", OTRFiles, {}, 3);
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_Animation, "Animation", std::make_shared<LUS::AnimationFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_PlayerAnimation, "PlayerAnimation", std::make_shared<LUS::PlayerAnimationFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_Room, "Room", std::make_shared<LUS::SceneFactory>()); // Is room scene? maybe?
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_CollisionHeader, "CollisionHeader", std::make_shared<LUS::CollisionHeaderFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_Skeleton, "Skeleton", std::make_shared<LUS::SkeletonFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_SkeletonLimb, "SkeletonLimb", std::make_shared<LUS::SkeletonLimbFactory>());
    // TODO should we use a custom command for this?
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(LUS::ResourceType::SOH_Path, "Path",
                                                                                std::make_shared<LUS::PathFactoryMM>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_Cutscene, "Cutscene", std::make_shared<LUS::CutsceneFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(LUS::ResourceType::SOH_Text, "Text",
                                                                                std::make_shared<LUS::TextFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(LUS::ResourceType::SOH_TextMM, "TextMM",
                                                                                std::make_shared<LUS::TextMMFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_AudioSample, "AudioSample", std::make_shared<LUS::AudioSampleFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_AudioSoundFont, "AudioSoundFont", std::make_shared<LUS::AudioSoundFontFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_AudioSequence, "AudioSequence", std::make_shared<LUS::AudioSequenceFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::SOH_Background, "Background", std::make_shared<LUS::BackgroundFactory>());
    context->GetResourceManager()->GetResourceLoader()->RegisterResourceFactory(
        LUS::ResourceType::TSH_TexAnim, "TextureAnimation", std::make_shared<LUS::TextureAnimationFactory>());

    //gSaveStateMgr = std::make_shared<SaveStateMgr>();
    //gRandomizer = std::make_shared<Randomizer>();
    hasMasterQuest = hasOriginal = false;

    // Move the camera strings from read only memory onto the heap (writable memory)
    // This is in OTRGlobals right now because this is a place that will only ever be run once at the beginning of
    // startup. We should probably find some code in db_camera that does initialization and only run once, and then
    // dealloc on deinitialization.
    //cameraStrings = (char**)malloc(sizeof(constCameraStrings));
    //for (int32_t i = 0; i < sizeof(constCameraStrings) / sizeof(char*); i++) {
    //    // OTRTODO: never deallocated...
    //    auto dup = strdup(constCameraStrings[i]);
    //    cameraStrings[i] = dup;
    //}

    auto versions = context->GetResourceManager()->GetArchive()->GetGameVersions();
    #if 0
    for (uint32_t version : versions) {
        if (!ValidHashes.contains(version)) {
#if defined(__SWITCH__)
            SPDLOG_ERROR("Invalid OTR File!");
#elif defined(__WIIU__)
            LUS::WiiU::ThrowInvalidOTR();
#else
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid OTR File",
                                     "Attempted to load an invalid OTR file. Try regenerating.", nullptr);
            SPDLOG_ERROR("Invalid OTR File!");
#endif
            exit(1);
        }
        switch (version) {
            case OOT_PAL_MQ:
            case OOT_NTSC_JP_MQ:
            case OOT_NTSC_US_MQ:
            case OOT_PAL_GC_MQ_DBG:
                hasMasterQuest = true;
                break;
            case OOT_NTSC_US_10:
            case OOT_NTSC_US_11:
            case OOT_NTSC_US_12:
            case OOT_PAL_10:
            case OOT_PAL_11:
            case OOT_NTSC_JP_GC_CE:
            case OOT_NTSC_JP_GC:
            case OOT_NTSC_US_GC:
            case OOT_PAL_GC:
            case OOT_PAL_GC_DBG1:
            case OOT_PAL_GC_DBG2:
                hasOriginal = true;
                break;
            default:
                break;
        }
    }
    #endif
}

OTRGlobals::~OTRGlobals() {
}

bool OTRGlobals::HasMasterQuest() {
    return hasMasterQuest;
}

bool OTRGlobals::HasOriginal() {
    return hasOriginal;
}

uint32_t OTRGlobals::GetInterpolationFPS() {
    if (LUS::Context::GetInstance()->GetWindow()->GetWindowBackend() == LUS::WindowBackend::DX11) {
        return CVarGetInteger("gInterpolationFPS", 20);
    }

    if (CVarGetInteger("gMatchRefreshRate", 0)) {
        return LUS::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
    }

    return std::min<uint32_t>(LUS::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate(),
                              CVarGetInteger("gInterpolationFPS", 20));
}

struct ExtensionEntry {
    std::string path;
    std::string ext;
};

extern uintptr_t clearMtx;
extern "C" Mtx gMtxClear;
extern "C" MtxF gMtxFClear;
extern "C" void OTRMessage_Init();
extern "C" void AudioMgr_CreateNextAudioBuffer(s16* samples, u32 num_samples);
extern "C" void AudioPlayer_Play(const uint8_t* buf, uint32_t len);
extern "C" int AudioPlayer_Buffered(void);
extern "C" int AudioPlayer_GetDesiredBuffered(void);
extern "C" void ResourceMgr_LoadDirectory(const char* resName);
std::unordered_map<std::string, ExtensionEntry> ExtensionCache;

extern "C" void OTRExtScanner() {
    auto lst = *LUS::Context::GetInstance()->GetResourceManager()->GetArchive()->ListFiles("*").get();

    for (auto& rPath : lst) {
        std::vector<std::string> raw = StringHelper::Split(rPath, ".");
        std::string ext = raw[raw.size() - 1];
        std::string nPath = rPath.substr(0, rPath.size() - (ext.size() + 1));
        replace(nPath.begin(), nPath.end(), '\\', '/');

        ExtensionCache[nPath] = { rPath, ext };
    }
}

extern "C" void InitOTR() {
#if not defined(__SWITCH__) && not defined(__WIIU__)
    if (!std::filesystem::exists(LUS::Context::LocateFileAcrossAppDirs("mm.otr", appShortName))) {
        std::string installPath = LUS::Context::GetAppBundlePath();
        if (!std::filesystem::exists(installPath + "/assets/extractor")) {
            Extractor::ShowErrorBox(
                "Extractor assets not found",
                "No OTR files found. Missing assets/extractor folder needed to generate OTR file. Exiting...");
            exit(1);
        }

        bool generatedOtrIsMQ = false;
        if (Extractor::ShowYesNoBox("No OTR Files", "No OTR files found. Generate one now?") == IDYES) {
            Extractor extract;
            if (!extract.Run()) {
                Extractor::ShowErrorBox("Error", "An error occured, no OTR file was generated. Exiting...");
                exit(1);
            }
            extract.CallZapd(installPath, LUS::Context::GetAppDirectoryPath(appShortName));
            generatedOtrIsMQ = extract.IsMasterQuest();
        } else {
            exit(1);
        }
    }
#endif

#ifdef __SWITCH__
    LUS::Switch::Init(LUS::PreInitPhase);
#elif defined(__WIIU__)
    LUS::WiiU::Init("soh");
#endif

    OTRGlobals::Instance = new OTRGlobals();
    SohGui::SetupGuiElements();

    clearMtx = (uintptr_t)&gMtxClear;
    //OTRMessage_Init();
    //OTRExtScanner();
    time_t now = time(NULL);
    tm* tm_now = localtime(&now);
    if (tm_now->tm_mon == 11 && tm_now->tm_mday >= 24 && tm_now->tm_mday <= 25) {
        CVarRegisterInteger("gLetItSnow", 1);
    } else {
        CVarClear("gLetItSnow");
    }

    srand(now);
#ifdef ENABLE_CROWD_CONTROL
    CrowdControl::Instance = new CrowdControl();
    CrowdControl::Instance->Init();
    if (CVarGetInteger("gCrowdControl", 0)) {
        CrowdControl::Instance->Enable();
    } else {
        CrowdControl::Instance->Disable();
    }
#endif

    std::shared_ptr<LUS::Config> conf = OTRGlobals::Instance->context->GetConfig();

}

extern "C" void SaveManager_ThreadPoolWait() {
    //SaveManager::Instance->ThreadPoolWait();
}

extern "C" void DeinitOTR() {
    SaveManager_ThreadPoolWait();
#ifdef ENABLE_CROWD_CONTROL
    CrowdControl::Instance->Disable();
    CrowdControl::Instance->Shutdown();
#endif

    // Destroying gui here because we have shared ptrs to LUS objects which output to SPDLOG which is destroyed before
    // these shared ptrs.
    //SohGui::Destroy();

    OTRGlobals::Instance->context = nullptr;
}

#ifdef _WIN32
extern "C" uint64_t GetFrequency() {
    LARGE_INTEGER nFreq;

    QueryPerformanceFrequency(&nFreq);

    return nFreq.QuadPart;
}

extern "C" uint64_t GetPerfCounter() {
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);

    return ticks.QuadPart;
}
#else
extern "C" uint64_t GetFrequency() {
    return 1000; // sec -> ms
}

extern "C" uint64_t GetPerfCounter() {
    struct timespec monotime;
    clock_gettime(CLOCK_MONOTONIC, &monotime);

    uint64_t remainingMs = (monotime.tv_nsec / 1000000);

    // in milliseconds
    return monotime.tv_sec * 1000 + remainingMs;
}
#endif

extern "C" uint64_t GetUnixTimestamp() {
    auto time = std::chrono::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    long now = millis.count();
    return now;
}

// C->C++ Bridge
extern "C" void Graph_ProcessFrame(void (*run_one_game_iter)(void)) {
    OTRGlobals::Instance->context->GetWindow()->MainLoop(run_one_game_iter);
}

extern bool ShouldClearTextureCacheAtEndOfFrame;

extern "C" void Graph_StartFrame() {
#ifndef __WIIU__
    using LUS::KbScancode;
    int32_t dwScancode = OTRGlobals::Instance->context->GetWindow()->GetLastScancode();
    OTRGlobals::Instance->context->GetWindow()->SetLastScancode(-1);

    switch (dwScancode) {
        #if 0
        case KbScancode::LUS_KB_F5: {
            if (CVarGetInteger("gSaveStatesEnabled", 0) == 0) {
                LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(
                    6.0f, true, "Save states not enabled. Check Cheats Menu.");
                return;
            }
            const unsigned int slot = OTRGlobals::Instance->gSaveStateMgr->GetCurrentSlot();
            const SaveStateReturn stateReturn =
                OTRGlobals::Instance->gSaveStateMgr->AddRequest({ slot, RequestType::SAVE });

            switch (stateReturn) {
                case SaveStateReturn::SUCCESS:
                    SPDLOG_INFO("[SOH] Saved state to slot {}", slot);
                    break;
                case SaveStateReturn::FAIL_WRONG_GAMESTATE:
                    SPDLOG_ERROR("[SOH] Can not save a state outside of \"GamePlay\"");
                    break;
                    [[unlikely]] default : break;
            }
            break;
        }
        case KbScancode::LUS_KB_F6: {
            if (CVarGetInteger("gSaveStatesEnabled", 0) == 0) {
                LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(
                    6.0f, true, "Save states not enabled. Check Cheats Menu.");
                return;
            }
            unsigned int slot = OTRGlobals::Instance->gSaveStateMgr->GetCurrentSlot();
            slot++;
            if (slot > 5) {
                slot = 0;
            }
            OTRGlobals::Instance->gSaveStateMgr->SetCurrentSlot(slot);
            SPDLOG_INFO("Set SaveState slot to {}.", slot);
            break;
        }
        case KbScancode::LUS_KB_F7: {
            if (CVarGetInteger("gSaveStatesEnabled", 0) == 0) {
                LUS::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->TextDrawNotification(
                    6.0f, true, "Save states not enabled. Check Cheats Menu.");
                return;
            }
            const unsigned int slot = OTRGlobals::Instance->gSaveStateMgr->GetCurrentSlot();
            const SaveStateReturn stateReturn =
                OTRGlobals::Instance->gSaveStateMgr->AddRequest({ slot, RequestType::LOAD });

            switch (stateReturn) {
                case SaveStateReturn::SUCCESS:
                    SPDLOG_INFO("[SOH] Loaded state from slot {}", slot);
                    break;
                case SaveStateReturn::FAIL_INVALID_SLOT:
                    SPDLOG_ERROR("[SOH] Invalid State Slot Number {}", slot);
                    break;
                case SaveStateReturn::FAIL_STATE_EMPTY:
                    SPDLOG_ERROR("[SOH] State Slot {} is empty", slot);
                    break;
                case SaveStateReturn::FAIL_WRONG_GAMESTATE:
                    SPDLOG_ERROR("[SOH] Can not load a state outside of \"GamePlay\"");
                    break;
                    [[unlikely]] default : break;
            }

            break;
        }
#endif
#if defined(_WIN32) || defined(__APPLE__)
        case KbScancode::LUS_KB_F9: {
            // Toggle TTS
            CVarSetInteger("gA11yTTS", !CVarGetInteger("gA11yTTS", 0));
            break;
        }
#endif
        case KbScancode::LUS_KB_TAB: {
            // Toggle HD Assets
            CVarSetInteger("gAltAssets", !CVarGetInteger("gAltAssets", 0));
            //ShouldClearTextureCacheAtEndOfFrame = true;
            break;
        }
    }
#endif
    OTRGlobals::Instance->context->GetWindow()->StartFrame();
}

void RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements) {
    //for (const auto& m : mtx_replacements) {
    gfx_run(Commands, {});
        gfx_end_frame();
   // }
}

// C->C++ Bridge
extern "C" void Graph_ProcessGfxCommands(Gfx* commands) {
    {
        //std::unique_lock<std::mutex> Lock(audio.mutex);
        //audio.processing = true;
    }

    //audio.cv_to_thread.notify_one();
    std::vector<std::unordered_map<Mtx*, MtxF>> mtx_replacements;
    int target_fps = 20;
    //OTRGlobals::Instance->GetInterpolationFPS();
    static int last_fps;
    static int last_update_rate;
    static int time;
    int fps = target_fps;
    int original_fps = 60 / R_UPDATE_RATE;

    if (target_fps == 20 || original_fps > target_fps) {
        fps = original_fps;
    }

    if (last_fps != fps || last_update_rate != R_UPDATE_RATE) {
        time = 0;
    }

    // time_base = fps * original_fps (one second)
   // int next_original_frame = fps;

    //while (time + original_fps <= next_original_frame) {
    //    time += original_fps;
    //    if (time != next_original_frame) {
    //        mtx_replacements.push_back(FrameInterpolation_Interpolate((float)time / next_original_frame));
    //    } else {
    //        mtx_replacements.emplace_back();
    //    }
    //}

    //time -= fps;

     OTRGlobals::Instance->context->GetWindow()->SetTargetFps(original_fps);
    // OTRGlobals::Instance->context->GetWindow()->SetTargetFps(20);
    //OTRGlobals::Instance->context->GetWindow()->SetTargetFps(60);

    //int threshold = CVarGetInteger("gExtraLatencyThreshold", 80);
    //OTRGlobals::Instance->context->GetWindow()->SetMaximumFrameLatency(threshold > 0 && target_fps >= threshold ? 2
                                                                                                                //: 1);

    RunCommands(commands, mtx_replacements);

    //last_fps = fps;
    //last_update_rate = R_UPDATE_RATE;

    //{
    //    std::unique_lock<std::mutex> Lock(audio.mutex);
    //    while (audio.processing) {
    //        audio.cv_from_thread.wait(Lock);
    //    }
    //}
    //
    //if (ShouldClearTextureCacheAtEndOfFrame) {
    //    gfx_texture_cache_clear();
    //    LUS::SkeletonPatcher::UpdateSkeletons();
    //    ShouldClearTextureCacheAtEndOfFrame = false;
    //}

    // OTRTODO: FIGURE OUT END FRAME POINT
    /* if (OTRGlobals::Instance->context->lastScancode != -1)
         OTRGlobals::Instance->context->lastScancode = -1;*/
}

float divisor_num = 0.0f;

extern "C" void OTRGetPixelDepthPrepare(float x, float y) {
    OTRGlobals::Instance->context->GetWindow()->GetPixelDepthPrepare(x, y);
}

extern "C" uint16_t OTRGetPixelDepth(float x, float y) {
    return OTRGlobals::Instance->context->GetWindow()->GetPixelDepth(x, y);
}

extern "C" uint32_t ResourceMgr_GetNumGameVersions() {
    return LUS::Context::GetInstance()->GetResourceManager()->GetArchive()->GetGameVersions().size();
}

extern "C" uint32_t ResourceMgr_GetGameVersion(int index) {
    return LUS::Context::GetInstance()->GetResourceManager()->GetArchive()->GetGameVersions()[index];
}

extern "C" uint32_t ResourceMgr_GetGamePlatform(int index) {
    uint32_t version = LUS::Context::GetInstance()->GetResourceManager()->GetArchive()->GetGameVersions()[index];

    switch (version) {
        case OOT_NTSC_US_10:
        case OOT_NTSC_US_11:
        case OOT_NTSC_US_12:
        case OOT_PAL_10:
        case OOT_PAL_11:
            return GAME_PLATFORM_N64;
        case OOT_NTSC_JP_GC:
        case OOT_NTSC_US_GC:
        case OOT_PAL_GC:
        case OOT_NTSC_JP_MQ:
        case OOT_NTSC_US_MQ:
        case OOT_PAL_MQ:
        case OOT_PAL_GC_DBG1:
        case OOT_PAL_GC_DBG2:
        case OOT_PAL_GC_MQ_DBG:
            return GAME_PLATFORM_GC;
    }
}

extern "C" uint32_t ResourceMgr_GetGameRegion(int index) {
    uint32_t version = LUS::Context::GetInstance()->GetResourceManager()->GetArchive()->GetGameVersions()[index];

    switch (version) {
        case OOT_NTSC_US_10:
        case OOT_NTSC_US_11:
        case OOT_NTSC_US_12:
        case OOT_NTSC_JP_GC:
        case OOT_NTSC_US_GC:
        case OOT_NTSC_JP_MQ:
        case OOT_NTSC_US_MQ:
            return GAME_REGION_NTSC;
        case OOT_PAL_10:
        case OOT_PAL_11:
        case OOT_PAL_GC:
        case OOT_PAL_MQ:
        case OOT_PAL_GC_DBG1:
        case OOT_PAL_GC_DBG2:
        case OOT_PAL_GC_MQ_DBG:
            return GAME_REGION_PAL;
    }
}

uint32_t IsSceneMasterQuest(s16 sceneNum) {
    return false;
    uint32_t value = 0;
    //uint8_t mqMode = CVarGetInteger("gBetterDebugWarpScreenMQMode", WARP_MODE_OVERRIDE_OFF);
    //if (mqMode == WARP_MODE_OVERRIDE_MQ_AS_VANILLA) {
    //    return 1;
    //} else if (mqMode == WARP_MODE_OVERRIDE_VANILLA_AS_MQ) {
    //    return 0;
    //} else {
    //    if (OTRGlobals::Instance->HasMasterQuest()) {
    //        if (!OTRGlobals::Instance->HasOriginal()) {
    //            value = 1;
    //        } else if (IS_MASTER_QUEST) {
    //            value = 1;
    //        } else {
    //            value = 0;
    //            if (IS_RANDO && !OTRGlobals::Instance->gRandomizer->masterQuestDungeons.empty() &&
    //                OTRGlobals::Instance->gRandomizer->masterQuestDungeons.contains(sceneNum)) {
    //                value = 1;
    //            }
    //        }
    //    }
    //}
    return value;
}

uint32_t IsGameMasterQuest() {
    return false;
    //return gPlayState != NULL ? IsSceneMasterQuest(gPlayState->sceneNum) : 0;
}

extern "C" uint32_t ResourceMgr_GameHasMasterQuest() {
    return OTRGlobals::Instance->HasMasterQuest();
}

extern "C" uint32_t ResourceMgr_GameHasOriginal() {
    return OTRGlobals::Instance->HasOriginal();
}

extern "C" uint32_t ResourceMgr_IsSceneMasterQuest(s16 sceneNum) {
    return IsSceneMasterQuest(sceneNum);
}

extern "C" uint32_t ResourceMgr_IsGameMasterQuest() {
    return IsGameMasterQuest();
}

extern "C" void ResourceMgr_LoadDirectory(const char* resName) {
    LUS::Context::GetInstance()->GetResourceManager()->LoadDirectory(resName);
}
extern "C" void ResourceMgr_DirtyDirectory(const char* resName) {
    LUS::Context::GetInstance()->GetResourceManager()->DirtyDirectory(resName);
}

// OTRTODO: There is probably a more elegant way to go about this...
// Kenix: This is definitely leaking memory when it's called.
extern "C" char** ResourceMgr_ListFiles(const char* searchMask, int* resultSize) {
    auto lst = LUS::Context::GetInstance()->GetResourceManager()->GetArchive()->ListFiles(searchMask);
    char** result = (char**)malloc(lst->size() * sizeof(char*));

    for (size_t i = 0; i < lst->size(); i++) {
        char* str = (char*)malloc(lst.get()[0][i].size() + 1);
        memcpy(str, lst.get()[0][i].data(), lst.get()[0][i].size());
        str[lst.get()[0][i].size()] = '\0';
        result[i] = str;
    }
    *resultSize = lst->size();

    return result;
}

extern "C" uint8_t ResourceMgr_FileExists(const char* filePath) {
    std::string path = filePath;
    if (path.substr(0, 7) == "__OTR__") {
        path = path.substr(7);
    }

    return ExtensionCache.contains(path);
}

extern "C" void ResourceMgr_LoadFile(const char* resName) {
    LUS::Context::GetInstance()->GetResourceManager()->LoadResource(resName);
}

std::shared_ptr<LUS::IResource> GetResourceByNameHandlingMQ(const char* path) {
    std::string Path = path;
    if (ResourceMgr_IsGameMasterQuest()) {
        size_t pos = 0;
        if ((pos = Path.find("/nonmq/", 0)) != std::string::npos) {
            Path.replace(pos, 7, "/mq/");
        }
    }
    return LUS::Context::GetInstance()->GetResourceManager()->LoadResource(Path.c_str());
}

extern "C" char* GetResourceDataByNameHandlingMQ(const char* path) {
    auto res = GetResourceByNameHandlingMQ(path);

    if (res == nullptr) {
        return nullptr;
    }

    return (char*)res->GetRawPointer();
}

extern "C" char* ResourceMgr_LoadFileFromDisk(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    fseek(file, 0, SEEK_END);
    int fSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(fSize);
    fread(data, 1, fSize, file);

    fclose(file);

    return data;
}

extern "C" uint8_t ResourceMgr_ResourceIsBackground(char* texPath) {
    auto res = GetResourceByNameHandlingMQ(texPath);
    return res->GetInitData()->Type == LUS::ResourceType::SOH_Background;
}

extern "C" char* ResourceMgr_LoadJPEG(char* data, size_t dataSize) {
    static char* finalBuffer = 0;

    if (finalBuffer == 0)
        finalBuffer = (char*)malloc(dataSize);

    int w;
    int h;
    int comp;

    unsigned char* pixels =
        stbi_load_from_memory((const unsigned char*)data, 320 * 240 * 2, &w, &h, &comp, STBI_rgb_alpha);
    // unsigned char* pixels = stbi_load_from_memory((const unsigned char*)data, 480 * 240 * 2, &w, &h, &comp,
    // STBI_rgb_alpha);
    int idx = 0;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint16_t* bufferTest = (uint16_t*)finalBuffer;
            int pixelIdx = ((y * w) + x) * 4;

            uint8_t r = pixels[pixelIdx + 0] / 8;
            uint8_t g = pixels[pixelIdx + 1] / 8;
            uint8_t b = pixels[pixelIdx + 2] / 8;

            uint8_t alphaBit = pixels[pixelIdx + 3] != 0;

            uint16_t data = (r << 11) + (g << 6) + (b << 1) + alphaBit;

            finalBuffer[idx++] = (data & 0xFF00) >> 8;
            finalBuffer[idx++] = (data & 0x00FF);
        }
    }

    return (char*)finalBuffer;
}

extern "C" uint16_t ResourceMgr_LoadTexWidthByName(char* texPath);

extern "C" uint16_t ResourceMgr_LoadTexHeightByName(char* texPath);

extern "C" char* ResourceMgr_LoadTexOrDListByName(const char* filePath) {
    auto res = GetResourceByNameHandlingMQ(filePath);

    if (res->GetInitData()->Type == LUS::ResourceType::DisplayList)
        return (char*)&((std::static_pointer_cast<LUS::DisplayList>(res))->Instructions[0]);
    else if (res->GetInitData()->Type == LUS::ResourceType::Array)
        return (char*)(std::static_pointer_cast<LUS::Array>(res))->Vertices.data();
    else {
        return (char*)GetResourceDataByNameHandlingMQ(filePath);
    }
}

extern "C" char* ResourceMgr_LoadIfDListByName(const char* filePath) {
    auto res = GetResourceByNameHandlingMQ(filePath);

    if (res->GetInitData()->Type == LUS::ResourceType::DisplayList)
        return (char*)&((std::static_pointer_cast<LUS::DisplayList>(res))->Instructions[0]);

    return nullptr;
}

//extern "C" Sprite* GetSeedTexture(uint8_t index) {
//    return OTRGlobals::Instance->gRandomizer->GetSeedTexture(index);
//}

extern "C" char* ResourceMgr_LoadPlayerAnimByName(const char* animPath) {
    auto anim = std::static_pointer_cast<LUS::PlayerAnimation>(GetResourceByNameHandlingMQ(animPath));

    return (char*)&anim->limbRotData[0];
}

extern "C" void ResourceMgr_PushCurrentDirectory(char* path) {
    gfx_push_current_dir(path);
}

extern "C" Gfx* ResourceMgr_LoadGfxByName(const char* path) {
    auto res = std::static_pointer_cast<LUS::DisplayList>(GetResourceByNameHandlingMQ(path));
    return (Gfx*)&res->Instructions[0];
}

typedef struct {
    int index;
    Gfx instruction;
} GfxPatch;

std::unordered_map<std::string, std::unordered_map<std::string, GfxPatch>> originalGfx;

// Attention! This is primarily for cosmetics & bug fixes. For things like mods and model replacement you should be
// using OTRs instead (When that is available). Index can be found using the commented out section below.
extern "C" void ResourceMgr_PatchGfxByName(const char* path, const char* patchName, int index, Gfx instruction) {
    auto res = std::static_pointer_cast<LUS::DisplayList>(
        LUS::Context::GetInstance()->GetResourceManager()->LoadResource(path));

    // Leaving this here for people attempting to find the correct Dlist index to patch
    /*if (strcmp("__OTR__objects/object_gi_longsword/gGiBiggoronSwordDL", path) == 0) {
        for (int i = 0; i < res->instructions.size(); i++) {
            Gfx* gfx = (Gfx*)&res->instructions[i];
            // Log all commands
            // SPDLOG_INFO("index:{} command:{}", i, gfx->words.w0 >> 24);
            // Log only SetPrimColors
            if (gfx->words.w0 >> 24 == 250) {
                SPDLOG_INFO("index:{} r:{} g:{} b:{} a:{}", i, _SHIFTR(gfx->words.w1, 24, 8), _SHIFTR(gfx->words.w1, 16,
    8), _SHIFTR(gfx->words.w1, 8, 8), _SHIFTR(gfx->words.w1, 0, 8));
            }
        }
    }*/

    // Index refers to individual gfx words, which are half the size on 32-bit
    // if (sizeof(uintptr_t) < 8) {
    // index /= 2;
    // }

    Gfx* gfx = (Gfx*)&res->Instructions[index];

    if (!originalGfx.contains(path) || !originalGfx[path].contains(patchName)) {
        originalGfx[path][patchName] = { index, *gfx };
    }

    *gfx = instruction;
}

extern "C" void ResourceMgr_PatchGfxCopyCommandByName(const char* path, const char* patchName, int destinationIndex,
                                                      int sourceIndex) {
    auto res = std::static_pointer_cast<LUS::DisplayList>(
        LUS::Context::GetInstance()->GetResourceManager()->LoadResource(path));

    Gfx* destinationGfx = (Gfx*)&res->Instructions[destinationIndex];
    Gfx sourceGfx = res->Instructions[sourceIndex];

    if (!originalGfx.contains(path) || !originalGfx[path].contains(patchName)) {
        originalGfx[path][patchName] = { destinationIndex, *destinationGfx };
    }

    *destinationGfx = sourceGfx;
}

extern "C" void ResourceMgr_UnpatchGfxByName(const char* path, const char* patchName) {
    if (originalGfx.contains(path) && originalGfx[path].contains(patchName)) {
        auto res = std::static_pointer_cast<LUS::DisplayList>(
            LUS::Context::GetInstance()->GetResourceManager()->LoadResource(path));

        Gfx* gfx = (Gfx*)&res->Instructions[originalGfx[path][patchName].index];
        *gfx = originalGfx[path][patchName].instruction;

        originalGfx[path].erase(patchName);
    }
}

extern "C" char* ResourceMgr_LoadVtxArrayByName(const char* path) {
    auto res = std::static_pointer_cast<LUS::Array>(GetResourceByNameHandlingMQ(path));

    return (char*)res->Vertices.data();
}

extern "C" size_t ResourceMgr_GetVtxArraySizeByName(const char* path) {
    auto res = std::static_pointer_cast<LUS::Array>(GetResourceByNameHandlingMQ(path));

    return res->Vertices.size();
    // }
}

extern "C" char* ResourceMgr_LoadArrayByName(const char* path) {
    auto res = std::static_pointer_cast<LUS::Array>(GetResourceByNameHandlingMQ(path));

    return (char*)res->Scalars.data();
}

extern "C" size_t ResourceMgr_GetArraySizeByName(const char* path) {
    auto res = std::static_pointer_cast<LUS::Array>(GetResourceByNameHandlingMQ(path));

    return res->Scalars.size();
    // }
}
extern "C" char* ResourceMgr_LoadArrayByNameAsVec3s(const char* path) {
    auto res = std::static_pointer_cast<LUS::Array>(GetResourceByNameHandlingMQ(path));

    // if (res->CachedGameAsset != nullptr)
    //     return (char*)res->CachedGameAsset;
    // else
    // {
    Vec3s* data = (Vec3s*)malloc(sizeof(Vec3s) * res->Scalars.size());

    for (size_t i = 0; i < res->Scalars.size(); i += 3) {
        data[(i / 3)].x = res->Scalars[i + 0].s16;
        data[(i / 3)].y = res->Scalars[i + 1].s16;
        data[(i / 3)].z = res->Scalars[i + 2].s16;
    }

    // res->CachedGameAsset = data;

    return (char*)data;
    // }
}

extern "C" AnimatedMaterial* ResourceMgr_LoadAnimatedMatByName(const char* path) {
    return (AnimatedMaterial*)ResourceGetDataByName(path);
}

extern "C" CollisionHeader* ResourceMgr_LoadColByName(const char* path) {
    return (CollisionHeader*)ResourceGetDataByName(path);
}

extern "C" Vtx* ResourceMgr_LoadVtxByName(char* path) {
    return (Vtx*)ResourceGetDataByName(path);
}

//extern "C" SequenceData ResourceMgr_LoadSeqByName(const char* path) {
//    SequenceData* sequence = (SequenceData*)ResourceGetDataByName(path);
//    return *sequence;
//}
//
//std::map<std::string, SoundFontSample*> cachedCustomSFs;
#if 0
extern "C" SoundFontSample* ReadCustomSample(const char* path) {
    return nullptr;
    /*
        if (!ExtensionCache.contains(path))
            return nullptr;

        ExtensionEntry entry = ExtensionCache[path];

        auto sampleRaw = LUS::Context::GetInstance()->GetResourceManager()->LoadFile(entry.path);
        uint32_t* strem = (uint32_t*)sampleRaw->Buffer.get();
        uint8_t* strem2 = (uint8_t*)strem;

        SoundFontSample* sampleC = new SoundFontSample;

        if (entry.ext == "wav") {
            drwav_uint32 channels;
            drwav_uint32 sampleRate;
            drwav_uint64 totalPcm;
            drmp3_int16* pcmData =
                drwav_open_memory_and_read_pcm_frames_s16(strem2, sampleRaw->BufferSize, &channels, &sampleRate,
       &totalPcm, NULL); sampleC->size = totalPcm; sampleC->sampleAddr = (uint8_t*)pcmData; sampleC->codec = CODEC_S16;

            sampleC->loop = new AdpcmLoop;
            sampleC->loop->start = 0;
            sampleC->loop->end = sampleC->size - 1;
            sampleC->loop->count = 0;
            sampleC->sampleRateMagicValue = 'RIFF';
            sampleC->sampleRate = sampleRate;

            cachedCustomSFs[path] = sampleC;
            return sampleC;
        } else if (entry.ext == "mp3") {
            drmp3_config mp3Info;
            drmp3_uint64 totalPcm;
            drmp3_int16* pcmData =
                drmp3_open_memory_and_read_pcm_frames_s16(strem2, sampleRaw->BufferSize, &mp3Info, &totalPcm, NULL);

            sampleC->size = totalPcm * mp3Info.channels * sizeof(short);
            sampleC->sampleAddr = (uint8_t*)pcmData;
            sampleC->codec = CODEC_S16;

            sampleC->loop = new AdpcmLoop;
            sampleC->loop->start = 0;
            sampleC->loop->end = sampleC->size;
            sampleC->loop->count = 0;
            sampleC->sampleRateMagicValue = 'RIFF';
            sampleC->sampleRate = mp3Info.sampleRate;

            cachedCustomSFs[path] = sampleC;
            return sampleC;
        }

        return nullptr;
    */
}

extern "C" SoundFontSample* ResourceMgr_LoadAudioSample(const char* path) {
    return (SoundFontSample*)ResourceGetDataByName(path);
}

extern "C" SoundFont* ResourceMgr_LoadAudioSoundFont(const char* path) {
    return (SoundFont*)ResourceGetDataByName(path);
}
#endif
extern "C" int ResourceMgr_OTRSigCheck(char* imgData) {
    uintptr_t i = (uintptr_t)(imgData);

    // if (i == 0xD9000000 || i == 0xE7000000 || (i & 1) == 1)
    if ((i & 1) == 1)
        return 0;

    // if ((i & 0xFF000000) != 0xAB000000 && (i & 0xFF000000) != 0xCD000000 && i != 0) {
    if (i != 0) {
        if (imgData[0] == '_' && imgData[1] == '_' && imgData[2] == 'O' && imgData[3] == 'T' && imgData[4] == 'R' &&
            imgData[5] == '_' && imgData[6] == '_')
            return 1;
    }

    return 0;
}

extern "C" AnimationHeaderCommon* ResourceMgr_LoadAnimByName(const char* path) {
    return (AnimationHeaderCommon*)ResourceGetDataByName(path);
}

extern "C" SkeletonHeader* ResourceMgr_LoadSkeletonByName(const char* path, SkelAnime* skelAnime) {
    std::string pathStr = std::string(path);
    static const std::string sOtr = "__OTR__";

    if (pathStr.starts_with(sOtr)) {
        pathStr = pathStr.substr(sOtr.length());
    }

    bool isAlt = CVarGetInteger("gAltAssets", 0);

    if (isAlt) {
        pathStr = LUS::IResource::gAltAssetPrefix + pathStr;
    }

    SkeletonHeader* skelHeader = (SkeletonHeader*)ResourceGetDataByName(pathStr.c_str());

    // If there isn't an alternate model, load the regular one
    if (isAlt && skelHeader == NULL) {
        skelHeader = (SkeletonHeader*)ResourceGetDataByName(path);
    }

    // This function is only called when a skeleton is initialized.
    // Therefore we can take this oppurtunity to take note of the Skeleton that is created...
    if (skelAnime != nullptr) {
        auto stringPath = std::string(path);
        //LUS::SkeletonPatcher::RegisterSkeleton(stringPath, skelAnime);
    }

    return skelHeader;
}

extern "C" void ResourceMgr_UnregisterSkeleton(SkelAnime* skelAnime) {
    if (skelAnime != nullptr)
        LUS::SkeletonPatcher::UnregisterSkeleton(skelAnime);
}

extern "C" void ResourceMgr_ClearSkeletons(SkelAnime* skelAnime) {
    if (skelAnime != nullptr)
        LUS::SkeletonPatcher::ClearSkeletons();
}

extern "C" s32* ResourceMgr_LoadCSByName(const char* path) {
    return (s32*)GetResourceDataByNameHandlingMQ(path);
}

std::filesystem::path GetSaveFile(std::shared_ptr<LUS::Config> Conf) {
    const std::string fileName =
        Conf->GetString("Game.SaveName", LUS::Context::GetPathRelativeToAppDirectory("oot_save.sav"));
    std::filesystem::path saveFile = std::filesystem::absolute(fileName);

    if (!exists(saveFile.parent_path())) {
        create_directories(saveFile.parent_path());
    }

    return saveFile;
}

std::filesystem::path GetSaveFile() {
    const std::shared_ptr<LUS::Config> pConf = OTRGlobals::Instance->context->GetConfig();

    return GetSaveFile(pConf);
}

void OTRGlobals::CheckSaveFile(size_t sramSize) const {
    const std::shared_ptr<LUS::Config> pConf = Instance->context->GetConfig();

    std::filesystem::path savePath = GetSaveFile(pConf);
    std::fstream saveFile(savePath, std::fstream::in | std::fstream::out | std::fstream::binary);
    if (saveFile.fail()) {
        saveFile.open(savePath, std::fstream::in | std::fstream::out | std::fstream::binary | std::fstream::app);
        for (int i = 0; i < sramSize; ++i) {
            saveFile.write("\0", 1);
        }
    }
    saveFile.close();
}

//extern "C" void Ctx_ReadSaveFile(uintptr_t addr, void* dramAddr, size_t size) {
//    SaveManager::ReadSaveFile(GetSaveFile(), addr, dramAddr, size);
//}

//extern "C" void Ctx_WriteSaveFile(uintptr_t addr, void* dramAddr, size_t size) {
//    SaveManager::WriteSaveFile(GetSaveFile(), addr, dramAddr, size);
//}

std::wstring StringToU16(const std::string& s) {
    std::vector<unsigned long> result;
    size_t i = 0;
    while (i < s.size()) {
        unsigned long uni;
        size_t nbytes;
        bool error = false;
        unsigned char c = s[i++];
        if (c < 0x80) { // ascii
            uni = c;
            nbytes = 0;
        } else if (c <= 0xBF) { // assuming kata/hiragana delimiter
            nbytes = 0;
            uni = '\1';
        } else if (c <= 0xDF) {
            uni = c & 0x1F;
            nbytes = 1;
        } else if (c <= 0xEF) {
            uni = c & 0x0F;
            nbytes = 2;
        } else if (c <= 0xF7) {
            uni = c & 0x07;
            nbytes = 3;
        }
        for (size_t j = 0; j < nbytes; ++j) {
            unsigned char c = s[i++];
            uni <<= 6;
            uni += c & 0x3F;
        }
        if (uni != '\1')
            result.push_back(uni);
    }
    std::wstring utf16;
    for (size_t i = 0; i < result.size(); ++i) {
        unsigned long uni = result[i];
        if (uni <= 0xFFFF) {
            utf16 += (wchar_t)uni;
        } else {
            uni -= 0x10000;
            utf16 += (wchar_t)((uni >> 10) + 0xD800);
            utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
        }
    }
    return utf16;
}

int CopyStringToCharBuffer(const std::string& inputStr, char* buffer, const int maxBufferSize) {
    if (!inputStr.empty()) {
        // Prevent potential horrible overflow due to implicit conversion of maxBufferSize to an unsigned. Prevents
        // negatives.
        memset(buffer, 0, std::max<int>(0, maxBufferSize));
        // Gaurentee that this value will be greater than 0, regardless of passed variables.
        const int copiedCharLen = std::min<int>(std::max<int>(0, maxBufferSize - 1), inputStr.length());
        memcpy(buffer, inputStr.c_str(), copiedCharLen);
        return copiedCharLen;
    }

    return 0;
}

extern "C" void OTRGfxPrint(const char* str, void* printer, void (*printImpl)(void*, char)) {
    const std::vector<uint32_t> hira1 = {
        u'を', u'ぁ', u'ぃ', u'ぅ', u'ぇ', u'ぉ', u'ゃ', u'ゅ', u'ょ', u'っ', u'-',  u'あ', u'い',
        u'う', u'え', u'お', u'か', u'き', u'く', u'け', u'こ', u'さ', u'し', u'す', u'せ', u'そ',
    };

    const std::vector<uint32_t> hira2 = {
        u'た', u'ち', u'つ', u'て', u'と', u'な', u'に', u'ぬ', u'ね', u'の', u'は', u'ひ', u'ふ', u'へ', u'ほ', u'ま',
        u'み', u'む', u'め', u'も', u'や', u'ゆ', u'よ', u'ら', u'り', u'る', u'れ', u'ろ', u'わ', u'ん', u'゛', u'゜',
    };

    std::wstring wstr = StringToU16(str);

    for (const auto& c : wstr) {
        unsigned char convt = ' ';
        if (c < 0x80) {
            printImpl(printer, c);
        } else if (c >= u'｡' && c <= u'ﾟ') { // katakana
            printImpl(printer, c - 0xFEC0);
        } else {
            auto it = std::find(hira1.begin(), hira1.end(), c);
            if (it != hira1.end()) { // hiragana block 1
                printImpl(printer, 0x88 + std::distance(hira1.begin(), it));
            }

            auto it2 = std::find(hira2.begin(), hira2.end(), c);
            if (it2 != hira2.end()) { // hiragana block 2
                printImpl(printer, 0xe0 + std::distance(hira2.begin(), it2));
            }
        }
    }
}

extern "C" uint32_t OTRGetCurrentWidth() {
    return OTRGlobals::Instance->context->GetWindow()->GetWidth();
}

extern "C" uint32_t OTRGetCurrentHeight() {
    return OTRGlobals::Instance->context->GetWindow()->GetHeight();
}

Color_RGB8 GetColorForControllerLED() {
    #if 0
    auto brightness = CVarGetFloat("gLedBrightness", 1.0f) / 1.0f;
    Color_RGB8 color = { 0, 0, 0 };
    if (brightness > 0.0f) {
        LEDColorSource source =
            static_cast<LEDColorSource>(CVarGetInteger("gLedColorSource", LED_SOURCE_TUNIC_ORIGINAL));
        bool criticalOverride = CVarGetInteger("gLedCriticalOverride", 1);
        if (gPlayState && (source == LED_SOURCE_TUNIC_ORIGINAL || source == LED_SOURCE_TUNIC_COSMETICS)) {
            switch (CUR_EQUIP_VALUE(EQUIP_TUNIC) - 1) {
                case PLAYER_TUNIC_KOKIRI:
                    color = source == LED_SOURCE_TUNIC_COSMETICS
                                ? CVarGetColor24("gCosmetics.Link_KokiriTunic.Value", kokiriColor)
                                : kokiriColor;
                    break;
                case PLAYER_TUNIC_GORON:
                    color = source == LED_SOURCE_TUNIC_COSMETICS
                                ? CVarGetColor24("gCosmetics.Link_GoronTunic.Value", goronColor)
                                : goronColor;
                    break;
                case PLAYER_TUNIC_ZORA:
                    color = source == LED_SOURCE_TUNIC_COSMETICS
                                ? CVarGetColor24("gCosmetics.Link_ZoraTunic.Value", zoraColor)
                                : zoraColor;
                    break;
            }
        }
        if (source == LED_SOURCE_CUSTOM) {
            color = CVarGetColor24("gLedPort1Color", { 255, 255, 255 });
        }
        if (criticalOverride || source == LED_SOURCE_HEALTH) {
            if (HealthMeter_IsCritical()) {
                color = { 0xFF, 0, 0 };
            } else if (source == LED_SOURCE_HEALTH) {
                if (gSaveContext.health / gSaveContext.healthCapacity <= 0.4f) {
                    color = { 0xFF, 0xFF, 0 };
                } else {
                    color = { 0, 0xFF, 0 };
                }
            }
        }
        color.r = color.r * brightness;
        color.g = color.g * brightness;
        color.b = color.b * brightness;
    }
    #endif
    return { 0, 0, 0 };
}

extern "C" void OTRControllerCallback(uint8_t rumble) {
    auto physicalDevice = LUS::Context::GetInstance()->GetControlDeck()->GetDeviceFromPortIndex(0);

    if (physicalDevice->CanSetLed()) {
        // We call this every tick, SDL accounts for this use and prevents driver spam
        // https://github.com/libsdl-org/SDL/blob/f17058b562c8a1090c0c996b42982721ace90903/src/joystick/SDL_joystick.c#L1114-L1144
        physicalDevice->SetLedColor(0, GetColorForControllerLED());
    }

    physicalDevice->SetRumble(0, rumble);
}

extern "C" float OTRGetAspectRatio() {
    return gfx_current_dimensions.aspect_ratio;
}

extern "C" float OTRGetDimensionFromLeftEdge(float v) {
    return (SCREEN_WIDTH / 2 - SCREEN_HEIGHT / 2 * OTRGetAspectRatio() + (v));
}

extern "C" float OTRGetDimensionFromRightEdge(float v) {
    return (SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 * OTRGetAspectRatio() - (SCREEN_WIDTH - v));
}

f32 floorf(f32 x);
f32 ceilf(f32 x);

extern "C" int16_t OTRGetRectDimensionFromLeftEdge(float v) {
    return ((int)floorf(OTRGetDimensionFromLeftEdge(v)));
}

extern "C" int16_t OTRGetRectDimensionFromRightEdge(float v) {
    return ((int)ceilf(OTRGetDimensionFromRightEdge(v)));
}

extern "C" int AudioPlayer_Buffered(void) {
    return AudioPlayerBuffered();
}

extern "C" int AudioPlayer_GetDesiredBuffered(void) {
    return AudioPlayerGetDesiredBuffered();
}

extern "C" void AudioPlayer_Play(const uint8_t* buf, uint32_t len) {
    AudioPlayerPlayFrame(buf, len);
}

extern "C" int Controller_ShouldRumble(size_t slot) {
    auto controlDeck = LUS::Context::GetInstance()->GetControlDeck();

    if (slot < controlDeck->GetNumConnectedPorts()) {
        auto physicalDevice = controlDeck->GetDeviceFromPortIndex(slot);

        if (physicalDevice->GetProfile(slot)->UseRumble && physicalDevice->CanRumble()) {
            return 1;
        }
    }

    return 0;
}
