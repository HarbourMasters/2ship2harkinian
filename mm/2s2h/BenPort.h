#ifndef OTR_GLOBALS_H
#define OTR_GLOBALS_H

#pragma once

#define GAME_REGION_NTSC 0
#define GAME_REGION_PAL 1

#define GAME_PLATFORM_N64 0
#define GAME_PLATFORM_GC 1

#define MM_NTSC_US_10 0x5354631C
#define MM_NTSC_US_GC 0xB443EB08
#define MM_NTSC_JP_GC 0x8473D0C1

#ifdef __cplusplus
#include <Context.h>

#include <vector>

const std::string customMessageTableID = "BaseGameOverrides";
const std::string appShortName = "2ship";

class OTRGlobals {
  public:
    static OTRGlobals* Instance;

    std::shared_ptr<Ship::Context> context;

    OTRGlobals();
    ~OTRGlobals();

    bool HasMasterQuest();
    bool HasOriginal();
    uint32_t GetInterpolationFPS();
    std::shared_ptr<std::vector<std::string>> ListFiles(std::string path);

  private:
    void CheckSaveFile(size_t sramSize) const;
    bool hasMasterQuest;
    bool hasOriginal;
};

uint32_t IsGameMasterQuest();
#endif

#ifndef __cplusplus
#include <z64bgcheck.h>
#include <z64camera.h>
#include <z64game.h>
#include <z64scene.h>
#include <z64skin.h>
void InitOTR(void);
void DeinitOTR(void);
void VanillaItemTable_Init();
void OTRAudio_Init();
void OTRMessage_Init();
void OTRJPFontMessage_Init();
void InitAudio();
void Graph_StartFrame();
void Graph_ProcessGfxCommands(Gfx* commands);
void Graph_ProcessFrame(void (*run_one_game_iter)(void));
void OTRLogString(const char* src);
void OTRGfxPrint(const char* str, void* printer, void (*printImpl)(void*, char));
void OTRGetPixelDepthPrepare(float x, float y);
uint16_t OTRGetPixelDepth(float x, float y);
int32_t OTRGetLastScancode();
uint32_t ResourceMgr_GetNumGameVersions();
uint32_t ResourceMgr_GetGameVersion(int index);
uint32_t ResourceMgr_GetGamePlatform(int index);
uint32_t ResourceMgr_GetGameRegion(int index);
uint32_t ResourceMgr_GetGameDefaultLanguage(int index);
void ResourceMgr_LoadDirectory(const char* resName);
char** ResourceMgr_ListFiles(const char* searchMask, int* resultSize);
uint8_t ResourceMgr_FileExists(const char* resName);
void ResourceMgr_LoadFile(const char* resName);
char* ResourceMgr_LoadFileFromDisk(const char* filePath);
uint8_t ResourceMgr_ResourceIsBackground(char* texPath);
char* ResourceMgr_LoadJPEG(char* data, size_t dataSize);
uint16_t ResourceMgr_LoadTexWidthByName(char* texPath);
uint16_t ResourceMgr_LoadTexHeightByName(char* texPath);
CollisionHeader* ResourceMgr_LoadColByName(const char* path);
AnimatedMaterial* ResourceMgr_LoadAnimatedMatByName(const char* path);
char* ResourceMgr_LoadTexOrDListByName(const char* filePath);
char* ResourceMgr_LoadIfDListByName(const char* filePath);
char* ResourceMgr_LoadPlayerAnimByName(const char* animPath);
AnimationHeaderCommon* ResourceMgr_LoadAnimByName(const char* path);
char* ResourceMgr_GetNameByCRC(uint64_t crc, char* alloc);
Gfx* ResourceMgr_LoadGfxByCRC(uint64_t crc);
Gfx* ResourceMgr_LoadGfxByName(const char* path);
void ResourceMgr_PatchGfxByName(const char* path, const char* patchName, int index, Gfx instruction);
void ResourceMgr_UnpatchGfxByName(const char* path, const char* patchName);
char* ResourceMgr_LoadArrayByNameAsVec3s(const char* path);
char* ResourceMgr_LoadArrayByName(const char* path);
size_t ResourceMgr_GetArraySizeByName(const char* path);
Vtx* ResourceMgr_LoadVtxByCRC(uint64_t crc);
char* ResourceMgr_LoadVtxArrayByName(const char* path);
size_t ResourceMgr_GetVtxArraySizeByName(const char* path);
Vtx* ResourceMgr_LoadVtxByName(char* path);

KeyFrameSkeleton* ResourceMgr_LoadKeyFrameSkelByName(const char* path);
KeyFrameAnimation* ResourceMgr_LoadKeyFrameAnimByName(const char* path);

void Ctx_ReadSaveFile(uintptr_t addr, void* dramAddr, size_t size);
void Ctx_WriteSaveFile(uintptr_t addr, void* dramAddr, size_t size);

uint64_t GetPerfCounter();
struct SkeletonHeader* ResourceMgr_LoadSkeletonByName(const char* path, SkelAnime* skelAnime);
void ResourceMgr_UnregisterSkeleton(SkelAnime* skelAnime);
void ResourceMgr_ClearSkeletons();
s32* ResourceMgr_LoadCSByName(const char* path);
int ResourceMgr_OTRSigCheck(char* imgData);
uint64_t osGetTime(void);
uint32_t osGetCount(void);
uint64_t GetFrequency();
uint32_t OTRGetCurrentWidth(void);
uint32_t OTRGetCurrentHeight(void);
float OTRGetAspectRatio(void);
int32_t OTRConvertHUDXToScreenX(int32_t v);
float OTRGetDimensionFromLeftEdge(float v);
float OTRGetDimensionFromRightEdge(float v);
int16_t OTRGetRectDimensionFromLeftEdge(float v);
int16_t OTRGetRectDimensionFromRightEdge(float v);
uint32_t OTRGetGameRenderWidth();
uint32_t OTRGetGameRenderHeight();
int AudioPlayer_Buffered(void);
int AudioPlayer_GetDesiredBuffered(void);
void AudioPlayer_Play(const uint8_t* buf, uint32_t len);
void AudioMgr_CreateNextAudioBuffer(s16* samples, u32 num_samples);
int Controller_ShouldRumble(size_t slot);
void Controller_BlockGameInput();
void Controller_UnblockGameInput();
void Overlay_DisplayText(float duration, const char* text);
void Overlay_DisplayText_Seconds(int seconds, const char* text);

void Gfx_RegisterBlendedTexture(const char* name, u8* mask, u8* replacement);
void Gfx_UnregisterBlendedTexture(const char* name);
void Gfx_TextureCacheDelete(const uint8_t* texAddr);
void CheckTracker_OnMessageClose();

int32_t GetGIID(uint32_t itemID);
#endif

#ifdef __cplusplus
extern "C" {
#endif
uint64_t GetUnixTimestamp();
#ifdef __cplusplus
};
#endif

#endif
