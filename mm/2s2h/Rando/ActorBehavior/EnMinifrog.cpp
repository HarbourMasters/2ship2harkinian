#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "2s2h/CustomMessage/CustomMessage.h"

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Minifrog/z_en_minifrog.h"

// void Player_TalkWithPlayer(PlayState* play, Actor* actor);
}


// TODOs: update text, logic, fix postLimbDraw crash, drawFrog colors, check tracker icon, cleanup (rename VBs)

void EnMinifrog_OnOpenText(u16* textId, bool* loadFromMessageTable){

    // Need to change depending on frog
    RandoSaveCheck frogCheck = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG];
    RandoItemId riFrogCheck = Rando::ConvertItem(frogCheck.randoItemId, RC_CLOCK_TOWN_LAUNDRY_FROG);

    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

    entry.msg = "Here's %y{{item}}%w!\x13";

    std::string itemName = Rando::StaticData::Items[riFrogCheck].name;

    CustomMessage::ReplaceColorChars(&entry.msg);
    CustomMessage::Replace(&entry.msg, "{{item}}", itemName);
    CustomMessage::EnsureMessageEnd(&entry.msg);
    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE)) {
        entry.nextMessageID = 0x0D83;
    } else {
        entry.nextMessageID = 0x0D86;
    }
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

// static u16 sIsFrogReturnedFlags[] = {
//     0,                  // FROG_YELLOW
//     WEEKEVENTREG_32_40, // FROG_CYAN
//     WEEKEVENTREG_32_80, // FROG_PINK
//     WEEKEVENTREG_33_01, // FROG_BLUE
//     WEEKEVENTREG_33_02, // FROG_WHITE
// };

void Rando::ActorBehavior::InitEnMinifrogBehavior() {

    COND_VB_SHOULD(VB_SPAWN_FROG, IS_RANDO /*&& RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS]*/, {
        EnMinifrog* enMinifrog = va_arg(args, EnMinifrog*);
        switch (enMinifrog->frogIndex) {
            case 0:
                break;
            case 1:
                *should = RANDO_SAVE_CHECKS[RC_WOODFALL_TEMPLE_GEKKO_FROG].cycleObtained;
                break;
            case 2:
                *should = RANDO_SAVE_CHECKS[RC_GREAT_BAY_TEMPLE_GEKKO_FROG].cycleObtained;
                break;
            case 3:
                *should = RANDO_SAVE_CHECKS[RC_SOUTHERN_SWAMP_FROG].cycleObtained;
                break;
            case 4:
                *should = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG].cycleObtained;
                break;
        }
    });

    COND_VB_SHOULD(VB_FROG_TEMP1, IS_RANDO /**/, {
        EnMinifrog* enMinifrog = va_arg(args, EnMinifrog*);
        *should = true;
        switch (enMinifrog->frogIndex) {
            case 0:
                break;
            case 1:
                RANDO_SAVE_CHECKS[RC_WOODFALL_TEMPLE_GEKKO_FROG].eligible = true;
                break;
            case 2:
                RANDO_SAVE_CHECKS[RC_GREAT_BAY_TEMPLE_GEKKO_FROG].eligible = true;
                break;
            case 3:
                RANDO_SAVE_CHECKS[RC_SOUTHERN_SWAMP_FROG].eligible = true;
                break;
            case 4:
                RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG].eligible = true;
                break;
        }
    });
    
    // Can I pass actor into this?
    // COND_ID_HOOK(OnOpenText, 0x0D82, IS_RANDO /*&& RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS]*/, EnMinifrog_OnOpenText);
}
