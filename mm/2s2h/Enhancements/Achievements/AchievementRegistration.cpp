#include "Achievements.h"
#include "AchievementDefinitions.h"
#include "2s2h/BenPort.h"
#include "2s2h/Rando/Rando.h"
#include "2s2h/Rando/Types.h"
#include "2s2h/ShipInit.hpp"
#include <libultraship/libultraship.h>

// Define the achievements configuration
#define CVAR_NAME_ACHIEVEMENTS "gEnhancements.Achievements.Enabled"
#define CVAR_ACHIEVEMENTS CVarGetInteger(CVAR_NAME_ACHIEVEMENTS, 1)

// Wrap registration in RegisterShipInitFunc
static RegisterShipInitFunc initFunc(
    []() {
        // Starting achievements
        REGISTER_ACHIEVEMENT("first_steps", "Beyond the Town Walls",
                             "Leave Clock Town and enter Termina Field for the first time",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_TERMINA_FIELD));

        // Boss achievements
        REGISTER_ACHIEVEMENT("defeat_odolwa", "Jungle Warrior", "Defeat Odolwa, Masked Jungle Warrior",
                             (const char*)gItemIcons[ITEM_REMAINS_ODOLWA], false, 20, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE));

        REGISTER_ACHIEVEMENT("defeat_goht", "Mountain Racer", "Defeat Goht, Masked Mechanical Monster",
                             (const char*)gItemIcons[ITEM_REMAINS_GOHT], false, 20, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE));

        REGISTER_ACHIEVEMENT("defeat_gyorg", "Ocean Conqueror", "Defeat Gyorg, Gargantuan Masked Fish",
                             (const char*)gItemIcons[ITEM_REMAINS_GYORG], false, 20, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE));

        REGISTER_ACHIEVEMENT("defeat_twinmold", "Desert Exterminator", "Defeat Twinmold, Giant Masked Insects",
                             (const char*)gItemIcons[ITEM_REMAINS_TWINMOLD], false, 20, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE));

        REGISTER_ACHIEVEMENT("defeat_majora", "Savior of Termina", "Defeat Majora and save Termina",
                             (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], false, 50, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_77_01));

        // Mask achievements
        REGISTER_ACHIEVEMENT("collect_deku_mask", "Deku Transformation", "Transform into Deku Link for the first time",
                             (const char*)gItemIcons[ITEM_MASK_DEKU], false, 10, AchievementCategory::BOTH, OnActorInit,
                             []() {
                                 // Check if player has used the Deku mask using the correct week event register flag
                                 return CHECK_WEEKEVENTREG(WEEKEVENTREG_30_10);
                             }());

        REGISTER_ACHIEVEMENT("collect_goron_mask", "Goron Transformation",
                             "Transform into Goron Link for the first time", (const char*)gItemIcons[ITEM_MASK_GORON],
                             false, 10, AchievementCategory::BOTH, OnActorInit, []() {
                                 // Check if player has used the Goron mask using the correct week event register flag
                                 return CHECK_WEEKEVENTREG(WEEKEVENTREG_30_20);
                             }());

        REGISTER_ACHIEVEMENT("collect_zora_mask", "Zora Transformation", "Transform into Zora Link for the first time",
                             (const char*)gItemIcons[ITEM_MASK_ZORA], false, 10, AchievementCategory::BOTH, OnActorInit,
                             []() {
                                 // Check if player has used the Zora mask using the correct week event register flag
                                 return CHECK_WEEKEVENTREG(WEEKEVENTREG_30_40);
                             }());

        REGISTER_ACHIEVEMENT("collect_postman_hat", "Delivery for a Link?", "Obtain the Postman's Hat",
                             (const char*)gItemIcons[ITEM_MASK_POSTMAN], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_POSTMAN) == ITEM_MASK_POSTMAN);

        REGISTER_ACHIEVEMENT("collect_allnight_mask", "Up for 24h/3d", "Obtain the All-Night Mask",
                             (const char*)gItemIcons[ITEM_MASK_ALL_NIGHT], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_ALL_NIGHT) == ITEM_MASK_ALL_NIGHT);

        REGISTER_ACHIEVEMENT("collect_blast_mask", "Face bomber", "Obtain the Blast Mask",
                             (const char*)gItemIcons[ITEM_MASK_BLAST], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_BLAST) == ITEM_MASK_BLAST);

        REGISTER_ACHIEVEMENT("collect_stone_mask", "I got a rock...", "Obtain the Stone Mask",
                             (const char*)gItemIcons[ITEM_MASK_STONE], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_STONE) == ITEM_MASK_STONE);

        REGISTER_ACHIEVEMENT("collect_greatfairy_mask", "Fairy collector", "Obtain the Great Fairy Mask",
                             (const char*)gItemIcons[ITEM_MASK_GREAT_FAIRY], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_GREAT_FAIRY) == ITEM_MASK_GREAT_FAIRY);

        REGISTER_ACHIEVEMENT("collect_keaton_mask", "Two-Tailed deceiver", "Obtain the Keaton Mask",
                             (const char*)gItemIcons[ITEM_MASK_KEATON], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_KEATON) == ITEM_MASK_KEATON);

        REGISTER_ACHIEVEMENT("collect_bremen_mask", "Leader of Animals", "Obtain the Bremen Mask",
                             (const char*)gItemIcons[ITEM_MASK_BREMEN], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_BREMEN) == ITEM_MASK_BREMEN);

        REGISTER_ACHIEVEMENT("collect_bunny_hood", "Speedy as a Hare", "Obtain the Bunny Hood",
                             (const char*)gItemIcons[ITEM_MASK_BUNNY], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_BUNNY) == ITEM_MASK_BUNNY);

        REGISTER_ACHIEVEMENT("collect_dongero_mask", "*Kero Kero*", "Obtain the Dongero Mask",
                             (const char*)gItemIcons[ITEM_MASK_DON_GERO], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_DON_GERO) == ITEM_MASK_DON_GERO);

        REGISTER_ACHIEVEMENT("collect_mask_of_scents", "Sniffer of many things", "Obtain the Mask of Scents",
                             (const char*)gItemIcons[ITEM_MASK_SCENTS], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_SCENTS) == ITEM_MASK_SCENTS);

        REGISTER_ACHIEVEMENT("collect_romani_mask", "Mark of maturity", "Obtain the Romani Mask",
                             (const char*)gItemIcons[ITEM_MASK_ROMANI], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_ROMANI) == ITEM_MASK_ROMANI);

        REGISTER_ACHIEVEMENT("collect_circus_leader_mask", "Troupe leader", "Obtain the Circus Leader's Mask",
                             (const char*)gItemIcons[ITEM_MASK_CIRCUS_LEADER], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_CIRCUS_LEADER) == ITEM_MASK_CIRCUS_LEADER);

        REGISTER_ACHIEVEMENT("collect_kafei_mask", "Town detective", "Obtain the Kafei Mask",
                             (const char*)gItemIcons[ITEM_MASK_KAFEIS_MASK], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_KAFEIS_MASK) == ITEM_MASK_KAFEIS_MASK);

        REGISTER_ACHIEVEMENT("collect_couples_mask", "Witness of union", "Obtain the Couple's Mask",
                             (const char*)gItemIcons[ITEM_MASK_COUPLE], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_COUPLE) == ITEM_MASK_COUPLE);

        REGISTER_ACHIEVEMENT("collect_mask_of_truth", "Seer of Truth", "Obtain the Mask of Truth",
                             (const char*)gItemIcons[ITEM_MASK_TRUTH], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_TRUTH) == ITEM_MASK_TRUTH);

        REGISTER_ACHIEVEMENT("collect_kamaro_mask", "Lord of the Dance", "Obtain the Kamaro Mask",
                             (const char*)gItemIcons[ITEM_MASK_KAMARO], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_KAMARO) == ITEM_MASK_KAMARO);

        REGISTER_ACHIEVEMENT("collect_gibdo_mask", "Mummified Taskmaster", "Obtain the Gibdo Mask",
                             (const char*)gItemIcons[ITEM_MASK_GIBDO], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_GIBDO) == ITEM_MASK_GIBDO);

        REGISTER_ACHIEVEMENT("collect_garo_mask", "Ninja Master", "Obtain the Garo Mask",
                             (const char*)gItemIcons[ITEM_MASK_GARO], false, 10, AchievementCategory::BOTH, OnActorInit,
                             INV_CONTENT(ITEM_MASK_GARO) == ITEM_MASK_GARO);

        REGISTER_ACHIEVEMENT("collect_captains_hat", "I'm the Captain now!", "Obtain the Captain's Hat",
                             (const char*)gItemIcons[ITEM_MASK_CAPTAIN], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_CAPTAIN) == ITEM_MASK_CAPTAIN);

        REGISTER_ACHIEVEMENT("collect_giants_hat", "5th Giant", "Obtain the Giant's Mask",
                             (const char*)gItemIcons[ITEM_MASK_GIANT], false, 10, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_GIANT) == ITEM_MASK_GIANT);

        REGISTER_ACHIEVEMENT("collect_fierce_deity", "God of War", "Obtain the Fierce Deity Mask",
                             (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], true, 30, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_MASK_FIERCE_DEITY) == ITEM_MASK_FIERCE_DEITY);

        REGISTER_ACHIEVEMENT("collect_all_masks", "Mask Collector", "Collect all 24 masks",
                             (const char*)gItemIcons[ITEM_MASK_TRUTH], true, 50, AchievementCategory::BOTH, OnActorInit,
                             []() {
                                 for (u8 i = ITEM_MASK_DEKU; i <= ITEM_MASK_GIANT; i++) {
                                     if (INV_CONTENT(i) == ITEM_NONE)
                                         return false;
                                 }
                                 return true;
                             }());

        // Event achievements
        REGISTER_ACHIEVEMENT("eavesdropper", "Eavesdropper", "Listen in on Anju and her Mothers conversation",
                             (const char*)gItemIcons[ITEM_ROOM_KEY], true, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_85_04));

        REGISTER_ACHIEVEMENT("unlimited_power", "Unlimited Power!", "Drink Chateau Romani",
                             (const char*)gItemIcons[ITEM_CHATEAU], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_DRANK_CHATEAU_ROMANI));

        REGISTER_ACHIEVEMENT("defeat_aliens", "Flatwoods Buster", "Defend Ranch from Alien threat",
                             (const char*)gItemIcons[ITEM_MILK_BOTTLE], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_DEFENDED_AGAINST_THEM));

        REGISTER_ACHIEVEMENT("hags_hero", "Hag's Hero", "Save Koume", (const char*)gItemIcons[ITEM_POTION_RED], false,
                             10, AchievementCategory::BOTH, OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_SAVED_KOUME));

        REGISTER_ACHIEVEMENT("speed_demon", "Speed Demon", "Win the Goron race",
                             (const char*)gItemIcons[ITEM_GOLD_DUST], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_41_08));

        // Heart piece achievements
        REGISTER_ACHIEVEMENT("collect_heart_container", "Heart of a Hero", "Collect your first Heart Container",
                             (const char*)gItemIcons[ITEM_HEART_CONTAINER], false, 10, AchievementCategory::BOTH,
                             OnActorInit, gSaveContext.save.saveInfo.playerData.healthCapacity > 0x30);

        REGISTER_ACHIEVEMENT("laborious_swimmer", "Laborious Swimmer", "Win the final Beaver race reward",
                             (const char*)gItemIcons[ITEM_MASK_ZORA], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_25_01));

        REGISTER_ACHIEVEMENT("town_sharkshooter", "Town Sharpshooter", "Win the final Town Shooting Gallery reward",
                             (const char*)gItemIcons[ITEM_BOW], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_TOWN_SHOOTING_GALLERY_HEART_PIECE));

        REGISTER_ACHIEVEMENT("swamp_sharkshooter", "Swamp Sharpshooter", "Win the final Swamp Shooting Gallery reward",
                             (const char*)gItemIcons[ITEM_BOW], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_SWAMP_SHOOTING_GALLERY_HEART_PIECE));

        REGISTER_ACHIEVEMENT("honey_and_darling_showstopper", "Honey and Darling Showstopper",
                             "Win the Honey & Darling Heart Piece", (const char*)gItemIcons[ITEM_BOW], false, 10,
                             AchievementCategory::BOTH, OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_22_80));

        REGISTER_ACHIEVEMENT("deku_champion", "Playground Champ", "Win final Deku Playground reward",
                             (const char*)gItemIcons[ITEM_DEKU_NUT], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_DEKU_PLAYGROUND_HEART_PIECE));

        REGISTER_ACHIEVEMENT("swamp_tourist", "Swamp Tourist", "Win the final Boat Tour minigame reward",
                             (const char*)gItemIcons[ITEM_DEED_SWAMP], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_26_40));

        REGISTER_ACHIEVEMENT("island_hopper", "Island Hopper", "Win the Greatbay Island minigame reward",
                             (const char*)gItemIcons[ITEM_DEED_OCEAN], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_26_40));

        REGISTER_ACHIEVEMENT(
            "deku_land_trader", "Deku real estate tycoon", "Trade all Deku Title Deeds",
            (const char*)gItemIcons[ITEM_DEED_LAND], false, 10, AchievementCategory::BOTH, OnActorInit, []() {
                return (CHECK_WEEKEVENTREG(WEEKEVENTREG_17_80) && CHECK_WEEKEVENTREG(WEEKEVENTREG_61_10) &&
                        CHECK_WEEKEVENTREG(WEEKEVENTREG_61_80) && CHECK_WEEKEVENTREG(WEEKEVENTREG_62_04) &&
                        CHECK_WEEKEVENTREG(WEEKEVENTREG_62_20));
            }());

        REGISTER_ACHIEVEMENT("poebuster", "Poebuster", "Win the Spirit House reward",
                             (const char*)gItemIcons[ITEM_BIG_POE], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_54_40));

        REGISTER_ACHIEVEMENT("seacrossed_reunion", "Seacrossed Reunion", "Reunite the Seahorses",
                             (const char*)gItemIcons[ITEM_SEAHORSE], false, 10, AchievementCategory::BOTH, OnActorInit,
                             CHECK_WEEKEVENTREG(WEEKEVENTREG_32_01));

        REGISTER_ACHIEVEMENT("max_health", "Full of Heart", "Obtain maximum health (20 hearts)",
                             (const char*)gItemIcons[ITEM_HEART_CONTAINER], true, 30, AchievementCategory::BOTH,
                             OnActorInit, gSaveContext.save.saveInfo.playerData.healthCapacity >= 0x140);

        // Fairy achievements
        REGISTER_ACHIEVEMENT("return_clock_town_fairy", "Fairy Rescue",
                             "Return the Stray Fairy to the Clock Town Great Fairy Fountain",
                             (const char*)gItemIcons[ITEM_MAGIC_JAR_SMALL], true, 5, AchievementCategory::BOTH,
                             OnActorInit, []() -> bool {
                                 if (IS_RANDO) {
                                     // In Rando, magic can come from anywhere, so just check if it's acquired.
                                     // No special handling for First Cycle Skip as that would require checking over
                                     // multiple frames Note: We may create a OnMagicConsume hook if user feedback is
                                     // strong enough
                                     return static_cast<bool>(gSaveContext.save.saveInfo.playerData.isMagicAcquired);
                                 } else {
                                     // In Vanilla, check the specific flag for returning the Clock Town stray fairy.
                                     return static_cast<bool>(CHECK_WEEKEVENTREG(WEEKEVENTREG_08_80));
                                 }
                             }());

        REGISTER_ACHIEVEMENT("collect_great_spin", "Master Spinner", "Obtain the Great Spin",
                             (const char*)gItemIcons[ITEM_SWORD_KOKIRI], true, 20, AchievementCategory::BOTH,
                             OnActorInit, CHECK_WEEKEVENTREG(WEEKEVENTREG_OBTAINED_GREAT_SPIN_ATTACK));

        REGISTER_ACHIEVEMENT("collect_double_magic", "Adept Magician", "Obtained Double Magic",
                             (const char*)gItemIcons[ITEM_MAGIC_JAR_BIG], true, 20, AchievementCategory::BOTH,
                             OnActorInit, gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired == true);

        REGISTER_ACHIEVEMENT("collect_double_defense", "Thick as Iron", "Obtained Double Defense",
                             (const char*)gItemIcons[ITEM_HEART_CONTAINER], true, 20, AchievementCategory::BOTH,
                             OnActorInit, gSaveContext.save.saveInfo.playerData.doubleDefense == true);

        REGISTER_ACHIEVEMENT("collect_great_fairy_sword", "Fairy's Champion", "Obtain the Great Fairy's Sword",
                             (const char*)gItemIcons[ITEM_SWORD_GREAT_FAIRY], true, 20, AchievementCategory::BOTH,
                             OnActorInit, INV_CONTENT(ITEM_SWORD_GREAT_FAIRY) == ITEM_SWORD_GREAT_FAIRY);

        // Song achievements
        REGISTER_ACHIEVEMENT("learn_song_of_time", "Time Traveler", "Learn the Song of Time",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, []() {
                                 bool skipFirstCycle = CVarGetInteger("gEnhancements.Cutscenes.SkipFirstCycle", 0);
                                 u16 resetCount = gSaveContext.save.saveInfo.playerData.threeDayResetCount;
                                 // If first cycle skip is on, count starts at 1, otherwise 0.
                                 // Achievement triggers after the first use of Song of Time.
                                 return skipFirstCycle ? (resetCount > 1) : (resetCount > 0);
                             }());

        REGISTER_ACHIEVEMENT(
            "learn_song_of_healing", "Soul Healer", "Use the Song of Healing to heal a soul for the first time",
            (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH, OnActorInit, []() {
                // Check if Darmani (Goron Mask), Mikau (Zora Mask), Pamela's Father, or Kamaro has been healed
                bool healedDarmani = CHECK_WEEKEVENTREG(WEEKEVENTREG_30_20);
                bool healedMikau = CHECK_WEEKEVENTREG(WEEKEVENTREG_30_40);
                bool healedPamelaFather = CHECK_WEEKEVENTREG(WEEKEVENTREG_75_20); // Flag for healing Gibdo dad
                bool healedKamaro = CHECK_WEEKEVENTREG(WEEKEVENTREG_82_04);       // Flag for healing Kamaro
                return healedDarmani || healedMikau || healedPamelaFather || healedKamaro;
            }());

        REGISTER_ACHIEVEMENT("learn_eponas_song", "Horse Whisperer", "Learn Epona's Song",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_EPONA));

        REGISTER_ACHIEVEMENT("learn_soaring", "Fly like a Owl", "Learn the Song of Soaring",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_SOARING));

        REGISTER_ACHIEVEMENT("learn_storms", "Song of Brotherly Love", "Learn the Song of Storms",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_STORMS));

        REGISTER_ACHIEVEMENT("learn_sonata", "A small misunderstanding", "Learn the Sonata of Awakening",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_SONATA));

        REGISTER_ACHIEVEMENT("learn_lullaby", "Silence...at last!", "Learn the Goron Lullaby",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_LULLABY));

        REGISTER_ACHIEVEMENT("learn_bossa_nova", "Miracle of Life", "Learn the New Wave Bossa Nova",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_BOSSA_NOVA));

        REGISTER_ACHIEVEMENT("learn_elegy", "Sunshine in Ikana", "Learn the Elegy of Emptiness",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_ELEGY));

        REGISTER_ACHIEVEMENT("learn_oath", "To me, my Giants!", "Learn the Oath to Order",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit, CHECK_QUEST_ITEM(QUEST_SONG_OATH));

        REGISTER_ACHIEVEMENT("learn_all_songs", "Musician of Termina", "Learn all the songs",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 30, AchievementCategory::BOTH,
                             OnActorInit, []() {
                                 // Check flags for all obtainable Majora's Mask songs
                                 u32 allSongsBits = (1 << QUEST_SONG_SONATA) | (1 << QUEST_SONG_LULLABY) |
                                                    (1 << QUEST_SONG_BOSSA_NOVA) | (1 << QUEST_SONG_ELEGY) |
                                                    (1 << QUEST_SONG_OATH) | (1 << QUEST_SONG_TIME) |
                                                    (1 << QUEST_SONG_HEALING) | (1 << QUEST_SONG_EPONA) |
                                                    (1 << QUEST_SONG_SOARING) | (1 << QUEST_SONG_STORMS);
                                 return (GET_SAVE_INVENTORY_QUEST_ITEMS & allSongsBits) == allSongsBits;
                             }());

        // Time-Loop Related Achievements
        REGISTER_ACHIEVEMENT("groundhog_day", "Groundhog Day", "Reset the time cycle 10 times",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::BOTH,
                             OnActorInit,
                             []() { return gSaveContext.save.saveInfo.playerData.threeDayResetCount >= 10; }());

        REGISTER_ACHIEVEMENT("trapped_in_time", "Trapped in Time", "Reset the time cycle 999 times",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 50, AchievementCategory::BOTH,
                             OnActorInit,
                             []() { return gSaveContext.save.saveInfo.playerData.threeDayResetCount == 999; }());

        // Randomizer-specific achievements
        REGISTER_ACHIEVEMENT("rando_first_item", "The Journey Begins", "Collect your first randomized item",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], false, 10, AchievementCategory::RANDOMIZER,
                             OnActorInit, []() {
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
                             }());

        REGISTER_ACHIEVEMENT("rando_playas", "Play as Rando", "Play as a non-default form in randomizer mode",
                             (const char*)gItemIcons[ITEM_MASK_DEKU], false, 20, AchievementCategory::RANDOMIZER,
                             OnActorInit, []() {
                                 // Check if player has transformed - this checks if any transformation mask is in
                                 // inventory
                                 return (INV_CONTENT(ITEM_MASK_DEKU) == ITEM_MASK_DEKU ||
                                         INV_CONTENT(ITEM_MASK_GORON) == ITEM_MASK_GORON ||
                                         INV_CONTENT(ITEM_MASK_ZORA) == ITEM_MASK_ZORA);
                             }());

        REGISTER_ACHIEVEMENT("novice_check_hunter", "Novice Check Hunter",
                             "Obtain items from at least 100 different checks",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 10, AchievementCategory::RANDOMIZER,
                             OnActorInit, []() {
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
                             }());

        REGISTER_ACHIEVEMENT("adept_check_hunter", "Adept Check Hunter",
                             "Obtain items from at least 1000 different checks",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 30, AchievementCategory::RANDOMIZER,
                             OnActorInit, []() {
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
                             }());

        REGISTER_ACHIEVEMENT("all_done", "All Done!", "Obtain all checks in a seed.",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 30, AchievementCategory::RANDOMIZER,
                             OnActorInit, []() {
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
                             }());

        REGISTER_ACHIEVEMENT("rando_master", "Rando Master", "Obtain all checks in a max sanity.",
                             (const char*)gItemIcons[ITEM_OCARINA_OF_TIME], true, 50, AchievementCategory::RANDOMIZER,
                             OnActorInit, []() {
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
                             }());

        REGISTER_ACHIEVEMENT(
            "rando_first_try", "Boss Rush", "Defeat all four temple bosses in first cycle outside of Clock Town",
            (const char*)gItemIcons[ITEM_MASK_FIERCE_DEITY], true, 50, AchievementCategory::BOTH, OnActorInit, []() {
                // Check if player has defeated all bosses in this cycle
                // This checks if all remains are in the inventory
                bool hasAllRemains = CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE) &&
                                     CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE) &&
                                     CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE) &&
                                     CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE);

                // Check if player is in first cycle (before using Song of Time)
                return hasAllRemains && (gSaveContext.save.saveInfo.playerData.threeDayResetCount <= 1);
            }());
    },
    { CVAR_NAME_ACHIEVEMENTS }); // Depend on the main achievement CVar
