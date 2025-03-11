#ifndef SHIP_UTILS_H
#define SHIP_UTILS_H

#include <libultraship/libultraship.h>
#include "PR/ultratypes.h"

#ifdef __cplusplus

void LoadGuiTextures();
std::string convertEnumToReadableName(const std::string& input);
extern std::vector<const char*> digitList;
extern std::vector<int16_t> orderedInventoryItemList;
extern const char* fairyIcons[4];
extern std::map<int16_t, int16_t> questToItemMap;
extern ImVec4 Ship_SongColors(int16_t itemID);

extern "C" {
#endif

struct PlayState;
struct Actor;

f32 Ship_GetExtendedAspectRatioMultiplier();
void Ship_ExtendedCullingActorAdjustProjectedZ(Actor* actor);
void Ship_ExtendedCullingActorAdjustProjectedX(Actor* actor);
void Ship_ExtendedCullingActorRestoreProjectedPos(PlayState* play, Actor* actor);
const char* Ship_GetSceneName(s16 sceneId);
bool Ship_IsCStringEmpty(const char* str);
void Ship_CreateQuadVertexGroup(Vtx* vtxList, s32 xStart, s32 yStart, s32 width, s32 height, u8 flippedH);
f32 Ship_GetCharFontWidthNES(u8 character);
TexturePtr Ship_GetCharFontTextureNES(u8 character);
void Ship_Random_Seed(u32 seed);
s32 Ship_Random(s32 min, s32 max);
int16_t findQuestByItem(int16_t itemId);

#ifdef __cplusplus
}
#endif

#endif // SHIP_UTILS_H
