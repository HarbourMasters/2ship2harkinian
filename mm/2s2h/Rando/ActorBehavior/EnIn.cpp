

#include "ActorBehavior.h"
#include <libultraship/libultraship.h>

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_In/z_en_in.h"
void func_808F3D40(EnIn* enIn, PlayState* play);
void func_808F3C40(EnIn* enIn, PlayState* play);

void func_808F5A94(EnIn* enIn, PlayState* play);
void Player_TalkWithPlayer(PlayState* play, Actor* actor);
}

void EnIn_OnOpenText(u16* textId, bool* loadFromMessageTable) {
    RandoSaveCheck milkPurchaseCheck = RANDO_SAVE_CHECKS[RC_GORMAN_MILK_PURCHASE];
    RandoItemId riMilkPurchase = Rando::ConvertItem(milkPurchaseCheck.randoItemId, RC_GORMAN_MILK_PURCHASE);
    if (milkPurchaseCheck.cycleObtained) {
        return;
    }

    auto entry = CustomMessage::LoadVanillaMessageTableEntry(*textId);

    std::string itemName1 = Rando::StaticData::Items[riMilkPurchase].name;
    std::string itemPrice1 = std::to_string(milkPurchaseCheck.price);

    CustomMessage::Replace(&entry.msg, "drink", "%y{{item}}%w");
    CustomMessage::Replace(&entry.msg, "{{item}}", itemName1);
    CustomMessage::Replace(&entry.msg, "50", itemPrice1);
    CustomMessage::EnsureMessageEnd(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

/*
 * This is the same block found for non-scripted actors in OfferGetItem.cpp, with the removal of
 * Player_TalkWithPlayer() and addition of the rando check.
 */
void Rando::ActorBehavior::InitEnInBehavior() {
    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, IS_RANDO, {
        GetItemId* item = va_arg(args, GetItemId*);
        Actor* actor = va_arg(args, Actor*);
        if (actor->id == ACTOR_EN_IN) {
            if (*item == GI_MASK_GARO) {
                Player* player = GET_PLAYER(gPlayState);
                *should = false;
                RANDO_SAVE_CHECKS[RC_GORMAN_TRACK_GARO_MASK].eligible = true;
                actor->parent = &player->actor;
                player->talkActor = actor;
                player->talkActorDistance = actor->xzDistToPlayer;
                player->exchangeItemAction = PLAYER_IA_MINUS1;
            }
        }
    });

    COND_VB_SHOULD(VB_HAVE_GARO_MASK, IS_RANDO, { *should = RANDO_SAVE_CHECKS[RC_GORMAN_TRACK_GARO_MASK].obtained; });

    // Milk

    // Won'tcha buy some fresh milk?
    // COND_ID_HOOK(OnOpenText, 0x3463, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_SHOPS], EnIn_OnOpenText);

    // Buyin' milk?
    // COND_ID_HOOK(OnOpenText, 0x346B, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_SHOPS], EnIn_OnOpenText);

    // 50 Rupees will do ya for one drink!
    COND_ID_HOOK(OnOpenText, 0x3490, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_SHOPS], EnIn_OnOpenText);
    COND_ID_HOOK(OnOpenText, 0x3466, IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_SHOPS], EnIn_OnOpenText);

    COND_VB_SHOULD(VB_BUY_GORMAN_MILK, IS_RANDO, {
        s32* ret = va_arg(args, s32*);
        EnIn* enIn = va_arg(args, EnIn*);
        if (RANDO_SAVE_CHECKS[RC_GORMAN_MILK_PURCHASE].cycleObtained) {
            return;
        }
        *should = true;
        if (gPlayState->msgCtx.choiceIndex == 0) {
            Audio_PlaySfx_MessageDecide();
            if (gSaveContext.save.saveInfo.playerData.rupees >= RANDO_SAVE_CHECKS[RC_GORMAN_MILK_PURCHASE].price) {
                Player* player = GET_PLAYER(gPlayState);
                RANDO_SAVE_CHECKS[RC_GORMAN_MILK_PURCHASE].eligible = true;
                enIn->actionFunc = func_808F3C40;
                Rupees_ChangeBy(-RANDO_SAVE_CHECKS[RC_GORMAN_MILK_PURCHASE].price);
                *ret = true;

                enIn->actor.parent = &player->actor;
                player->talkActor = &enIn->actor;
                player->talkActorDistance = enIn->actor.xzDistToPlayer;
                player->exchangeItemAction = PLAYER_IA_MINUS1;

            } else {
                Audio_PlaySfx(NA_SE_SY_ERROR);
                Actor_ContinueText(gPlayState, &enIn->actor, 0x3468);
                *ret = false;
            }
        } else {
            Audio_PlaySfx_MessageCancel();
            Actor_ContinueText(gPlayState, &enIn->actor, 0x3491);
            *ret = false;
        }
    });
}
