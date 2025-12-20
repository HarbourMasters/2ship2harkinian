#include "ActorBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "Rando/DrawFuncs.h"

extern "C" {
#include "variables.h"
#include "functions.h"

#include "overlays/actors/ovl_Boss_Hakugin/z_boss_hakugin.h"

void BossHakugin_DrawIce(BossHakugin*, PlayState*);
}

bool shouldMajoraRegister() {
    bool registerStatus = false;
    if (IS_RANDO) {
        if (RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] == RO_GENERIC_YES ||
            RANDO_SAVE_OPTIONS[RO_SHUFFLE_TRIFORCE_PIECES] == RO_GENERIC_YES) {
            registerStatus = true;
        }
    }
    return registerStatus;
}

#define RI_TO_RANDO_INF(randoItemId) ((randoItemId - RI_SOUL_ARMOS) + RANDO_INF_OBTAINED_SOUL_OF_ARMOS)

// clang-format off
std::unordered_map<int16_t, RandoItemId> soulMap = {
    // TODO: Combine all Poes into one soul? regular, big, sisters. regular and big have no drops as of yet
    // TODO: Special cases: Igos du Ikana, Captain Keeta
    // FIXME: Real Bomchus and flying pots don't depend on attack collision to die; they can die from hitting anything
    // { ACTOR_EN_INVADEPOH, RI_SOUL_ALIEN },
    { ACTOR_EN_AM,          RI_SOUL_ARMOS },
    { ACTOR_EN_BAT,         RI_SOUL_BAD_BAT },
    { ACTOR_EN_VM,          RI_SOUL_BEAMOS },
    { ACTOR_EN_BB,          RI_SOUL_BUBBLE },
    { ACTOR_EN_BBFALL,      RI_SOUL_BUBBLE },
    { ACTOR_EN_MKK,         RI_SOUL_BOE },
    { ACTOR_EN_SLIME,       RI_SOUL_CHUCHU },
    { ACTOR_EN_FAMOS,       RI_SOUL_DEATH_ARMOS },
    // { ACTOR_EN_DRAGON, RI_SOUL_DEEP_PYTHON },
    { ACTOR_EN_DEKUBABA,    RI_SOUL_DEKU_BABA },
    { ACTOR_EN_KAREBABA,    RI_SOUL_DEKU_BABA },
    { ACTOR_BOSS_05,        RI_SOUL_DEKU_BABA },
    // { ACTOR_EN_WDHAND, RI_SOUL_DEXIHAND },
    { ACTOR_EN_DINOFOS,     RI_SOUL_DINOLFOS },
    { ACTOR_EN_DODONGO,     RI_SOUL_DODONGO },
    { ACTOR_EN_GRASSHOPPER, RI_SOUL_DRAGONFLY },
    { ACTOR_EN_SNOWMAN,     RI_SOUL_EENO },
    // { ACTOR_EN_EGOL, RI_SOUL_EYEGORE },
    { ACTOR_EN_TUBO_TRAP,   RI_SOUL_FLYING_POT },
    { ACTOR_EN_FZ,          RI_SOUL_FREEZARD },
    // { ACTOR_EN_JSO, RI_SOUL_GARO },
    // { ACTOR_EN_JSO2, RI_SOUL_GARO_MASTER },
    // { ACTOR_EN_BIGSLIME, RI_SOUL_GEKKO },
    // { ACTOR_EN_PAMETFROG, RI_SOUL_GEKKO },
    // { ACTOR_EN_BEE, RI_SOUL_GIANT_BEE },
    { ACTOR_EN_CROW,        RI_SOUL_GUAY },
    { ACTOR_EN_RUPPECROW,   RI_SOUL_GUAY },
    { ACTOR_EN_PP,          RI_SOUL_HIPLOOP },
    { ACTOR_EN_IK,          RI_SOUL_IRON_KNUCKLE },
    { ACTOR_EN_FIREFLY,     RI_SOUL_KEESE },
    { ACTOR_EN_NEO_REEBA,   RI_SOUL_LEEVER },
    { ACTOR_EN_RR,          RI_SOUL_LIKE_LIKE },
    { ACTOR_EN_DEKUNUTS,    RI_SOUL_MAD_SCRUB },
    { ACTOR_EN_BAGUO,       RI_SOUL_NEJIRON },
    { ACTOR_EN_OKUTA,       RI_SOUL_OCTOROK },
    { ACTOR_EN_PEEHAT,      RI_SOUL_PEAHAT },
    // { ACTOR_EN_KAIZOKU, RI_SOUL_PIRATE },
    // { ACTOR_EN_PO_SISTERS, RI_SOUL_POE_SISTER },
    { ACTOR_EN_RAT,         RI_SOUL_REAL_BOMBCHU },
    { ACTOR_EN_RD,          RI_SOUL_REDEAD },
    { ACTOR_EN_SB,          RI_SOUL_SHELLBLADE },
    { ACTOR_EN_PR,          RI_SOUL_SKULLFISH },
    { ACTOR_EN_PR2,         RI_SOUL_SKULLFISH },
    { ACTOR_EN_PRZ,         RI_SOUL_SKULLFISH },
    { ACTOR_EN_ST,          RI_SOUL_SKULLTULA },
    { ACTOR_EN_SW,          RI_SOUL_SKULLTULA },
    { ACTOR_EN_BIGPAMET,    RI_SOUL_SNAPPER },
    { ACTOR_EN_KAME,        RI_SOUL_SNAPPER },
    { ACTOR_EN_HINT_SKB,    RI_SOUL_STALCHILD },
    { ACTOR_EN_RAIL_SKB,    RI_SOUL_STALCHILD },
    { ACTOR_EN_SKB,         RI_SOUL_STALCHILD },
    // { ACTOR_EN_THIEFBIRD, RI_SOUL_TAKKURI },
    { ACTOR_EN_TITE,        RI_SOUL_TEKTITE },
    { ACTOR_EN_FLOORMAS,    RI_SOUL_WALLMASTER },
    { ACTOR_EN_WALLMAS,     RI_SOUL_WALLMASTER },
    // { ACTOR_BOSS_04, RI_SOUL_WART },
    // { ACTOR_EN_WIZ, RI_SOUL_WIZROBE },
    { ACTOR_EN_WF,          RI_SOUL_WOLFOS },
};
// clang-format on

void ShouldActorUpdate(Actor* actor, bool* should, RandoInf randoInf) {
    if (!Flags_GetRandoInf(randoInf)) {
        *should = false;
        actor->flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    } else if (!actor->flags & ACTOR_FLAG_ATTENTION_ENABLED) {
        actor->flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    }
}

void ShouldActorDraw(Actor* actor, bool* should, RandoInf randoInf) {
    if (!Flags_GetRandoInf(randoInf)) {
        *should = false;
    }
}

void Rando::ActorBehavior::InitSoulsBehavior() {
    bool shouldBossRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_BOSS_SOULS] == RO_GENERIC_YES;
    bool shouldEnemyInjure = IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_ENEMY_SOULS] == RO_GENERIC_YES;

    COND_VB_SHOULD(VB_PERFORM_AC_COLLISION, shouldEnemyInjure, {
        Collider* at = va_arg(args, Collider*);
        Collider* ac = va_arg(args, Collider*);

        auto findSoulFlag = soulMap.find(ac->actor->id);
        if (findSoulFlag != soulMap.end()) {
            RandoItemId randoItemId = findSoulFlag->second;
            if (randoItemId != RI_UNKNOWN) {

                if (!Flags_GetRandoInf(RI_TO_RANDO_INF(randoItemId))) {
                    *should = false;
                }
            }
        }
    });

    // ShouldActorDraw & ShouldActorUpdate for Boss Souls
    COND_ID_HOOK(ShouldActorDraw, ACTOR_BOSS_HAKUGIN, shouldBossRegister, [](Actor* actor, bool* should) {
        if (!Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_GOHT)) {
            BossHakugin_DrawIce((BossHakugin*)actor, gPlayState);
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_GOHT_UNFREEZE, shouldBossRegister, {
        if (!Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_GOHT)) {
            *should = false;
        }
    });

    COND_ID_HOOK(ShouldActorDraw, ACTOR_BOSS_03, shouldBossRegister,
                 [](Actor* actor, bool* should) { ShouldActorDraw(actor, should, RANDO_INF_OBTAINED_SOUL_OF_GYORG); });

    COND_ID_HOOK(ShouldActorDraw, ACTOR_BOSS_07, shouldMajoraRegister(),
                 [](Actor* actor, bool* should) { ShouldActorDraw(actor, should, RANDO_INF_OBTAINED_SOUL_OF_MAJORA); });

    COND_ID_HOOK(ShouldActorDraw, ACTOR_BOSS_01, shouldBossRegister,
                 [](Actor* actor, bool* should) { ShouldActorDraw(actor, should, RANDO_INF_OBTAINED_SOUL_OF_ODOLWA); });

    COND_ID_HOOK(ShouldActorDraw, ACTOR_BOSS_02, shouldBossRegister, [](Actor* actor, bool* should) {
        ShouldActorDraw(actor, should, RANDO_INF_OBTAINED_SOUL_OF_TWINMOLD);
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_BOSS_03, shouldBossRegister, [](Actor* actor, bool* should) {
        ShouldActorUpdate(actor, should, RANDO_INF_OBTAINED_SOUL_OF_GYORG);
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_BOSS_07, shouldMajoraRegister(), [](Actor* actor, bool* should) {
        ShouldActorUpdate(actor, should, RANDO_INF_OBTAINED_SOUL_OF_MAJORA);
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_BOSS_01, shouldBossRegister, [](Actor* actor, bool* should) {
        ShouldActorUpdate(actor, should, RANDO_INF_OBTAINED_SOUL_OF_ODOLWA);
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_BOSS_02, shouldBossRegister, [](Actor* actor, bool* should) {
        ShouldActorUpdate(actor, should, RANDO_INF_OBTAINED_SOUL_OF_TWINMOLD);
    });

    /*
     * Giant's Mask functionality is handled by two pieces. The scene (Twinmold's Lair) determines whether the mask can
     * be used, while the Twinmold actor itself handles the transformation. Boss Souls prevent Twinmold from updating
     * unless its soul has been obtained, which results in a softlock. In this case, disable the item.
     */
    COND_VB_SHOULD(VB_ITEM_BE_RESTRICTED, shouldBossRegister, {
        ItemId itemId = *va_arg(args, ItemId*);
        if (itemId == ITEM_MASK_GIANT && gPlayState->sceneId == SCENE_INISIE_BS &&
            !Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_TWINMOLD)) {
            *should = true;
        }
    });

    COND_VB_SHOULD(VB_DRAW_LOCK_ON_ARROW, shouldEnemyInjure, {
        Actor* refActor = va_arg(args, Actor*);
        auto findSoulFlag = soulMap.find(refActor->id);
        if (findSoulFlag != soulMap.end()) {
            RandoItemId randoItemId = findSoulFlag->second;
            if (!Flags_GetRandoInf(RI_TO_RANDO_INF(randoItemId))) {
                DrawEnLight({ 155, 0, 0 }, { 1.0f, 1.0f, 1.0f });
                *should = false;
            }
        }
    });
}
