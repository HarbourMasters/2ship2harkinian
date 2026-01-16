#include "ActorBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>

extern "C" {
#include "variables.h"
#include "overlays/actors/ovl_En_Fsn/z_en_fsn.h"
#include "overlays/actors/ovl_En_GirlA/z_en_girla.h"
void Player_StartTalking(PlayState* play, Actor* actor);
void EnFsn_ResumeInteraction(EnFsn* enFsn, PlayState* play);
}

#define ENFSN_END_CONVERSATION (1 << 0)
#define ENFSN_GAVE_KEATONS_MASK (1 << 2)
#define ENFSN_GAVE_LETTER_TO_MAMA (1 << 3)

void EnGirlA_RandoInit(EnGirlA* enGirlA, PlayState* play);

namespace {
constexpr s32 sCuriosityShopMaxItems = 3;
constexpr u16 sCuriosityShopBottleParams = SI_BOTTLE;

const std::array<Vec3f, sCuriosityShopMaxItems> sCuriosityShopItemPositions = {
    Vec3f{ -5.0f, 35.0f, -95.0f },
    Vec3f{ 13.0f, 35.0f, -95.0f },
    Vec3f{ 31.0f, 35.0f, -95.0f },
};

struct CuriosityRefillEntry {
    RandoCheckId checkId;
    RandoItemId itemId;
    RandoCheckId prerequisiteCheck; // Check that must be obtained before this refill is available
    u16 defaultPrice;
};

const std::array<CuriosityRefillEntry, 3> sCuriosityRefills = {
    CuriosityRefillEntry{ RC_CURIOSITY_SHOP_SEAHORSE, RI_SEAHORSE, RC_PINNACLE_ROCK_REUNITE_SEAHORSE,
                          100 },
    CuriosityRefillEntry{ RC_CURIOSITY_SHOP_REFILL_GOLD_DUST, RI_GOLD_DUST_REFILL, RC_UNKNOWN, 200 },
    CuriosityRefillEntry{ RC_CURIOSITY_SHOP_REFILL_CHATEAU, RI_CHATEAU_ROMANI_REFILL, RC_UNKNOWN, 200 },
};
} // namespace

void EndEnFsnDialogue(EnFsn* actor) {
    Player* player = GET_PLAYER(gPlayState);

    player->talkActor = &actor->actor;
    player->talkActorDistance = actor->actor.xzDistToPlayer;
    player->exchangeItemAction = PLAYER_IA_MINUS1;
    Player_StartTalking(gPlayState, &actor->actor);
    actor->flags |= ACTOR_FLAG_TALK;
    actor->actionFunc = EnFsn_ResumeInteraction;
}

void Rando::ActorBehavior::InitEnFsnBehavior() {
    COND_ID_HOOK(OnActorInit, ACTOR_EN_FSN, IS_RANDO, [](Actor* actor) {
        EnFsn* enFsn = (EnFsn*)actor;

        if (!ENFSN_IS_SHOP(&enFsn->actor) || gPlayState->sceneId != SCENE_AYASHIISHOP) {
            return;
        }

        s32 slotIndex = enFsn->totalSellingItems;
        if (slotIndex >= sCuriosityShopMaxItems) {
            return;
        }

        for (const auto& refill : sCuriosityRefills) {
            if (slotIndex >= sCuriosityShopMaxItems) {
                break;
            }

            // Check if the player has obtained the prerequisite item
            bool hasPrerequisite = false;
            if (refill.prerequisiteCheck != RC_UNKNOWN) {
                // For Seahorse, check the static check directly
                hasPrerequisite = RANDO_SAVE_CHECKS[refill.prerequisiteCheck].obtained;
            } else {
                // For Gold Dust and Chateau, find which check has the item and see if it's been obtained
                RandoItemId originalItemId;
                if (refill.itemId == RI_GOLD_DUST_REFILL) {
                    originalItemId = RI_BOTTLE_GOLD_DUST;
                } else if (refill.itemId == RI_CHATEAU_ROMANI_REFILL) {
                    originalItemId = RI_BOTTLE_CHATEAU_ROMANI;
                } else {
                    continue; // Unknown refill type
                }

                RandoCheckId itemPlacement = Rando::FindItemPlacement(originalItemId);
                if (itemPlacement != RC_UNKNOWN) {
                    hasPrerequisite = RANDO_SAVE_CHECKS[itemPlacement].obtained;
                }
            }

            // Skip spawning this refill if prerequisite not met
            if (!hasPrerequisite) {
                continue;
            }

            auto& saveCheck = RANDO_SAVE_CHECKS[refill.checkId];
            // Force the item to always be the specific refill, not randomized
            saveCheck.randoItemId = refill.itemId;

            if (!RANDO_SAVE_OPTIONS[RO_SHUFFLE_SHOPS] && saveCheck.price == 0) {
                saveCheck.price = refill.defaultPrice;
            }

            const Vec3f& spawnPos = sCuriosityShopItemPositions[slotIndex];
            EnGirlA* enGirlA =
                (EnGirlA*)Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_GIRLA, spawnPos.x, spawnPos.y,
                                      spawnPos.z, 0, 0, 0, sCuriosityShopBottleParams);
            if (enGirlA == nullptr) {
                continue;
            }

            enGirlA->actor.world.rot.z = refill.checkId;
            enGirlA->mainActionFunc = EnGirlA_RandoInit;
            enFsn->items[slotIndex] = enGirlA;
            enFsn->itemIds[slotIndex] = sCuriosityShopBottleParams;
            enFsn->totalSellingItems++;
            enFsn->numSellingItems++;
            slotIndex++;
        }
    });

    COND_VB_SHOULD(VB_GIVE_ITEM_FROM_OFFER, IS_RANDO, {
        GetItemId* item = va_arg(args, GetItemId*);
        Actor* actor = va_arg(args, Actor*);
        Player* player = GET_PLAYER(gPlayState);

        if (actor->id == ACTOR_EN_FSN) { // Curiosity Shop owner
            EnFsn* enFsn = (EnFsn*)actor;
            if (enFsn->getItemId == GI_MASK_KEATON || enFsn->getItemId == GI_LETTER_TO_MAMA) {
                *should = false;
                EndEnFsnDialogue(enFsn);
                return;
            }
            // Handling for when the Curiosity Shop owner sells something to the player
            if (enFsn->isSelling && enFsn->cursorIndex >= 0 && enFsn->cursorIndex <= 2) {
                EnGirlA* enGirlA = enFsn->items[enFsn->cursorIndex];
                RandoCheckId randoCheckId = (RandoCheckId)enGirlA->actor.world.rot.z;
                // Handle refill items and special checks
                if (randoCheckId == RC_CURIOSITY_SHOP_SPECIAL_ITEM ||
                    randoCheckId == RC_BOMB_SHOP_ITEM_04_OR_CURIOSITY_SHOP_ITEM ||
                    randoCheckId == RC_CURIOSITY_SHOP_SEAHORSE ||
                    randoCheckId == RC_CURIOSITY_SHOP_REFILL_GOLD_DUST ||
                    randoCheckId == RC_CURIOSITY_SHOP_REFILL_CHATEAU) {
                    *should = false;
                    EndEnFsnDialogue(enFsn);
                    enGirlA->buyFunc(gPlayState, enGirlA);
                    /*
                     * This notebook event must be faked because the randomized item probably won't be the All-Night
                     * Mask. There exists a minor bug where, if the player tries to sell something immediately after
                     * buying the special item, this notebook event will pop as Link pulls out the item to show. The
                     * Curiosity Shop owner's response will then erase that textbox. Not game breaking, but something to
                     * note.
                     */
                    if (randoCheckId == RC_CURIOSITY_SHOP_SPECIAL_ITEM) {
                        Message_BombersNotebookQueueEvent(gPlayState, BOMBERS_NOTEBOOK_EVENT_RECEIVED_ALL_NIGHT_MASK);
                    }
                }
            }
        }
    });

    COND_VB_SHOULD(VB_EN_FSN_HAS_ITEMS, IS_RANDO, {
        *should = true;
    });

    COND_VB_SHOULD(VB_GIVE_KEATON_MASK, IS_RANDO, {
        EnFsn* enFsn = va_arg(args, EnFsn*);
        RANDO_SAVE_CHECKS[RC_KAFEIS_HIDEOUT_KEATON_MASK].eligible = true;
        enFsn->flags |= ENFSN_GAVE_KEATONS_MASK;
        enFsn->textId = 0x29E2;
        EndEnFsnDialogue(enFsn);
        *should = false;
    });

    COND_VB_SHOULD(VB_GIVE_LETTER_TO_MAMA, IS_RANDO, {
        EnFsn* enFsn = va_arg(args, EnFsn*);
        RANDO_SAVE_CHECKS[RC_KAFEIS_HIDEOUT_LETTER_TO_MAMA].eligible = true;
        enFsn->flags |= ENFSN_END_CONVERSATION;
        enFsn->flags |= ENFSN_GAVE_LETTER_TO_MAMA;
        enFsn->textId = 0x29E4;
        enFsn->actionFunc = EnFsn_ResumeInteraction;
        SET_WEEKEVENTREG(WEEKEVENTREG_RECEIVED_PRIORITY_MAIL);
        *should = false;
    });
}
