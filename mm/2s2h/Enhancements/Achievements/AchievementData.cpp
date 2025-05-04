#include "AchievementData.h"
#include "2s2h/BenPort.h" // For CHECK_WEEKEVENTREG, GET_SAVE_INVENTORY_QUEST_ITEMS, etc.
#include "2s2h/Rando/Rando.h" // Added for IS_RANDO, RANDO_SAVE_CHECKS, etc.
#include <z64scene.h>     // For SCENE_*
#include <z64item.h>      // For ITEM_*
#include <z64save.h>      // For gSaveContext access
#include <z64.h>          // Added for gPlayState

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
    // --- Data translated from AchievementRegistration.cpp ---

    // Test Achievement (Now uses OnSceneInit with additional condition)
    { AID_FIRST_STEPS, {
        .id = AID_FIRST_STEPS, .name = "Beyond the Town Walls",
        .description = "Leave Clock Town and enter Termina Field for the first time",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkSceneId = SCENE_00KEIKOKU,
        .additionalCondition = []{ return CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_TERMINA_FIELD); }
    }},

    // Boss achievements
    { AID_DEFEAT_ODOLWA, {
        .id = AID_DEFEAT_ODOLWA, .name = "Jungle Warrior", .description = "Defeat Odolwa, Masked Jungle Warrior",
        .iconPath = (const char*)gItemIcons[ITEM_REMAINS_ODOLWA], .isSecret = false, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE
    }},
    { AID_DEFEAT_GOHT, {
        .id = AID_DEFEAT_GOHT, .name = "Mountain Racer", .description = "Defeat Goht, Masked Mechanical Monster",
        .iconPath = (const char*)gItemIcons[ITEM_REMAINS_GOHT], .isSecret = false, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE
    }},
    { AID_DEFEAT_GYORG, {
        .id = AID_DEFEAT_GYORG, .name = "Ocean Conqueror", .description = "Defeat Gyorg, Gargantuan Masked Fish",
        .iconPath = (const char*)gItemIcons[ITEM_REMAINS_GYORG], .isSecret = false, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE
    }},
    { AID_DEFEAT_TWINMOLD, {
        .id = AID_DEFEAT_TWINMOLD, .name = "Desert Exterminator", .description = "Defeat Twinmold, Giant Masked Insects",
        .iconPath = (const char*)gItemIcons[ITEM_REMAINS_TWINMOLD], .isSecret = false, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE
    }},
    { AID_DEFEAT_MAJORA, {
         .id = AID_DEFEAT_MAJORA, .name = "Savior of Termina", .description = "Defeat Majora and save Termina",
         .iconPath = (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], .isSecret = false, .gamerscore = 50, .category = AchievementCategory::BOTH,
         .triggerType = AchievementTriggerType::ShouldVanillaBehavior,
         .checkVbHookId = VB_SETUP_TRANSITION,
         .additionalCondition = []{ return (gPlayState != nullptr && gPlayState->nextEntrance == 0x5400); }
     }},

    // Mask achievements (Transformation)
    { AID_COLLECT_DEKU_MASK, {
        .id = AID_COLLECT_DEKU_MASK, .name = "Deku Transformation", .description = "Transform into Deku Link for the first time",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_DEKU], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_30_10
    }},
     { AID_COLLECT_GORON_MASK, {
        .id = AID_COLLECT_GORON_MASK, .name = "Goron Transformation", .description = "Transform into Goron Link for the first time",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_GORON], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_30_20
    }},
    { AID_COLLECT_ZORA_MASK, {
        .id = AID_COLLECT_ZORA_MASK, .name = "Zora Transformation", .description = "Transform into Zora Link for the first time",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_ZORA], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_30_40
    }},

    // Mask achievements (Collection - OnItemGive)
    { AID_COLLECT_POSTMAN_HAT, {
        .id = AID_COLLECT_POSTMAN_HAT, .name = "Delivery for a Link?", .description = "Obtain the Postman's Hat",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_POSTMAN], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_POSTMAN
    }},
    { AID_COLLECT_ALLNIGHT_MASK, {
        .id = AID_COLLECT_ALLNIGHT_MASK, .name = "Up for 24h/3d", .description = "Obtain the All-Night Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_ALL_NIGHT], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_ALL_NIGHT
    }},
    { AID_COLLECT_BLAST_MASK, {
        .id = AID_COLLECT_BLAST_MASK, .name = "Face bomber", .description = "Obtain the Blast Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_BLAST], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_BLAST
    }},
    { AID_COLLECT_STONE_MASK, {
        .id = AID_COLLECT_STONE_MASK, .name = "I got a rock...", .description = "Obtain the Stone Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_STONE], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_STONE
    }},
    { AID_COLLECT_GREATFAIRY_MASK, {
        .id = AID_COLLECT_GREATFAIRY_MASK, .name = "Fairy collector", .description = "Obtain the Great Fairy Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_GREAT_FAIRY], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_GREAT_FAIRY
    }},
    { AID_COLLECT_KEATON_MASK, {
        .id = AID_COLLECT_KEATON_MASK, .name = "Two-Tailed deceiver", .description = "Obtain the Keaton Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_KEATON], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_KEATON
    }},
    { AID_COLLECT_BREMEN_MASK, {
        .id = AID_COLLECT_BREMEN_MASK, .name = "Leader of Animals", .description = "Obtain the Bremen Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_BREMEN], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_BREMEN
    }},
    { AID_COLLECT_BUNNY_HOOD, {
        .id = AID_COLLECT_BUNNY_HOOD, .name = "Speedy as a Hare", .description = "Obtain the Bunny Hood",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_BUNNY], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_BUNNY
    }},
    { AID_COLLECT_DONGERO_MASK, {
        .id = AID_COLLECT_DONGERO_MASK, .name = "*Kero Kero*", .description = "Obtain the Dongero Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_DON_GERO], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_DON_GERO
    }},
    { AID_COLLECT_MASK_OF_SCENTS, {
        .id = AID_COLLECT_MASK_OF_SCENTS, .name = "Sniffer of many things", .description = "Obtain the Mask of Scents",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_SCENTS], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_SCENTS
    }},
    { AID_COLLECT_ROMANI_MASK, {
        .id = AID_COLLECT_ROMANI_MASK, .name = "Mark of maturity", .description = "Obtain the Romani Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_ROMANI], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_ROMANI
    }},
     { AID_COLLECT_CIRCUS_LEADER_MASK, {
        .id = AID_COLLECT_CIRCUS_LEADER_MASK, .name = "Troupe leader", .description = "Obtain the Circus Leader's Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_CIRCUS_LEADER], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_CIRCUS_LEADER
    }},
    { AID_COLLECT_KAFEI_MASK, {
        .id = AID_COLLECT_KAFEI_MASK, .name = "Town detective", .description = "Obtain the Kafei Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_KAFEIS_MASK], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_KAFEIS_MASK
    }},
    { AID_COLLECT_COUPLES_MASK, {
        .id = AID_COLLECT_COUPLES_MASK, .name = "Witness of union", .description = "Obtain the Couple's Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_COUPLE], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_COUPLE
    }},
     { AID_COLLECT_MASK_OF_TRUTH, {
        .id = AID_COLLECT_MASK_OF_TRUTH, .name = "Seer of Truth", .description = "Obtain the Mask of Truth",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_TRUTH], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_TRUTH
    }},
    { AID_COLLECT_KAMARO_MASK, {
        .id = AID_COLLECT_KAMARO_MASK, .name = "Lord of the Dance", .description = "Obtain the Kamaro Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_KAMARO], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_KAMARO
    }},
    { AID_COLLECT_GIBDO_MASK, {
        .id = AID_COLLECT_GIBDO_MASK, .name = "Mummified Taskmaster", .description = "Obtain the Gibdo Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_GIBDO], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_GIBDO
    }},
     { AID_COLLECT_GARO_MASK, {
        .id = AID_COLLECT_GARO_MASK, .name = "Ninja Master", .description = "Obtain the Garo Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_GARO], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_GARO
    }},
    { AID_COLLECT_CAPTAINS_HAT, {
        .id = AID_COLLECT_CAPTAINS_HAT, .name = "I'm the Captain now!", .description = "Obtain the Captain's Hat",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_CAPTAIN], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_CAPTAIN
    }},
    { AID_COLLECT_GIANTS_HAT, { // Note: Enum is AID_COLLECT_GIANTS_HAT, item is ITEM_MASK_GIANT
        .id = AID_COLLECT_GIANTS_HAT, .name = "5th Giant", .description = "Obtain the Giant's Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_GIANT], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_GIANT
    }},
    { AID_COLLECT_FIERCE_DEITY, {
        .id = AID_COLLECT_FIERCE_DEITY, .name = "God of War", .description = "Obtain the Fierce Deity Mask",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], .isSecret = true, .gamerscore = 30, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_MASK_FIERCE_DEITY
    }},
    { AID_COLLECT_ALL_MASKS, {
        .id = AID_COLLECT_ALL_MASKS, .name = "Mask Collector", .description = "Collect all 24 masks",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_TRUTH], .isSecret = true, .gamerscore = 50, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, // Trigger on any mask give, check condition
        .additionalCondition = []{
             // Check if all masks are present in inventory (excluding Fierce Deity, matching original logic)
             for (u8 i = ITEM_MASK_DEKU; i <= ITEM_MASK_GIANT; i++) {
                 if (INV_CONTENT(i) == ITEM_NONE) return false;
             }
             return true; // No need to check Fierce Deity based on original macro
        }
    }},

     // Event achievements (OnFlagSet)
    { AID_EAVESDROPPER, {
        .id = AID_EAVESDROPPER, .name = "Eavesdropper", .description = "Listen in on Anju and her Mothers conversation",
        .iconPath = (const char*)gItemIcons[ITEM_ROOM_KEY], .isSecret = true, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_85_04
    }},
    { AID_UNLIMITED_POWER, {
        .id = AID_UNLIMITED_POWER, .name = "Unlimited Power!", .description = "Drink Chateau Romani",
        .iconPath = (const char*)gItemIcons[ITEM_CHATEAU], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_DRANK_CHATEAU_ROMANI
    }},
    { AID_DEFEAT_ALIENS, {
        .id = AID_DEFEAT_ALIENS, .name = "Flatwoods Buster", .description = "Defend Ranch from Alien threat",
        .iconPath = (const char*)gItemIcons[ITEM_MILK_BOTTLE], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_DEFENDED_AGAINST_THEM
    }},
    { AID_HAGS_HERO, {
        .id = AID_HAGS_HERO, .name = "Hag's Hero", .description = "Save Koume",
        .iconPath = (const char*)gItemIcons[ITEM_POTION_RED], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_SAVED_KOUME
    }},
    { AID_SPEED_DEMON, {
        .id = AID_SPEED_DEMON, .name = "Speed Demon", .description = "Win the Goron race",
        .iconPath = (const char*)gItemIcons[ITEM_GOLD_DUST], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_41_08
    }},

    // Heart piece achievements (Mostly OnFlagSet)
    { AID_COLLECT_HEART_CONTAINER, { // Reverted to original logic
        .id = AID_COLLECT_HEART_CONTAINER, .name = "Heart of a Hero", .description = "Collect your first Heart Container",
        .iconPath = (const char*)gItemIcons[ITEM_HEART_CONTAINER], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnActorInit, // Original trigger
        .additionalCondition = []{ return gSaveContext.save.saveInfo.playerData.healthCapacity > 0x30; } // Original condition
    }},
    { AID_LABORIOUS_SWIMMER, {
        .id = AID_LABORIOUS_SWIMMER, .name = "Laborious Swimmer", .description = "Win the final Beaver race reward",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_ZORA], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_25_01
    }},
    { AID_TOWN_SHARKSHOOTER, {
        .id = AID_TOWN_SHARKSHOOTER, .name = "Town Sharpshooter", .description = "Win the final Town Shooting Gallery reward",
        .iconPath = (const char*)gItemIcons[ITEM_BOW], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_RECEIVED_TOWN_SHOOTING_GALLERY_HEART_PIECE
    }},
    { AID_SWAMP_SHARKSHOOTER, {
        .id = AID_SWAMP_SHARKSHOOTER, .name = "Swamp Sharpshooter", .description = "Win the final Swamp Shooting Gallery reward",
        .iconPath = (const char*)gItemIcons[ITEM_BOW], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_RECEIVED_SWAMP_SHOOTING_GALLERY_HEART_PIECE
    }},
    { AID_HONEY_AND_DARLING_SHOWSTOPPER, {
        .id = AID_HONEY_AND_DARLING_SHOWSTOPPER, .name = "Honey and Darling Showstopper", .description = "Win the Honey & Darling Heart Piece",
        .iconPath = (const char*)gItemIcons[ITEM_BOW], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_22_80
    }},
    { AID_DEKU_CHAMPION, {
        .id = AID_DEKU_CHAMPION, .name = "Playground Champ", .description = "Win final Deku Playground reward",
        .iconPath = (const char*)gItemIcons[ITEM_DEKU_NUT], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_RECEIVED_DEKU_PLAYGROUND_HEART_PIECE
    }},
    { AID_SWAMP_PHOTOGRAPHER, {
        .id = AID_SWAMP_PHOTOGRAPHER, .name = "Swamp Photographer", .description = "Complete the Swamp Boat Tour and receive the Pictograph Box",
        .iconPath = (const char*)gItemIcons[ITEM_DEED_SWAMP], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_61_10
    }},
    { AID_ISLAND_HOPPER, {
        .id = AID_ISLAND_HOPPER, .name = "Island Hopper", .description = "Win the Greatbay Island minigame reward",
        .iconPath = (const char*)gItemIcons[ITEM_DEED_OCEAN], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_82_10
    }},
    { AID_DEKU_LAND_TRADER, {
        .id = AID_DEKU_LAND_TRADER, .name = "Deku real estate tycoon", .description = "Trade all Deku Title Deeds",
        .iconPath = (const char*)gItemIcons[ITEM_DEED_LAND], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_62_20,
        .additionalCondition = []{
            return (CHECK_WEEKEVENTREG(WEEKEVENTREG_17_80) && CHECK_WEEKEVENTREG(WEEKEVENTREG_61_10) &&
                    CHECK_WEEKEVENTREG(WEEKEVENTREG_61_80) && CHECK_WEEKEVENTREG(WEEKEVENTREG_62_04) &&
                    CHECK_WEEKEVENTREG(WEEKEVENTREG_62_20));
        }
    }},
    { AID_POEBUSTER, {
        .id = AID_POEBUSTER, .name = "Poebuster", .description = "Win the Spirit House reward",
        .iconPath = (const char*)gItemIcons[ITEM_BIG_POE], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_54_40
    }},
    { AID_SEACROSSED_REUNION, {
        .id = AID_SEACROSSED_REUNION, .name = "Seacrossed Reunion", .description = "Reunite the Seahorses",
        .iconPath = (const char*)gItemIcons[ITEM_SEAHORSE], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_32_01
    }},
    { AID_MAX_HEALTH, { // Changed trigger
        .id = AID_MAX_HEALTH, .name = "Full of Heart", .description = "Obtain maximum health (20 hearts)",
        .iconPath = (const char*)gItemIcons[ITEM_HEART_CONTAINER], .isSecret = true, .gamerscore = 30, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, // Trigger on getting HC or HP
        .additionalCondition = []{ return gSaveContext.save.saveInfo.playerData.healthCapacity >= 0x140; }
    }},

    // Fairy achievements
     { AID_LOST_AND_FOUND, {
        .id = AID_LOST_AND_FOUND, .name = "Lost and Found", .description = "Collect the lost Stray Fairy in Clock Town",
        .iconPath = (const char*)gItemIcons[ITEM_MAGIC_JAR_SMALL], .isSecret = true, .gamerscore = 5, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_08_80
        // Note: Rando check is separate if needed, vanilla relies on flag
    }},
    { AID_COLLECT_GREAT_SPIN, {
        .id = AID_COLLECT_GREAT_SPIN, .name = "Master Spinner", .description = "Obtain the Great Spin",
        .iconPath = (const char*)gItemIcons[ITEM_SWORD_KOKIRI], .isSecret = true, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_OBTAINED_GREAT_SPIN_ATTACK
    }},
     { AID_COLLECT_DOUBLE_MAGIC, { // Changed trigger
        .id = AID_COLLECT_DOUBLE_MAGIC, .name = "Adept Magician", .description = "Obtained Double Magic",
        .iconPath = (const char*)gItemIcons[ITEM_MAGIC_JAR_BIG], .isSecret = true, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet, // Trigger on flag set by GF
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_12_80, // Use generic GF reward flag
        .additionalCondition = []{ return gSaveContext.save.saveInfo.playerData.magicLevel >= 2; } // Double check level
    }},
    { AID_COLLECT_DOUBLE_DEFENSE, { // Changed trigger
        .id = AID_COLLECT_DOUBLE_DEFENSE, .name = "Thick as Iron", .description = "Obtained Double Defense",
        .iconPath = (const char*)gItemIcons[ITEM_HEART_CONTAINER], .isSecret = true, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet, // Trigger on flag set by GF
        .checkFlagType = FLAG_WEEK_EVENT_REG, .checkFlag = WEEKEVENTREG_12_80, // Use generic GF reward flag
        .additionalCondition = []{ return gSaveContext.save.saveInfo.playerData.doubleDefense; } // Check defense bool
    }},
    { AID_COLLECT_GREAT_FAIRY_SWORD, {
        .id = AID_COLLECT_GREAT_FAIRY_SWORD, .name = "Fairy's Champion", .description = "Obtain the Great Fairy's Sword",
        .iconPath = (const char*)gItemIcons[ITEM_SWORD_GREAT_FAIRY], .isSecret = true, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnItemGive, .checkItemId = ITEM_SWORD_GREAT_FAIRY
    }},

    // Song achievements
    { AID_LEARN_SONG_OF_TIME, {
        .id = AID_LEARN_SONG_OF_TIME, .name = "Time Traveler", .description = "Use the Song of Time to reset the cycle for the first time",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::AfterEndOfCycleSave,
        .additionalCondition = []{
             u16 resetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;
             // Trigger if the count is 1 (first normal reset) or 2 (first reset after skipping cycle 1)
             return (resetCount == 1 || resetCount == 2);
        }
    }},
    { AID_MASTER_HEALER, {
        .id = AID_MASTER_HEALER, .name = "Master Healer", .description = "Heal all cursed souls with the Song of Healing",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 20, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG,
        .checkFlag = WEEKEVENTREG_82_04,
        .additionalCondition = []{
             return CHECK_WEEKEVENTREG(WEEKEVENTREG_30_20) && CHECK_WEEKEVENTREG(WEEKEVENTREG_30_40) &&
                    CHECK_WEEKEVENTREG(WEEKEVENTREG_75_20) && CHECK_WEEKEVENTREG(WEEKEVENTREG_82_04);
        }
    }},
    { AID_LEARN_EPONAS_SONG, { // Changed trigger
        .id = AID_LEARN_EPONAS_SONG, .name = "Horse Whisperer", .description = "Learn Epona's Song",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0, // Correct Order
        .checkSceneId = SCENE_ID_CHECK_ALWAYS, // Correct Order
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_EPONA); }
    }},
     { AID_LEARN_SOARING, { // Changed trigger
        .id = AID_LEARN_SOARING, .name = "Fly like a Owl", .description = "Learn the Song of Soaring",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_SOARING); }
    }},
     { AID_LEARN_STORMS, { // Changed trigger
        .id = AID_LEARN_STORMS, .name = "Song of Brotherly Love", .description = "Learn the Song of Storms",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_STORMS); }
    }},
    { AID_LEARN_SONATA, { // Changed trigger
        .id = AID_LEARN_SONATA, .name = "A small misunderstanding", .description = "Learn the Sonata of Awakening",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_SONATA); }
    }},
    { AID_LEARN_LULLABY, { // Changed trigger
        .id = AID_LEARN_LULLABY, .name = "Silence...at last!", .description = "Learn the Goron Lullaby",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_LULLABY); }
    }},
    { AID_LEARN_BOSSA_NOVA, { // Changed trigger
        .id = AID_LEARN_BOSSA_NOVA, .name = "Miracle of Life", .description = "Learn the New Wave Bossa Nova",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_BOSSA_NOVA); }
    }},
     { AID_LEARN_ELEGY, { // Changed trigger
        .id = AID_LEARN_ELEGY, .name = "Sunshine in Ikana", .description = "Learn the Elegy of Emptiness",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_ELEGY); }
    }},
     { AID_LEARN_OATH, { // Changed trigger
        .id = AID_LEARN_OATH, .name = "To me, my Giants!", .description = "Learn the Oath to Order",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{ return CHECK_QUEST_ITEM(QUEST_SONG_OATH); }
    }},
     { AID_LEARN_ALL_SONGS, { // Changed trigger
        .id = AID_LEARN_ALL_SONGS, .name = "Musician of Termina", .description = "Learn all the songs",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = true, .gamerscore = 30, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::OnSceneInit,
        .checkFlagType = FLAG_NONE, .checkFlag = 0,
        .checkSceneId = SCENE_ID_CHECK_ALWAYS,
        .additionalCondition = []{
            u32 allSongsBits = (1 << QUEST_SONG_SONATA) | (1 << QUEST_SONG_LULLABY) |
                               (1 << QUEST_SONG_BOSSA_NOVA) | (1 << QUEST_SONG_ELEGY) |
                               (1 << QUEST_SONG_OATH) | (1 << QUEST_SONG_TIME) |
                               (1 << QUEST_SONG_HEALING) | (1 << QUEST_SONG_EPONA) |
                               (1 << QUEST_SONG_SOARING) | (1 << QUEST_SONG_STORMS);
            return (GET_SAVE_INVENTORY_QUEST_ITEMS & allSongsBits) == allSongsBits;
        }
    }},

    // Time-Loop Related Achievements
    { AID_GROUNDHOG_DAY, {
        .id = AID_GROUNDHOG_DAY, .name = "Groundhog Day", .description = "Reset the time cycle 10 times",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::AfterEndOfCycleSave,
        .additionalCondition = []{ return gSaveContext.save.saveInfo.playerData.threeDayResetCount >= 10; }
    }},
     { AID_TRAPPED_IN_TIME, {
        .id = AID_TRAPPED_IN_TIME, .name = "Trapped in Time", .description = "Reset the time cycle 999 times",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = true, .gamerscore = 50, .category = AchievementCategory::BOTH,
        .triggerType = AchievementTriggerType::AfterEndOfCycleSave,
        .additionalCondition = []{ return gSaveContext.save.saveInfo.playerData.threeDayResetCount == 999; }
    }},

    // Randomizer-specific achievements
    { AID_RANDO_FIRST_ITEM, {
        .id = AID_RANDO_FIRST_ITEM, .name = "The Journey Begins", .description = "Collect your first randomized item",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = false, .gamerscore = 10, .category = AchievementCategory::RANDOMIZER,
        .triggerType = AchievementTriggerType::OnFlagSet, // Trigger when any Rando flag is set
        .checkFlagType = FLAG_RANDO_INF, // Requires specific handling in dispatcher
        .additionalCondition = []{
            if (!IS_RANDO) return false;
            for (size_t i = 0; i < RC_MAX; i++) {
                 if (RANDO_SAVE_CHECKS[i].obtained) return true;
            }
            return false;
        }
    }},
    { AID_RANDO_PLAYAS, {
        .id = AID_RANDO_PLAYAS, .name = "Play as Rando", .description = "Play as a non-default form in randomizer mode",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_DEKU], .isSecret = false, .gamerscore = 20, .category = AchievementCategory::RANDOMIZER,
        .triggerType = AchievementTriggerType::OnItemGive, // Check if item is a transformation mask
        .checkItemId = ITEM_NONE // Logic moved to handler
    }},
    { AID_NOVICE_CHECK_HUNTER, {
        .id = AID_NOVICE_CHECK_HUNTER, .name = "Novice Check Hunter", .description = "Obtain items from at least 100 different checks",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = true, .gamerscore = 10, .category = AchievementCategory::RANDOMIZER,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_RANDO_INF, // Requires specific handling
        .additionalCondition = []{
            if (!IS_RANDO) return false;
            int checkedLocations = 0;
            for (size_t i = 0; i < RC_MAX; i++) {
                if (RANDO_SAVE_CHECKS[i].obtained) checkedLocations++;
            }
            return checkedLocations >= 100;
        }
    }},
    { AID_ADEPT_CHECK_HUNTER, {
        .id = AID_ADEPT_CHECK_HUNTER, .name = "Adept Check Hunter", .description = "Obtain items from at least 1000 different checks",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = true, .gamerscore = 30, .category = AchievementCategory::RANDOMIZER,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_RANDO_INF, // Requires specific handling
        .additionalCondition = []{
             if (!IS_RANDO) return false;
             int checkedLocations = 0;
             for (size_t i = 0; i < RC_MAX; i++) {
                 if (RANDO_SAVE_CHECKS[i].obtained) checkedLocations++;
             }
             return checkedLocations >= 1000;
        }
    }},
    { AID_ALL_DONE, {
        .id = AID_ALL_DONE, .name = "All Done!", .description = "Obtain all checks in a seed.",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = true, .gamerscore = 30, .category = AchievementCategory::RANDOMIZER,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_RANDO_INF, // Requires specific handling
        .additionalCondition = []{
            if (!IS_RANDO) return false;
            int check_count = 0;
            int checkedLocations = 0;
            for (size_t i = 0; i < RC_MAX; i++) {
                if (RANDO_SAVE_CHECKS[i].obtained) checkedLocations++;
                if (RANDO_SAVE_CHECKS[i].shuffled) check_count++;
            }
            return (check_count > 0 && checkedLocations >= check_count); // Avoid triggering if check_count is 0
        }
    }},
    { AID_RANDO_MASTER, {
        .id = AID_RANDO_MASTER, .name = "Rando Master", .description = "Obtain all checks in a max sanity.",
        .iconPath = (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], .isSecret = true, .gamerscore = 50, .category = AchievementCategory::RANDOMIZER,
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_RANDO_INF, // Requires specific handling
        .additionalCondition = []{
            if (!IS_RANDO) return false;
            int checkedLocations = 0;
            for (size_t i = 0; i < RC_MAX; i++) {
                if (RANDO_SAVE_CHECKS[i].obtained) checkedLocations++;
            }
            return checkedLocations >= RC_MAX;
        }
    }},
     { AID_RANDO_FIRST_TRY, {
        .id = AID_RANDO_FIRST_TRY, .name = "Boss Rush", .description = "Defeat all four temple bosses in first cycle outside of Clock Town",
        .iconPath = (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], .isSecret = true, .gamerscore = 50, .category = AchievementCategory::BOTH, // Category BOTH is intentional from original
        .triggerType = AchievementTriggerType::OnFlagSet,
        .checkFlagType = FLAG_WEEK_EVENT_REG, // Added specific trigger flag
        .checkFlag = WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE, // Added specific trigger flag (last boss)
        .additionalCondition = []{
            bool hasAllRemains = CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE) &&
                                 CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE) &&
                                 CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE) &&
                                 CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE);
            // reset count is 0 initially, 1 after first reset. Need <= 1.
            return hasAllRemains && (gSaveContext.save.saveInfo.playerData.threeDayResetCount <= 1);
        }
    }},

    // Add more achievements here following the pattern...
};


// --- Helper Function Definitions ---

// Helper to get AchievementStaticData based on flag type and value
// Mirrors Rando's StaticData::GetCheckFromFlag
AchievementStaticData GetAchievementFromFlagSet(FlagType flagType, u32 flag) {
    for (auto const& [key, val] : AllAchievementData) {
        if (val.triggerType == AchievementTriggerType::OnFlagSet) {
            // Handle specific RANDO_INF case - return first match for now
            // More complex handling might be needed if multiple achievements use RANDO_INF
            if (flagType == FLAG_RANDO_INF && val.checkFlagType == FLAG_RANDO_INF) {
                return val;
            }
            // Standard check for other flag types (e.g., FLAG_WEEK_EVENT_REG)
            if (val.checkFlagType == flagType && val.checkFlag == flag) {
                return val;
            }
        }
    }
    return {}; // Return default/invalid AchievementStaticData
}

// Helper to get AchievementStaticData based on item ID
AchievementStaticData GetAchievementFromItemGive(u8 item) {
     for (auto const& [key, val] : AllAchievementData) {
         if (val.triggerType == AchievementTriggerType::OnItemGive) {
            // Check if the trigger is specifically for this item
            if (val.checkItemId == item) {
                 return val;
            }
            // Handle cases like AID_COLLECT_ALL_MASKS or AID_RANDO_PLAYAS that trigger on a range or type of item
            // These rely on additionalCondition, but we return the base struct here so the handler can check it.
             if (val.id == AID_COLLECT_ALL_MASKS && item >= ITEM_MASK_DEKU && item <= ITEM_MASK_FIERCE_DEITY) {
                 return val;
             }
             if (val.id == AID_MAX_HEALTH && (item == ITEM_HEART_CONTAINER || item == ITEM_HEART_PIECE)) {
                 return val;
             }
              if (val.id == AID_RANDO_PLAYAS && (item == ITEM_MASK_DEKU || item == ITEM_MASK_GORON || item == ITEM_MASK_ZORA)) {
                 return val;
             }
         }
     }
     return {}; // Return default/invalid
}

// Helper to get AchievementStaticData based on Scene ID
AchievementStaticData GetAchievementFromSceneInit(s16 sceneId) {
     for (auto const& [key, val] : AllAchievementData) {
         if (val.triggerType == AchievementTriggerType::OnSceneInit && val.checkSceneId == sceneId) {
             return val;
         }
     }
     return {}; // Return default/invalid
}

// Helper for AfterEndOfCycleSave (no params)
AchievementStaticData GetAchievementFromEndOfCycleSave() {
     // This trigger is less common, could potentially return multiple if needed later
     for (auto const& [key, val] : AllAchievementData) {
         if (val.triggerType == AchievementTriggerType::AfterEndOfCycleSave) {
             return val; // Return first match for now
         }
     }
     return {};
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