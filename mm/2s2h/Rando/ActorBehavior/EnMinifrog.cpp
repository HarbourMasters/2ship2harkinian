#include "ActorBehavior.h"

extern "C" {

#include "overlays/actors/ovl_En_Minifrog/z_en_minifrog.h"

#include "variables.h"
// void Player_TalkWithPlayer(PlayState* play, Actor* actor);
}

void EnMinifrog_OnOpenText(u16* textId, bool* loadFromMessageTable){

    // Need to change depending on frog
    RandoSaveCheck frogCheck = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG];
    RandoItemId riFrogCheck = Rando::ConvertItem(frogCheck.randoItemId, RC_CLOCK_TOWN_LAUNDRY_FROG);

    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

    entry.msg = "Here's %y{{item}}%w!";

    std::string itemName = Rando::StaticData::Items[riFrogCheck].name;

    CustomMessage::ReplaceColorChars(&entry.msg);
    CustomMessage::Replace(&entry.msg, "{{item}}", itemName);
    CustomMessage::EnsureMessageEnd(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;

    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE)) {
        Message_ContinueTextbox(gPlayState, 0xD83);
    } else {
        Message_ContinueTextbox(gPlayState, 0xD86);
    }
}

// static u16 sIsFrogReturnedFlags[] = {
//     0,                  // FROG_YELLOW
//     WEEKEVENTREG_32_40, // FROG_CYAN
//     WEEKEVENTREG_32_80, // FROG_PINK
//     WEEKEVENTREG_33_01, // FROG_BLUE
//     WEEKEVENTREG_33_02, // FROG_WHITE
// };

void Rando::ActorBehavior::InitEnMinifrogBehavior() {

    // COND_VB_SHOULD(VB_SPAWN_FROG, IS_RANDO /*&& RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS]*/, {
    //     // EnMinifrog* enMinifrog = va_arg(args, EnMinifrog*);
    //     // switch (enMinifrog->frogIndex) {
    //     //     case 0:
    //     //         break;
    //     //     case 1:
    //     //         *should = RANDO_SAVE_CHECKS[RC_WOODFALL_TEMPLE_GEKKO_FROG].cycleObtained;
    //     //         break;
    //     //     case 2:
    //     //         *should = RANDO_SAVE_CHECKS[RC_GREAT_BAY_TEMPLE_GEKKO_FROG].cycleObtained;
    //     //         break;
    //     //     case 3:
    //     //         *should = RANDO_SAVE_CHECKS[RC_SOUTHERN_SWAMP_FROG].cycleObtained;
    //     //         break;
    //     //     case 4:
    //     //         *should = RANDO_SAVE_CHECKS[RC_CLOCK_TOWN_LAUNDRY_FROG].cycleObtained;
    //     //         break;
    //     // }
    // });
    
    // Can I pass actor into this?
    COND_ID_HOOK(OnOpenText, 0x0D82, IS_RANDO /*&& RANDO_SAVE_OPTIONS[RO_SHUFFLE_FROGS]*/, EnMinifrog_OnOpenText);
}
