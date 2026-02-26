#include "MiscBehavior.h"
#include "2s2h/Rando/Logic/Logic.h"
#include "2s2h/Network/Archipelago/Archipelago.h"
#include "2s2h/Network/Archipelago/ArchipelagoBridge.h"

extern "C" {
#include "variables.h"
#include "functions.h"
}

// Entry point for the module, run once on game boot
void Rando::MiscBehavior::Init() {
    Rando::MiscBehavior::InitFileSelect();
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveInit>(Rando::MiscBehavior::OnFileCreate);
}

void Rando::MiscBehavior::OnFileLoad() {
    Rando::MiscBehavior::CheckQueueReset();
    Rando::MiscBehavior::InitKaleidoItemPage();
    Rando::MiscBehavior::InitOfferGetItemBehavior();
    Rando::MiscBehavior::SariasSongHint();

    // For Archipelago saves, repopulate location data from server
    if (IS_ARCHI) {
        // Clear session-only item dedupe to allow items to be received again
        // when loading an older save (e.g., soft reset without saving)
        ArchipelagoBridge::OnFileLoad();

        // NOTE: We do NOT mark all checks as shuffled here.
        // The shuffled status is loaded from the save file and properly set by
        // RepopulateLocationRewardsFromCache when location_info is available.

        // Try to reapply cached slot_data from AP server (if user connected before creating save)
        // This will apply the actual server options instead of defaults
        bool appliedCachedSlotData = ArchipelagoBridge::ReapplySlotOptionsFromCache();

        // If no cached slot_data was available (i.e., fresh save, not connected yet),
        // DON'T override options that are already in the save file
        // Only set defaults if options are all 0 (uninitialized)
        if (!appliedCachedSlotData) {
            // Check if options look uninitialized (all values are 0)
            bool needsInit =
                (RANDO_SAVE_OPTIONS[RO_SHUFFLE_SWIM] == 0 && RANDO_SAVE_OPTIONS[RO_SHUFFLE_ENEMY_SOULS] == 0 &&
                 RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] == 0 && RANDO_SAVE_OPTIONS[RO_LOGIC] == 0);

            if (needsInit) {
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_COWS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_TINGLE_SHOPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_TREE_DROPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_ENEMY_DROPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_SWIM] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_ENEMY_SOULS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_GRASS_DROPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_POT_DROPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_BARREL_DROPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_CRATE_DROPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_SNOWBALL_DROPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_FREESTANDING_ITEMS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_SHOPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_GOLD_SKULLTULAS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_OWL_STATUES] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_REMAINS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_SONG_SUN] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_SONG_DOUBLE_TIME] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_SONG_INVERTED_TIME] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_SONG_SARIA] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_OCARINA_BUTTONS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRAPS] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRIFORCE_PIECES] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_CLOCK_SHUFFLE] = RO_GENERIC_NO;
                RANDO_SAVE_OPTIONS[RO_LOGIC] = RO_LOGIC_GLITCHLESS;
                RANDO_SAVE_OPTIONS[RO_STARTING_MAPS_AND_COMPASSES] = RO_GENERIC_NO;
            }
        }

        // Repopulate location rewards from cached AP server data
        ArchipelagoBridge::RepopulateLocationRewardsFromCache();

        // Apply cached checked locations from server
        // This marks locations as obtained BEFORE scenes load, preventing items from spawning
        // This is crucial for new save files connecting to existing AP slots
        ArchipelagoBridge::ApplyCachedCheckedLocations();
    }

    COND_HOOK(OnFlagSet, IS_RANDO, Rando::MiscBehavior::OnFlagSet);
    COND_HOOK(OnSceneFlagSet, IS_RANDO, Rando::MiscBehavior::OnSceneFlagSet);
    COND_HOOK(BeforeEndOfCycleSave, IS_RANDO, Rando::MiscBehavior::BeforeEndOfCycleSave);
    COND_HOOK(AfterEndOfCycleSave, IS_RANDO, Rando::MiscBehavior::AfterEndOfCycleSave);
    COND_HOOK(OnSceneInit, IS_RANDO, Rando::MiscBehavior::OnSceneInit);
    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, IS_RANDO, [](Actor* actor) { Rando::MiscBehavior::CheckQueue(); });

    // This overrides the ocarina condition for Termina Field
    COND_VB_SHOULD(VB_TERMINA_FIELD_BE_EMPTY, IS_RANDO, { *should = false; });

    // Override faster first-cycle time speed if you don't have the Ocarina
    COND_VB_SHOULD(VB_FASTER_FIRST_CYCLE, IS_RANDO, { *should = false; });

    // The game normally only checks the trade slot for the room key directly, which would mean the player could be
    // denied entry to the Stock Pot Inn if they have the room key but it isn't assigned as the active item for the
    // slot. In rando, use this flag instead.
    COND_VB_SHOULD(VB_CHECK_FOR_ROOM_KEY, IS_RANDO, { *should = Flags_GetRandoInf(RANDO_INF_OBTAINED_ROOM_KEY); });

    // In the case of receiving a sword, we only want to equip it to the Human's B button. Vanilla avoids this issue by
    // never letting you be other forms when you get a sword from the smithy or curiosity shop.
    COND_VB_SHOULD(VB_ITEM_GIVE_SWORD_SET_FORM_EQUIP, IS_RANDO, {
        *should = false;
        // FD and human share equip slots, so do not change the equip slot if the player is FD.
        if (GET_PLAYER_FORM != PLAYER_FORM_FIERCE_DEITY) {
            u8* item = va_arg(args, u8*);
            BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = *item;
        }
    });

    // Fix vanilla bug where the player can often use magic before it's acquired.
    COND_VB_SHOULD(VB_GRANT_MAGIC_UPON_REQUEST, IS_RANDO, {
        if (!gSaveContext.save.saveInfo.playerData.isMagicAcquired) {
            *should = false;
            gSaveContext.isMagicRequested = false;
            gSaveContext.save.saveInfo.playerData.magic = 0;
            gSaveContext.magicToAdd = 0;
        }
    });

    COND_VB_SHOULD(VB_MEET_MOON_REQUIREMENTS, IS_RANDO, { *should = Rando::Logic::MeetsMoonRequirements(); });

    // Fix issue where bombchus/bombs can't be used in Honey and Darling if the player has no sword equipped and no bow.
    COND_VB_SHOULD(VB_CLEAR_B_BUTTON_FOR_NO_BOW, IS_RANDO, {
        // Playing Honey and Darling
        if (CHECK_WEEKEVENTREG(WEEKEVENTREG_08_01) && (gPlayState->sceneId == SCENE_BOWLING)) {
            *should = false;
        }
    });
}
