#include "Achievements.h"
#include "AchievementDefinitions.h"
#include "2s2h/BenPort.h"
#include "2s2h/Rando/Rando.h"
#include "2s2h/Rando/Types.h"
#include "2s2h/ShipInit.hpp"
#include <libultraship/libultraship.h>
#include <z64scene.h>
#include <z64item.h>

#define CVAR_NAME_ACHIEVEMENTS "gEnhancements.Achievements.Enabled"
#define CVAR_ACHIEVEMENTS CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)

// --- Function to Register ALL Achievement Triggers ---
// This function will be called by ShipInit if the CVar is enabled
void RegisterAllAchievementTriggers() {

    SPDLOG_INFO("Registering all achievement triggers...");

    // --- Place ALL your REGISTER_ACHIEVEMENT, ACHIEVEMENT_ID_HOOK, ACHIEVEMENT_VB_HOOK calls HERE ---
    // Test Achievements
    ACHIEVEMENT_ID_HOOK(AID_FIRST_STEPS, "Beyond the Town Walls", // Use AID_FIRST_STEPS
                        "Leave Clock Town and enter Termina Field for the first time",
                        (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                        OnSceneInit, SCENE_00KEIKOKU,
                        [](s8 sceneId, s8 spawnNum) { return CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_TERMINA_FIELD); });

    // Boss achievements
    REGISTER_ACHIEVEMENT(AID_DEFEAT_ODOLWA, "Jungle Warrior",
                         "Defeat Odolwa, Masked Jungle Warrior", // Use AID_DEFEAT_ODOLWA
                         (const char*)gItemIcons[ITEM_REMAINS_ODOLWA], false, 20, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE;
                         });

    REGISTER_ACHIEVEMENT(AID_DEFEAT_GOHT, "Mountain Racer",
                         "Defeat Goht, Masked Mechanical Monster", // Use AID_DEFEAT_GOHT
                         (const char*)gItemIcons[ITEM_REMAINS_GOHT], false, 20, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE;
                         });

    REGISTER_ACHIEVEMENT(AID_DEFEAT_GYORG, "Ocean Conqueror",
                         "Defeat Gyorg, Gargantuan Masked Fish", // Use AID_DEFEAT_GYORG
                         (const char*)gItemIcons[ITEM_REMAINS_GYORG], false, 20, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE;
                         });

    REGISTER_ACHIEVEMENT(AID_DEFEAT_TWINMOLD, "Desert Exterminator",
                         "Defeat Twinmold, Giant Masked Insects", // Use AID_DEFEAT_TWINMOLD
                         (const char*)gItemIcons[ITEM_REMAINS_TWINMOLD], false, 20, AchievementCategory::BOTH,
                         OnFlagSet, [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE;
                         });

    ACHIEVEMENT_VB_HOOK(AID_DEFEAT_MAJORA, "Savior of Termina",
                        "Defeat Majora and save Termina", // Use AID_DEFEAT_MAJORA
                        (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], false, 50, AchievementCategory::BOTH,
                        VB_SETUP_TRANSITION, (gPlayState->nextEntrance == 0x5400));

    // Mask achievements
    REGISTER_ACHIEVEMENT(AID_COLLECT_DEKU_MASK, "Deku Transformation",
                         "Transform into Deku Link for the first time", // Use AID_COLLECT_DEKU_MASK
                         (const char*)gItemIcons[ITEM_MASK_DEKU], false, 10, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             // Trigger when the specific flag is set
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_30_10;
                         });

    REGISTER_ACHIEVEMENT(AID_COLLECT_GORON_MASK, "Goron Transformation", // Use AID_COLLECT_GORON_MASK
                         "Transform into Goron Link for the first time", (const char*)gItemIcons[ITEM_MASK_GORON],
                         false, 10, AchievementCategory::BOTH, OnFlagSet, [](FlagType flagType, u32 flag) {
                             // Trigger when the specific flag is set
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_30_20;
                         });

    REGISTER_ACHIEVEMENT(AID_COLLECT_ZORA_MASK, "Zora Transformation",
                         "Transform into Zora Link for the first time", // Use AID_COLLECT_ZORA_MASK
                         (const char*)gItemIcons[ITEM_MASK_ZORA], false, 10, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             // Trigger when the specific flag is set
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_30_40;
                         });

    REGISTER_ACHIEVEMENT(AID_COLLECT_POSTMAN_HAT, "Delivery for a Link?",
                         "Obtain the Postman's Hat", // Use AID_COLLECT_POSTMAN_HAT
                         (const char*)gItemIcons[ITEM_MASK_POSTMAN], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_POSTMAN; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_ALLNIGHT_MASK, "Up for 24h/3d",
                         "Obtain the All-Night Mask", // Use AID_COLLECT_ALLNIGHT_MASK
                         (const char*)gItemIcons[ITEM_MASK_ALL_NIGHT], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_ALL_NIGHT; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_BLAST_MASK, "Face bomber", "Obtain the Blast Mask", // Use AID_COLLECT_BLAST_MASK
                         (const char*)gItemIcons[ITEM_MASK_BLAST], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_BLAST; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_STONE_MASK, "I got a rock...",
                         "Obtain the Stone Mask", // Use AID_COLLECT_STONE_MASK
                         (const char*)gItemIcons[ITEM_MASK_STONE], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_STONE; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_GREATFAIRY_MASK, "Fairy collector",
                         "Obtain the Great Fairy Mask", // Use AID_COLLECT_GREATFAIRY_MASK
                         (const char*)gItemIcons[ITEM_MASK_GREAT_FAIRY], false, 10, AchievementCategory::BOTH,
                         OnItemGive, [](u8 item) { return item == ITEM_MASK_GREAT_FAIRY; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_KEATON_MASK, "Two-Tailed deceiver",
                         "Obtain the Keaton Mask", // Use AID_COLLECT_KEATON_MASK
                         (const char*)gItemIcons[ITEM_MASK_KEATON], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_KEATON; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_BREMEN_MASK, "Leader of Animals",
                         "Obtain the Bremen Mask", // Use AID_COLLECT_BREMEN_MASK
                         (const char*)gItemIcons[ITEM_MASK_BREMEN], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_BREMEN; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_BUNNY_HOOD, "Speedy as a Hare",
                         "Obtain the Bunny Hood", // Use AID_COLLECT_BUNNY_HOOD
                         (const char*)gItemIcons[ITEM_MASK_BUNNY], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_BUNNY; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_DONGERO_MASK, "*Kero Kero*",
                         "Obtain the Dongero Mask", // Use AID_COLLECT_DONGERO_MASK
                         (const char*)gItemIcons[ITEM_MASK_DON_GERO], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_DON_GERO; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_MASK_OF_SCENTS, "Sniffer of many things",
                         "Obtain the Mask of Scents", // Use AID_COLLECT_MASK_OF_SCENTS
                         (const char*)gItemIcons[ITEM_MASK_SCENTS], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_SCENTS; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_ROMANI_MASK, "Mark of maturity",
                         "Obtain the Romani Mask", // Use AID_COLLECT_ROMANI_MASK
                         (const char*)gItemIcons[ITEM_MASK_ROMANI], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_ROMANI; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_CIRCUS_LEADER_MASK, "Troupe leader",
                         "Obtain the Circus Leader's Mask", // Use AID_COLLECT_CIRCUS_LEADER_MASK
                         (const char*)gItemIcons[ITEM_MASK_CIRCUS_LEADER], false, 10, AchievementCategory::BOTH,
                         OnItemGive, [](u8 item) { return item == ITEM_MASK_CIRCUS_LEADER; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_KAFEI_MASK, "Town detective",
                         "Obtain the Kafei Mask", // Use AID_COLLECT_KAFEI_MASK
                         (const char*)gItemIcons[ITEM_MASK_KAFEIS_MASK], false, 10, AchievementCategory::BOTH,
                         OnItemGive, [](u8 item) { return item == ITEM_MASK_KAFEIS_MASK; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_COUPLES_MASK, "Witness of union",
                         "Obtain the Couple's Mask", // Use AID_COLLECT_COUPLES_MASK
                         (const char*)gItemIcons[ITEM_MASK_COUPLE], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_COUPLE; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_MASK_OF_TRUTH, "Seer of Truth",
                         "Obtain the Mask of Truth", // Use AID_COLLECT_MASK_OF_TRUTH
                         (const char*)gItemIcons[ITEM_MASK_TRUTH], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_TRUTH; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_KAMARO_MASK, "Lord of the Dance",
                         "Obtain the Kamaro Mask", // Use AID_COLLECT_KAMARO_MASK
                         (const char*)gItemIcons[ITEM_MASK_KAMARO], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_KAMARO; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_GIBDO_MASK, "Mummified Taskmaster",
                         "Obtain the Gibdo Mask", // Use AID_COLLECT_GIBDO_MASK
                         (const char*)gItemIcons[ITEM_MASK_GIBDO], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_GIBDO; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_GARO_MASK, "Ninja Master", "Obtain the Garo Mask", // Use AID_COLLECT_GARO_MASK
                         (const char*)gItemIcons[ITEM_MASK_GARO], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_GARO; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_CAPTAINS_HAT, "I'm the Captain now!",
                         "Obtain the Captain's Hat", // Use AID_COLLECT_CAPTAINS_HAT
                         (const char*)gItemIcons[ITEM_MASK_CAPTAIN], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_CAPTAIN; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_GIANTS_HAT, "5th Giant", "Obtain the Giant's Mask", // Use AID_COLLECT_GIANTS_HAT
                         (const char*)gItemIcons[ITEM_MASK_GIANT], false, 10, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) { return item == ITEM_MASK_GIANT; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_FIERCE_DEITY, "God of War",
                         "Obtain the Fierce Deity Mask", // Use AID_COLLECT_FIERCE_DEITY
                         (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], true, 30, AchievementCategory::BOTH,
                         OnItemGive, [](u8 item) { return item == ITEM_MASK_FIERCE_DEITY; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_ALL_MASKS, "Mask Collector", "Collect all 24 masks", // Use AID_COLLECT_ALL_MASKS
                         (const char*)gItemIcons[ITEM_MASK_TRUTH], true, 50, AchievementCategory::BOTH, OnItemGive,
                         [](u8 item) {
                             // Only check if the received item is potentially the last mask needed
                             if (item < ITEM_MASK_DEKU || item > ITEM_MASK_GIANT) {
                                 return false;
                             }
                             for (u8 i = ITEM_MASK_DEKU; i <= ITEM_MASK_GIANT; i++) {
                                 if (INV_CONTENT(i) == ITEM_NONE)
                                     return false;
                             }
                             return true;
                         });

    // Event achievements
    REGISTER_ACHIEVEMENT(
        AID_EAVESDROPPER, "Eavesdropper", "Listen in on Anju and her Mothers conversation", // Use AID_EAVESDROPPER
        (const char*)gItemIcons[ITEM_ROOM_KEY], true, 10, AchievementCategory::BOTH, OnFlagSet,
        [](FlagType flagType, u32 flag) { return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_85_04; });

    REGISTER_ACHIEVEMENT(AID_UNLIMITED_POWER, "Unlimited Power!", "Drink Chateau Romani", // Use AID_UNLIMITED_POWER
                         (const char*)gItemIcons[ITEM_CHATEAU], false, 10, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_DRANK_CHATEAU_ROMANI;
                         });

    REGISTER_ACHIEVEMENT(AID_DEFEAT_ALIENS, "Flatwoods Buster",
                         "Defend Ranch from Alien threat", // Use AID_DEFEAT_ALIENS
                         (const char*)gItemIcons[ITEM_MILK_BOTTLE], false, 10, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_DEFENDED_AGAINST_THEM;
                         });

    REGISTER_ACHIEVEMENT(AID_HAGS_HERO, "Hag's Hero", "Save Koume", (const char*)gItemIcons[ITEM_POTION_RED],
                         false, // Use AID_HAGS_HERO
                         10, AchievementCategory::BOTH, OnFlagSet, [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_SAVED_KOUME;
                         });

    REGISTER_ACHIEVEMENT(AID_SPEED_DEMON, "Speed Demon", "Win the Goron race", (const char*)gItemIcons[ITEM_GOLD_DUST],
                         false, 10, // Use AID_SPEED_DEMON
                         AchievementCategory::BOTH, OnFlagSet, [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_41_08;
                         });

    // Heart piece achievements
    REGISTER_ACHIEVEMENT(AID_COLLECT_HEART_CONTAINER, "Heart of a Hero",
                         "Collect your first Heart Container", // Use AID_COLLECT_HEART_CONTAINER
                         (const char*)gItemIcons[ITEM_HEART_CONTAINER], false, 10, AchievementCategory::BOTH,
                         OnActorInit,
                         [](Actor* actor) { return gSaveContext.save.saveInfo.playerData.healthCapacity > 0x30; });

    REGISTER_ACHIEVEMENT(
        AID_LABORIOUS_SWIMMER, "Laborious Swimmer", "Win the final Beaver race reward", // Use AID_LABORIOUS_SWIMMER
        (const char*)gItemIcons[ITEM_MASK_ZORA], false, 10, AchievementCategory::BOTH, OnFlagSet,
        [](FlagType flagType, u32 flag) { return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_25_01; });

    REGISTER_ACHIEVEMENT(AID_TOWN_SHARKSHOOTER, "Town Sharpshooter",
                         "Win the final Town Shooting Gallery reward", // Use AID_TOWN_SHARKSHOOTER
                         (const char*)gItemIcons[ITEM_BOW], false, 10, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG &&
                                    flag == WEEKEVENTREG_RECEIVED_TOWN_SHOOTING_GALLERY_HEART_PIECE;
                         });

    REGISTER_ACHIEVEMENT(AID_SWAMP_SHARKSHOOTER, "Swamp Sharpshooter",
                         "Win the final Swamp Shooting Gallery reward", // Use AID_SWAMP_SHARKSHOOTER
                         (const char*)gItemIcons[ITEM_BOW], false, 10, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG &&
                                    flag == WEEKEVENTREG_RECEIVED_SWAMP_SHOOTING_GALLERY_HEART_PIECE;
                         });

    REGISTER_ACHIEVEMENT(
        AID_HONEY_AND_DARLING_SHOWSTOPPER, "Honey and Darling Showstopper",
        "Win the Honey & Darling Heart Piece", // Use AID_HONEY_AND_DARLING_SHOWSTOPPER
        (const char*)gItemIcons[ITEM_BOW], false, 10, AchievementCategory::BOTH, OnFlagSet,
        [](FlagType flagType, u32 flag) { return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_22_80; });

    REGISTER_ACHIEVEMENT(
        AID_DEKU_CHAMPION, "Playground Champ", "Win final Deku Playground reward", // Use AID_DEKU_CHAMPION
        (const char*)gItemIcons[ITEM_DEKU_NUT], false, 10, AchievementCategory::BOTH, OnFlagSet,
        [](FlagType flagType, u32 flag) {
            return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_RECEIVED_DEKU_PLAYGROUND_HEART_PIECE;
        });

    REGISTER_ACHIEVEMENT(AID_SWAMP_PHOTOGRAPHER, "Swamp Photographer",
                         "Complete the Swamp Boat Tour and receive the Pictograph Box", // Use AID_SWAMP_PHOTOGRAPHER
                         (const char*)gItemIcons[ITEM_DEED_SWAMP], false, 10, AchievementCategory::BOTH,
                         // Flag set after getting the Picto Box/Swamp Deed reward (first run)
                         OnFlagSet, [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_61_10;
                         });

    REGISTER_ACHIEVEMENT(
        AID_ISLAND_HOPPER, "Island Hopper", "Win the Greatbay Island minigame reward", // Use AID_ISLAND_HOPPER
        (const char*)gItemIcons[ITEM_DEED_OCEAN], false, 10, AchievementCategory::BOTH,
        // Flag set after winning the HP for the first time (see En_Jgame_Tsn actor code)
        OnFlagSet,
        [](FlagType flagType, u32 flag) { return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_82_10; });

    REGISTER_ACHIEVEMENT(
        AID_DEKU_LAND_TRADER, "Deku real estate tycoon", "Trade all Deku Title Deeds", // Use AID_DEKU_LAND_TRADER
        (const char*)gItemIcons[ITEM_DEED_LAND], false, 10, AchievementCategory::BOTH, OnFlagSet,
        [](FlagType flagType, u32 flag) {
            if (flagType != FLAG_WEEK_EVENT_REG ||
                !(flag == WEEKEVENTREG_17_80 || flag == WEEKEVENTREG_61_10 || flag == WEEKEVENTREG_61_80 ||
                  flag == WEEKEVENTREG_62_04 || flag == WEEKEVENTREG_62_20)) {
                return false; // Only check full condition if a relevant flag was set
            }
            return (CHECK_WEEKEVENTREG(WEEKEVENTREG_17_80) && CHECK_WEEKEVENTREG(WEEKEVENTREG_61_10) &&
                    CHECK_WEEKEVENTREG(WEEKEVENTREG_61_80) && CHECK_WEEKEVENTREG(WEEKEVENTREG_62_04) &&
                    CHECK_WEEKEVENTREG(WEEKEVENTREG_62_20));
        });

    REGISTER_ACHIEVEMENT(AID_POEBUSTER, "Poebuster", "Win the Spirit House reward",
                         (const char*)gItemIcons[ITEM_BIG_POE], false, 10, // Use AID_POEBUSTER
                         AchievementCategory::BOTH, OnFlagSet, [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_54_40;
                         });

    REGISTER_ACHIEVEMENT(AID_SEACROSSED_REUNION, "Seacrossed Reunion", "Reunite the Seahorses",
                         (const char*)gItemIcons[ITEM_SEAHORSE], // Use AID_SEACROSSED_REUNION
                         false, 10, AchievementCategory::BOTH, OnFlagSet, [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_32_01;
                         });

    REGISTER_ACHIEVEMENT(AID_MAX_HEALTH, "Full of Heart", "Obtain maximum health (20 hearts)", // Use AID_MAX_HEALTH
                         (const char*)gItemIcons[ITEM_HEART_CONTAINER], true, 30, AchievementCategory::BOTH,
                         OnActorInit,
                         [](Actor* actor) { return gSaveContext.save.saveInfo.playerData.healthCapacity >= 0x140; });

    // Fairy achievements
    REGISTER_ACHIEVEMENT(
        AID_LOST_AND_FOUND, "Lost and Found", "Collect the lost Stray Fairy in Clock Town", // Use AID_LOST_AND_FOUND
        (const char*)gItemIcons[ITEM_MAGIC_JAR_SMALL], true, 5, AchievementCategory::BOTH,
        // Vanilla check: Flag set when fairy is collected.
        // Rando check: Verified separately by OnItemGive for RI_PROGRESSIVE_MAGIC.
        OnFlagSet,
        [](FlagType flagType, u32 flag) { return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_08_80; });

    REGISTER_ACHIEVEMENT(AID_COLLECT_GREAT_SPIN, "Master Spinner", "Obtain the Great Spin",
                         (const char*)gItemIcons[ITEM_SWORD_KOKIRI], // Use AID_COLLECT_GREAT_SPIN
                         true, 20, AchievementCategory::BOTH, OnFlagSet, [](FlagType flagType, u32 flag) {
                             return flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_OBTAINED_GREAT_SPIN_ATTACK;
                         });

    REGISTER_ACHIEVEMENT(AID_COLLECT_DOUBLE_MAGIC, "Adept Magician",
                         "Obtained Double Magic", // Use AID_COLLECT_DOUBLE_MAGIC
                         (const char*)gItemIcons[ITEM_MAGIC_JAR_BIG], true, 20, AchievementCategory::BOTH, OnActorInit,
                         [](Actor* actor) {
                             // magic_level 0=none, 1=single, 2=double
                             return gSaveContext.save.saveInfo.playerData.magicLevel >= 2;
                         });

    REGISTER_ACHIEVEMENT(AID_COLLECT_DOUBLE_DEFENSE, "Thick as Iron",
                         "Obtained Double Defense", // Use AID_COLLECT_DOUBLE_DEFENSE
                         (const char*)gItemIcons[ITEM_HEART_CONTAINER], true, 20, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) {
                             // defense_hearts 0=normal, 20=double defense
                             return gSaveContext.save.saveInfo.inventory.defenseHearts > 0;
                         });

    REGISTER_ACHIEVEMENT(AID_COLLECT_GREAT_FAIRY_SWORD, "Fairy's Champion",
                         "Obtain the Great Fairy's Sword", // Use AID_COLLECT_GREAT_FAIRY_SWORD
                         (const char*)gItemIcons[ITEM_SWORD_GREAT_FAIRY], true, 20, AchievementCategory::BOTH,
                         OnItemGive, [](u8 item) { return item == ITEM_SWORD_GREAT_FAIRY; });

    // Song achievements
    REGISTER_ACHIEVEMENT(AID_LEARN_SONG_OF_TIME, "Time Traveler", // Use AID_LEARN_SONG_OF_TIME
                         "Use the Song of Time to reset the cycle for the first time",
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         AfterEndOfCycleSave, []() {
                             u16 resetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;
                             // Trigger if the count is 1 (first normal reset) or 2 (first reset after skipping cycle 1)
                             return (resetCount == 1 || resetCount == 2);
                         });

    REGISTER_ACHIEVEMENT(AID_MASTER_HEALER, "Master Healer",
                         "Heal all cursed souls with the Song of Healing", // Use AID_MASTER_HEALER
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 20, AchievementCategory::BOTH, OnFlagSet,
                         [](FlagType flagType, u32 flag) {
                             // Check if the flag that triggered the hook is one of the relevant soul-healing flags
                             if (flagType != FLAG_WEEK_EVENT_REG ||
                                 !(flag == WEEKEVENTREG_30_20 || // Darmani -> Goron Mask
                                   flag == WEEKEVENTREG_30_40 || // Mikau -> Zora Mask
                                   flag == WEEKEVENTREG_75_20 || // Pamela's Father -> Gibdo Mask (via healing cutscene)
                                   flag == WEEKEVENTREG_82_04))  // Kamaro -> Kamaro's Mask (via healing cutscene)
                             {
                                 return false; // Only check full condition if a relevant flag was set
                             }
                             // Now check if *all* relevant souls have been healed
                             return CHECK_WEEKEVENTREG(WEEKEVENTREG_30_20) && CHECK_WEEKEVENTREG(WEEKEVENTREG_30_40) &&
                                    CHECK_WEEKEVENTREG(WEEKEVENTREG_75_20) && CHECK_WEEKEVENTREG(WEEKEVENTREG_82_04);
                         });

    REGISTER_ACHIEVEMENT(AID_LEARN_EPONAS_SONG, "Horse Whisperer", "Learn Epona's Song", // Use AID_LEARN_EPONAS_SONG
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         // Reverted: CHECK_QUEST_ITEM reads save context directly, no specific flag/hook found.
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_EPONA); });

    REGISTER_ACHIEVEMENT(AID_LEARN_SOARING, "Fly like a Owl", "Learn the Song of Soaring", // Use AID_LEARN_SOARING
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_SOARING); });

    REGISTER_ACHIEVEMENT(AID_LEARN_STORMS, "Song of Brotherly Love", "Learn the Song of Storms", // Use AID_LEARN_STORMS
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_STORMS); });

    REGISTER_ACHIEVEMENT(AID_LEARN_SONATA, "A small misunderstanding",
                         "Learn the Sonata of Awakening", // Use AID_LEARN_SONATA
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_SONATA); });

    REGISTER_ACHIEVEMENT(AID_LEARN_LULLABY, "Silence...at last!", "Learn the Goron Lullaby", // Use AID_LEARN_LULLABY
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_LULLABY); });

    REGISTER_ACHIEVEMENT(AID_LEARN_BOSSA_NOVA, "Miracle of Life",
                         "Learn the New Wave Bossa Nova", // Use AID_LEARN_BOSSA_NOVA
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_BOSSA_NOVA); });

    REGISTER_ACHIEVEMENT(AID_LEARN_ELEGY, "Sunshine in Ikana", "Learn the Elegy of Emptiness", // Use AID_LEARN_ELEGY
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_ELEGY); });

    REGISTER_ACHIEVEMENT(AID_LEARN_OATH, "To me, my Giants!", "Learn the Oath to Order", // Use AID_LEARN_OATH
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         OnActorInit, [](Actor* actor) { return CHECK_QUEST_ITEM(QUEST_SONG_OATH); });

    REGISTER_ACHIEVEMENT(AID_LEARN_ALL_SONGS, "Musician of Termina", "Learn all the songs", // Use AID_LEARN_ALL_SONGS
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 30, AchievementCategory::BOTH,
                         // Reverted: Complex check based on quest items, no specific trigger hook.
                         OnActorInit, [](Actor* actor) {
                             // Check flags for all obtainable Majora's Mask songs
                             u32 allSongsBits = (1 << QUEST_SONG_SONATA) | (1 << QUEST_SONG_LULLABY) |
                                                (1 << QUEST_SONG_BOSSA_NOVA) | (1 << QUEST_SONG_ELEGY) |
                                                (1 << QUEST_SONG_OATH) | (1 << QUEST_SONG_TIME) |
                                                (1 << QUEST_SONG_HEALING) | (1 << QUEST_SONG_EPONA) |
                                                (1 << QUEST_SONG_SOARING) | (1 << QUEST_SONG_STORMS);
                             return (GET_SAVE_INVENTORY_QUEST_ITEMS & allSongsBits) == allSongsBits;
                         });

    // Time-Loop Related Achievements
    REGISTER_ACHIEVEMENT(AID_GROUNDHOG_DAY, "Groundhog Day", "Reset the time cycle 10 times", // Use AID_GROUNDHOG_DAY
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                         AfterEndOfCycleSave, []() {
                             // Check count *after* the cycle save increments it
                             return gSaveContext.save.saveInfo.playerData.threeDayResetCount >= 10;
                         });

    REGISTER_ACHIEVEMENT(
        AID_TRAPPED_IN_TIME, "Trapped in Time", "Reset the time cycle 999 times", // Use AID_TRAPPED_IN_TIME
        (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 50, AchievementCategory::BOTH, AfterEndOfCycleSave,
        []() { return gSaveContext.save.saveInfo.playerData.threeDayResetCount == 999; });

    // Randomizer-specific achievements
    REGISTER_ACHIEVEMENT(AID_RANDO_FIRST_ITEM, "The Journey Begins",
                         "Collect your first randomized item", // Use AID_RANDO_FIRST_ITEM
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::RANDOMIZER,
                         // Trigger when any Rando flag is set
                         OnFlagSet, [](FlagType flagType, u32 flag) {
                             // Check if it was a Rando flag
                             if (flagType != FLAG_RANDO_INF) {
                                 return false;
                             }
                             // Skip this check if the randomizer save data isn't initialized yet
                             if (!IS_RANDO) {
                                 return false;
                             }

                             // Check if player has obtained at least one item
                             for (size_t i = 0; i < RC_MAX; i++) {
                                 if (RANDO_SAVE_CHECKS[i].obtained) {
                                     return true;
                                 }
                             }
                             return false;
                         });

    REGISTER_ACHIEVEMENT(
        AID_RANDO_PLAYAS, "Play as Rando", "Play as a non-default form in randomizer mode", // Use AID_RANDO_PLAYAS
        (const char*)gItemIcons[ITEM_MASK_DEKU], false, 20, AchievementCategory::RANDOMIZER, OnItemGive,
        [](u8 item) { return (item == ITEM_MASK_DEKU || item == ITEM_MASK_GORON || item == ITEM_MASK_ZORA); });

    REGISTER_ACHIEVEMENT(AID_NOVICE_CHECK_HUNTER, "Novice Check Hunter", // Use AID_NOVICE_CHECK_HUNTER
                         "Obtain items from at least 100 different checks",
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 10, AchievementCategory::RANDOMIZER,
                         // Trigger when any Rando flag is set
                         OnFlagSet, [](FlagType flagType, u32 flag) {
                             // Check if it was a Rando flag
                             if (flagType != FLAG_RANDO_INF) {
                                 return false;
                             }
                             // Skip this check if the randomizer save data isn't initialized yet
                             if (!IS_RANDO) {
                                 return false;
                             }

                             // Count how many checks have been completed
                             int checkedLocations = 0;
                             for (size_t i = 0; i < RC_MAX; i++) {
                                 if (RANDO_SAVE_CHECKS[i].obtained) {
                                     checkedLocations++;
                                 }
                             }
                             return checkedLocations >= 100;
                         });

    REGISTER_ACHIEVEMENT(AID_ADEPT_CHECK_HUNTER, "Adept Check Hunter", // Use AID_ADEPT_CHECK_HUNTER
                         "Obtain items from at least 1000 different checks",
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 30, AchievementCategory::RANDOMIZER,
                         // Trigger when any Rando flag is set
                         OnFlagSet, [](FlagType flagType, u32 flag) {
                             // Check if it was a Rando flag
                             if (flagType != FLAG_RANDO_INF) {
                                 return false;
                             }
                             // Skip this check if the randomizer save data isn't initialized yet
                             if (!IS_RANDO) {
                                 return false;
                             }

                             // Count how many checks have been completed
                             int checkedLocations = 0;
                             for (size_t i = 0; i < RC_MAX; i++) {
                                 if (RANDO_SAVE_CHECKS[i].obtained) {
                                     checkedLocations++;
                                 }
                             }
                             return checkedLocations >= 1000;
                         });

    REGISTER_ACHIEVEMENT(AID_ALL_DONE, "All Done!", "Obtain all checks in a seed.", // Use AID_ALL_DONE
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 30, AchievementCategory::RANDOMIZER,
                         // Trigger when any Rando flag is set
                         OnFlagSet, [](FlagType flagType, u32 flag) {
                             // Check if it was a Rando flag
                             if (flagType != FLAG_RANDO_INF) {
                                 return false;
                             }
                             // Skip this check if the randomizer save data isn't initialized yet
                             if (!IS_RANDO) {
                                 return false;
                             }

                             int check_count = 0;
                             int checkedLocations = 0;
                             for (size_t i = 0; i < RC_MAX; i++) {
                                 // Count how many checks have been completed
                                 if (RANDO_SAVE_CHECKS[i].obtained) {
                                     checkedLocations++;
                                 }
                                 // Get the number of checks in the current seed.
                                 if (RANDO_SAVE_CHECKS[i].shuffled) {
                                     check_count++;
                                 }
                             }
                             return checkedLocations >= check_count;
                         });

    REGISTER_ACHIEVEMENT(AID_RANDO_MASTER, "Rando Master", "Obtain all checks in a max sanity.", // Use AID_RANDO_MASTER
                         (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 50, AchievementCategory::RANDOMIZER,
                         // Trigger when any Rando flag is set
                         OnFlagSet, [](FlagType flagType, u32 flag) {
                             // Check if it was a Rando flag
                             if (flagType != FLAG_RANDO_INF) {
                                 return false;
                             }
                             // Skip this check if the randomizer save data isn't initialized yet
                             if (!IS_RANDO) {
                                 return false;
                             }

                             // Count how many checks have been completed
                             int checkedLocations = 0;
                             for (size_t i = 0; i < RC_MAX; i++) {
                                 if (RANDO_SAVE_CHECKS[i].obtained) {
                                     checkedLocations++;
                                 }
                             }
                             return checkedLocations >= RC_MAX;
                         });

    REGISTER_ACHIEVEMENT(
        AID_RANDO_FIRST_TRY, "Boss Rush",
        "Defeat all four temple bosses in first cycle outside of Clock Town", // Use AID_RANDO_FIRST_TRY
        (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], true, 50, AchievementCategory::BOTH, OnFlagSet,
        [](FlagType flagType, u32 flag) {
            if (flagType != FLAG_WEEK_EVENT_REG ||
                !(flag == WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE || flag == WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE ||
                  flag == WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE || flag == WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE)) {
                return false; // Only check if a boss flag was set
            }

            // Check if player has defeated all bosses in this cycle
            bool hasAllRemains = CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE) &&
                                 CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE) &&
                                 CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE) &&
                                 CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE);

            // Check if player is in first cycle (before using Song of Time)
            // Note: reset count is 0 initially, 1 after first reset. Need <= 1.
            return hasAllRemains && (gSaveContext.save.saveInfo.playerData.threeDayResetCount <= 1);
        });

    SPDLOG_INFO("Finished registering achievement triggers.");
}

// --- Register the trigger function with ShipInit ---
// It will run at boot.
static RegisterShipInitFunc
    initFunc(RegisterAllAchievementTriggers, // The function containing all REGISTER_ACHIEVEMENT calls
             { CVAR_NAME_ACHIEVEMENTS });    // Depend on the main achievement CVar
