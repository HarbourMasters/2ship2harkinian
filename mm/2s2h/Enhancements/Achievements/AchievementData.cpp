#include "AchievementData.h"
#include "2s2h/BenPort.h"     // For CHECK_WEEKEVENTREG, GET_SAVE_INVENTORY_QUEST_ITEMS, etc.
#include "2s2h/Rando/Rando.h" // Added for IS_RANDO, RANDO_SAVE_CHECKS, etc.
#include <z64scene.h>         // For SCENE_*
#include <z64item.h>          // For ITEM_*
#include <z64save.h>          // For gSaveContext access
#include <z64.h>              // Added for gPlayState
#include "assets/archives/schedule_dma_static/schedule_dma_static_yar.h" // Added for Bomber's Notebook icon textures

// Wrap C includes in extern "C"
extern "C" {
#include <variables.h> // For gSaveContext, gPlayState
#include <functions.h> // Potentially needed?
// Keep z64item.h and z64save.h here for their defines/structs used in lambdas
#include <z64item.h> // ITEM_*, gItemIcons, gItemSlots, FLAG_QUEST_ITEM
#include <z64save.h> // gSaveContext, WEEKEVENTREG_*
}

// Define the global map to store all achievement data
std::map<AchievementId, AchievementStaticData> AllAchievementData = {
    // Bomber's Notebook (Vanilla)
    { AID_BN_HONORARY_MEMBER,
      { .id = AID_BN_HONORARY_MEMBER,
        .name = "Honorary Member",
        .description = "Fully complete the entry for the Bomber's Gang",
        .iconPath = (const char*)gBombersNotebookPhotoBombersTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 2,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_LEARNED_SECRET_CODE))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BOMBERS_NOTEBOOK))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_FAITHFUL_BRIDE,
      { .id = AID_BN_FAITHFUL_BRIDE,
        .name = "Faithful Bride",
        .description = "Fully complete the entry for Anju",
        .iconPath = (const char*)gBombersNotebookPhotoAnjuTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 5,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROOM_KEY))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_MIDNIGHT_MEETING))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_LETTER_TO_KAFEI))
                progress++; // Anju gives letter
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DELIVERED_PENDANT_OF_MEMORIES))
                progress++; // Link gives pendant to Anju
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_COUPLES_MASK))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_MINI_HE,
      { .id = AID_BN_MINI_HE,
        .name = "Mini-He",
        .description = "Fully complete the entry for Kafei",
        .iconPath = (const char*)gBombersNotebookPhotoKafeiTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 4,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_TO_MEET_KAFEI))
                progress++; // Met Kafei in hideout
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_PENDANT_OF_MEMORIES))
                progress++; // Kafei gives pendant
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_ESCAPED_SAKONS_HIDEOUT))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_COUPLES_MASK))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_AMBIGUOUS_ALLEGIANCE,
      { .id = AID_BN_AMBIGUOUS_ALLEGIANCE,
        .name = "Ambiguous Allegiance",
        .description = "Fully complete the entry for the Curiosity Shop Owner",
        .iconPath = (const char*)gBombersNotebookPhotoCuriosityShopManTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 3,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KEATON_MASK))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_PRIORITY_MAIL))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ALL_NIGHT_MASK))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_FRAIL_DID,
      { .id = AID_BN_FRAIL_DID,
        .name = "Frail D.I.D.",
        .description = "Fully complete the entry for the Bomb Shop Owner's Mother",
        .iconPath = (const char*)gBombersNotebookPhotoBombShopLadyTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BLAST_MASK } },
    { AID_BN_BRAVE_LITTLE_ARCHER,
      { .id = AID_BN_BRAVE_LITTLE_ARCHER,
        .name = "Brave Little Archer",
        .description = "Fully complete the entry for Romani",
        .iconPath = (const char*)gBombersNotebookPhotoRomaniTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 3,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_PROMISED_TO_HELP_WITH_THEM))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DEFENDED_AGAINST_THEM))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_MILK_BOTTLE))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_MATURE_RANCH_HAND,
      { .id = AID_BN_MATURE_RANCH_HAND,
        .name = "Mature Ranch Hand",
        .description = "Fully complete the entry for Cremia",
        .iconPath = (const char*)gBombersNotebookPhotoCremiaTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 2,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_ESCORTED_CREMIA))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROMANIS_MASK))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_DROWNING_IN_RESPONSIBILITIES,
      { .id = AID_BN_DROWNING_IN_RESPONSIBILITIES,
        .name = "Drowning in Responsibilities",
        .description = "Fully complete the entry for Mayor Doutour",
        .iconPath = (const char*)gBombersNotebookPhotoMayorDotourTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_MAYOR_HP } },
    { AID_BN_CONCERNED_MOTHER,
      { .id = AID_BN_CONCERNED_MOTHER,
        .name = "Concerned Mother",
        .description = "Fully complete the entry for Madame Aroma",
        .iconPath = (const char*)gBombersNotebookPhotoMadameAromaTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 2,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KAFEIS_MASK))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_DELIVERED_PRIORITY_MAIL))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_LOOKING_FOR_VOLUNTEERS,
      { .id = AID_BN_LOOKING_FOR_VOLUNTEERS,
        .name = "Looking for Volunteers",
        .description = "Fully complete the entry for Toto",
        .iconPath = (const char*)gBombersNotebookPhotoTotoTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_CIRCUS_LEADERS_MASK } },
    { AID_BN_BREAKAWAY_BROTHER,
      { .id = AID_BN_BREAKAWAY_BROTHER,
        .name = "Breakaway Brother",
        .description = "Fully complete the entry for Gorman",
        .iconPath = (const char*)gBombersNotebookPhotoGormanTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_MET_GORMAN } },
    { AID_BN_ALWAYS_MAKES_HIS_APPOINTED_DELIVERIES,
      { .id = AID_BN_ALWAYS_MAKES_HIS_APPOINTED_DELIVERIES,
        .name = "Always Makes His Appointed Deliveries",
        .description = "Fully complete the entry for the Postman",
        .iconPath = (const char*)gBombersNotebookPhotoPostmanTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 2,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMAN_HP))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_POSTMANS_HAT))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_PREPARE_FOR_TROUBLE,
      { .id = AID_BN_PREPARE_FOR_TROUBLE,
        .name = "Prepare For Trouble, and Make it Double!",
        .description = "Fully complete the entry for the Rosa Sisters",
        .iconPath = (const char*)gBombersNotebookPhotoRosaSistersTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_ROSA_SISTERS_HP } },
    { AID_BN_YOU_MISSED_A_SPOT,
      { .id = AID_BN_YOU_MISSED_A_SPOT,
        .name = "You Missed a Spot",
        .description = "Fully complete the entry for ???",
        .iconPath = (const char*)gBombersNotebookPhotoToiletHandTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_TOILET_HAND_HP } },
    { AID_BN_STORYTELLER,
      { .id = AID_BN_STORYTELLER,
        .name = "Storyteller",
        .description = "Fully complete the entry for Anju's Grandmother",
        .iconPath = (const char*)gBombersNotebookPhotoAnjusGrandmotherTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 2,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GRANDMA_SHORT_STORY_HP))
                progress++;
            if (CHECK_WEEKEVENTREG(WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GRANDMA_LONG_STORY_HP))
                progress++;
            return progress;
        },
        .unlockOnTargetMet = true } },
    { AID_BN_PASSING_ON_THE_GROOVE,
      { .id = AID_BN_PASSING_ON_THE_GROOVE,
        .name = "Passing on the Groove",
        .description = "Fully complete the entry for Kamaro",
        .iconPath = (const char*)gBombersNotebookPhotoKamaroTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_KAMAROS_MASK } },
    { AID_BN_CARETAKER_OF_THE_FOWL,
      { .id = AID_BN_CARETAKER_OF_THE_FOWL,
        .name = "Caretaker of the Fowl",
        .description = "Fully complete the entry for Grog",
        .iconPath = (const char*)gBombersNotebookPhotoGrogTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BUNNY_HOOD } },
    { AID_BN_THERES_THREE_OF_YOU,
      { .id = AID_BN_THERES_THREE_OF_YOU,
        .name = "There's THREE of you?!",
        .description = "Fully complete the entry for the Gorman Brothers",
        .iconPath = (const char*)gBombersNotebookPhotoGormanBrothersTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_GAROS_MASK } },
    { AID_BN_PERFECTING_THE_ART,
      { .id = AID_BN_PERFECTING_THE_ART,
        .name = "Perfecting the Art of Standing Perfectly Still",
        .description = "Fully complete the entry for Shiro",
        .iconPath = (const char*)gBombersNotebookPhotoShiroTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_STONE_MASK } },
    { AID_BN_WINDMILL_MAN,
      { .id = AID_BN_WINDMILL_MAN,
        .name = "Windmill Man",
        .description = "Fully complete the entry for Guru-Guru",
        .iconPath = (const char*)gBombersNotebookPhotoGuruGuruTex,
        .isSecret = false,
        .gamerscore = 5,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_BOMBERS_NOTEBOOK_EVENT_RECEIVED_BREMEN_MASK } },
    { AID_BN_LOCAL_HERO,
      { .id = AID_BN_LOCAL_HERO,
        .name = "Local Hero",
        .description = "100% the Bomber\'s Notebook",
        .iconPath = (const char*)gBombersNotebookEntryIconRibbonTex,
        .isSecret = true,
        .gamerscore = 25,
        .category = AchievementCategory::VANILLA,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .hasProgressTracking = true,
        .targetProgress = 20,
        .getCurrentProgress = []() -> s32 {
            s32 progress = 0;
            const AchievementId bnAchievementIds[] = { AID_BN_HONORARY_MEMBER,
                                                       AID_BN_FAITHFUL_BRIDE,
                                                       AID_BN_MINI_HE,
                                                       AID_BN_AMBIGUOUS_ALLEGIANCE,
                                                       AID_BN_FRAIL_DID,
                                                       AID_BN_BRAVE_LITTLE_ARCHER,
                                                       AID_BN_MATURE_RANCH_HAND,
                                                       AID_BN_DROWNING_IN_RESPONSIBILITIES,
                                                       AID_BN_CONCERNED_MOTHER,
                                                       AID_BN_LOOKING_FOR_VOLUNTEERS,
                                                       AID_BN_BREAKAWAY_BROTHER,
                                                       AID_BN_ALWAYS_MAKES_HIS_APPOINTED_DELIVERIES,
                                                       AID_BN_PREPARE_FOR_TROUBLE,
                                                       AID_BN_YOU_MISSED_A_SPOT,
                                                       AID_BN_STORYTELLER,
                                                       AID_BN_PASSING_ON_THE_GROOVE,
                                                       AID_BN_CARETAKER_OF_THE_FOWL,
                                                       AID_BN_THERES_THREE_OF_YOU,
                                                       AID_BN_PERFECTING_THE_ART,
                                                       AID_BN_WINDMILL_MAN };
            for (AchievementId id : bnAchievementIds) {
                if (id > AID_UNKNOWN && id < AID_MAX) { // Basic bounds check
                    if (gSaveContext.save.shipSaveInfo.achievements.achievementData[id].unlocked) {
                        progress++;
                    }
                }
            }
            return progress;
        },
        .unlockOnTargetMet = true } },
    // End of Bomber's Notebook (Vanilla)

};

// --- Helper Function Definitions ---

// Helper to get all AchievementStaticData relevant to a specific FlagSet event
std::vector<AchievementStaticData> GetAchievementsFromFlagSet(FlagType flagType, u32 flag) {
    std::vector<AchievementStaticData> relevantAchievements;
    for (auto const& [key, val] : AllAchievementData) {
        if (val.triggerType != AchievementTriggerType::OnFlagSet) {
            continue; // Only interested in OnFlagSet achievements
        }

        // Handle specific RANDO_INF case
        if (flagType == FLAG_RANDO_INF && val.checkFlagType == FLAG_RANDO_INF) {
            relevantAchievements.push_back(val);
            continue; // Added, move to next achievement
        }

        // Handle non-RANDO_INF cases
        if (val.checkFlagType == flagType) {
            // For progress tracking achievements, matching the type is enough to trigger an update check
            if (val.hasProgressTracking) {
                relevantAchievements.push_back(val);
                continue; // Added, move to next achievement
            }
            // For non-progress tracking, require an exact flag match
            else if (val.checkFlag == flag) { // Combined !val.hasProgressTracking implicitly
                relevantAchievements.push_back(val);
                // No continue needed here
            }
        }
    }
    return relevantAchievements;
}

// Helper to get all AchievementStaticData relevant to a specific ItemGive event
std::vector<AchievementStaticData> GetAchievementsFromItemGive() {
    std::vector<AchievementStaticData> relevantAchievements;
    for (auto const& [key, val] : AllAchievementData) {
        if (val.triggerType == AchievementTriggerType::OnItemGive) {
            relevantAchievements.push_back(val);
        }
    }
    return relevantAchievements;
}

// Helper to get all AchievementStaticData relevant to a specific SceneInit event
std::vector<AchievementStaticData> GetAchievementsFromSceneInit(s16 sceneId) {
    std::vector<AchievementStaticData> relevantAchievements;
    for (auto const& [key, val] : AllAchievementData) {
        if (val.triggerType == AchievementTriggerType::OnSceneInit &&
            (val.checkSceneId == sceneId || val.checkSceneId == SCENE_ID_CHECK_ALWAYS)) {
            relevantAchievements.push_back(val);
        }
    }
    return relevantAchievements;
}

// Helper for AfterEndOfCycleSave (no params)
std::vector<AchievementStaticData> GetAchievementsFromEndOfCycleSave() {
    std::vector<AchievementStaticData> relevantAchievements;
    for (auto const& [key, val] : AllAchievementData) {
        if (val.triggerType == AchievementTriggerType::AfterEndOfCycleSave) {
            relevantAchievements.push_back(val);
        }
    }
    return relevantAchievements;
}

// Helper to get all AchievementStaticData relevant to a specific Vanilla Behavior hook ID
std::vector<AchievementStaticData> GetAchievementsFromVanillaBehavior(GIVanillaBehavior vbHookId) {
    std::vector<AchievementStaticData> relevantAchievements;
    for (auto const& [key, val] : AllAchievementData) {
        if (val.triggerType == AchievementTriggerType::ShouldVanillaBehavior && val.checkVbHookId == vbHookId) {
            relevantAchievements.push_back(val);
        }
    }
    return relevantAchievements;
}