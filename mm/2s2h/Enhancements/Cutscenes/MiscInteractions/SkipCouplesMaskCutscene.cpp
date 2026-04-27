#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/CustomItem/CustomItem.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "src/overlays/actors/ovl_En_An/z_en_an.h"
#include "src/overlays/actors/ovl_En_Test3/z_en_test3.h"
}

#define CVAR_NAME "gEnhancements.Cutscenes.SkipMiscInteractions"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static constexpr u16 GET_COUPLES_MASK_TEXT_ID = 0x85;

static Vec3f POSITION = { -420.0f, 210.0f, -160.0f };
static Vec3s ROTATION = { 0x0000, 0xD555, 0x0000 };

static void SkipHandleCouplesMaskCs(EnTest3* kafei) {
    EnAn* anju = (EnAn*)SubS_FindActor(gPlayState, NULL, ACTORCAT_NPC, ACTOR_EN_AN);
    if (anju != NULL) {
        anju->unk_3C0 = true;
        kafei->player.actor.world.pos = anju->actor.world.pos = POSITION;
        kafei->player.actor.world.rot = kafei->player.actor.shape.rot = anju->actor.world.rot = anju->actor.shape.rot =
            ROTATION;
    }

    if (GameInteractor_Should(VB_GIVE_COUPLES_MASK, true)) {
        GameInteractor::Instance->events.emplace_back(GIEventGiveItem{
            .showGetItemCutscene = !CVarGetInteger("gEnhancements.Cutscenes.SkipGetItemCutscenes", 0),
            .param = GID_MASK_COUPLE,
            .giveItem =
                [](Actor* actor, PlayState* play) {
                    CustomMessage::Entry text = CustomMessage::LoadVanillaMessageTableEntry(GET_COUPLES_MASK_TEXT_ID);
                    if (CUSTOM_ITEM_FLAGS & CustomItem::GIVE_ITEM_CUTSCENE) {
                        CustomMessage::SetActiveCustomMessage(text.msg, text);
                    } else {
                        CustomMessage::StartTextbox(text.msg + "\x1C\x02\x10", text);
                    }

                    Item_Give(gPlayState, ITEM_MASK_COUPLE);
                    Message_BombersNotebookQueueEvent(play, BOMBERS_NOTEBOOK_EVENT_MET_ANJU);
                    Message_BombersNotebookQueueEvent(play, BOMBERS_NOTEBOOK_EVENT_MET_KAFEI);
                    Message_BombersNotebookQueueEvent(play, BOMBERS_NOTEBOOK_EVENT_RECEIVED_COUPLES_MASK);
                },
        });
    }
}

static void RegisterSkipCouplesMaskCs() {
    COND_VB_SHOULD(VB_PLAY_COUPLES_MASK_CUTSCENE, CVAR, {
        if (*should) {
            EnTest3* kafei = va_arg(args, EnTest3*);
            s32* couplesMaskCsPhase = va_arg(args, s32*);
            *couplesMaskCsPhase = 2;
            *should = false;

            SkipHandleCouplesMaskCs(kafei);
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSkipCouplesMaskCs, { CVAR_NAME });
