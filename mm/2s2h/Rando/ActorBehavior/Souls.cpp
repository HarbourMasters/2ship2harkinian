#include "Souls.h"
#include "ActorBehavior.h"
#include "public/bridge/consolevariablebridge.h"
#include "Rando/DrawFuncs.h"

extern "C" {
#include "variables.h"
#include "functions.h"

#include "overlays/actors/ovl_Boss_Hakugin/z_boss_hakugin.h"

void BossHakugin_DrawIce(BossHakugin*, PlayState*);
}

// clang-format off
std::unordered_map<RandoItemId, std::tuple<std::function<void()>, std::vector<ActorId>, RandoInf>> soulMap = {
    { RI_SOUL_ARMOS,        { DrawArmos,        { ACTOR_EN_AM }, RANDO_INF_OBTAINED_SOUL_OF_ARMOS } },
    { RI_SOUL_BAT,          { DrawBat,          { ACTOR_EN_BAT }, RANDO_INF_OBTAINED_SOUL_OF_BATS } },
    { RI_SOUL_BEAMOS,       { DrawBeamos,       { ACTOR_EN_VM }, RANDO_INF_OBTAINED_SOUL_OF_BEAMOS } },
    { RI_SOUL_BOE,          { DrawBoe,          { ACTOR_EN_MKK }, RANDO_INF_OBTAINED_SOUL_OF_BOES } },
    { RI_SOUL_BOMBCHU,      { DrawRealBombchu,  { ACTOR_EN_RAT }, RANDO_INF_OBTAINED_SOUL_OF_BOMBCHU } },
    { RI_SOUL_DEATH_ARMOS,  { DrawDeathArmos,   { ACTOR_EN_FAMOS }, RANDO_INF_OBTAINED_SOUL_OF_DEATH_ARMOS } },
    { RI_SOUL_DEKU_BABA,    { DrawDekuBaba,     { ACTOR_EN_DEKUBABA, ACTOR_EN_KAREBABA, ACTOR_BOSS_05 }, RANDO_INF_OBTAINED_SOUL_OF_DEKU_BABAS } },
    { RI_SOUL_DINOLFOS,     { DrawDinolfos,     { ACTOR_EN_DINOFOS }, RANDO_INF_OBTAINED_SOUL_OF_DINOLFOS } },
    { RI_SOUL_DODONGO,      { DrawDodongo,      { ACTOR_EN_DODONGO }, RANDO_INF_OBTAINED_SOUL_OF_DODONGOS } },
    { RI_SOUL_EENO,         { DrawEeno,         { ACTOR_EN_SNOWMAN }, RANDO_INF_OBTAINED_SOUL_OF_EENOS } },
    { RI_SOUL_GARO,         { DrawGaroMaster,   { ACTOR_EN_JSO2, ACTOR_EN_JSO }, RANDO_INF_OBTAINED_SOUL_OF_GARO_MASTERS } },
    { RI_SOUL_GRASSHOPPER,  { DrawGrasshopper,  { ACTOR_EN_GRASSHOPPER }, RANDO_INF_OBTAINED_SOUL_OF_GRASSHOPPERS } },
    { RI_SOUL_GUAY,         { DrawGuay,         { ACTOR_EN_CROW, ACTOR_EN_RUPPECROW }, RANDO_INF_OBTAINED_SOUL_OF_GUAYS } },
    { RI_SOUL_HIPLOOP,      { DrawHiploop,      { ACTOR_EN_PP }, RANDO_INF_OBTAINED_SOUL_OF_HIPLOOPS } },
    { RI_SOUL_IRON_KNUCKLE, { DrawIronKnuckle,  { ACTOR_EN_IK }, RANDO_INF_OBTAINED_SOUL_OF_IRON_KNUCKLES } },
    { RI_SOUL_KEESE,        { DrawKeese,        { ACTOR_EN_FIREFLY }, RANDO_INF_OBTAINED_SOUL_OF_KEESE } },
    { RI_SOUL_LEEVER,       { DrawLeever,       { ACTOR_EN_NEO_REEBA }, RANDO_INF_OBTAINED_SOUL_OF_LEEVERS } },
    { RI_SOUL_MAD_SCRUB,    { DrawMadScrub,     { ACTOR_EN_DEKUNUTS }, RANDO_INF_OBTAINED_SOUL_OF_MAD_SCRUBS } },
    { RI_SOUL_OCTOROK,      { DrawOctorok,      { ACTOR_EN_OKUTA }, RANDO_INF_OBTAINED_SOUL_OF_OCTOROKS } },
    { RI_SOUL_PEAHAT,       { DrawPeahat,       { ACTOR_EN_PEEHAT }, RANDO_INF_OBTAINED_SOUL_OF_PEAHATS } },
    { RI_SOUL_REDEAD,       { DrawRedead,       { ACTOR_EN_RD, ACTOR_EN_RAILGIBUD }, RANDO_INF_OBTAINED_SOUL_OF_REDEADS } },
    { RI_SOUL_SHELLBLADE,   { DrawShellBlade,   { ACTOR_EN_SB }, RANDO_INF_OBTAINED_SOUL_OF_SHELLBLADES } },
    { RI_SOUL_SKULLFISH,    { DrawSkullfish,    { ACTOR_EN_PR, ACTOR_EN_PRZ, ACTOR_EN_PR2 }, RANDO_INF_OBTAINED_SOUL_OF_SKULLFISH } },
    { RI_SOUL_SKULLTULA,    { DrawSkulltula,    { ACTOR_EN_ST, ACTOR_EN_SW }, RANDO_INF_OBTAINED_SOUL_OF_SKULLTULAS } },
    { RI_SOUL_SLIME,        { DrawSlime,        { ACTOR_EN_SLIME }, RANDO_INF_OBTAINED_SOUL_OF_SLIMES } },
    { RI_SOUL_SNAPPER,      { DrawSnapper,      { ACTOR_EN_KAME }, RANDO_INF_OBTAINED_SOUL_OF_SNAPPERS } },
    { RI_SOUL_STALCHILD,    { DrawStalchild,    { ACTOR_EN_SKB, ACTOR_EN_RAIL_SKB }, RANDO_INF_OBTAINED_SOUL_OF_STALCHILDREN } },
    { RI_SOUL_TEKTITE,      { DrawTektite,      { ACTOR_EN_TITE }, RANDO_INF_OBTAINED_SOUL_OF_TEKTITES } },
    { RI_SOUL_WALLMASTER,   { DrawWallmaster,   { ACTOR_EN_WALLMAS, ACTOR_EN_FLOORMAS }, RANDO_INF_OBTAINED_SOUL_OF_WALLMASTERS } },
    { RI_SOUL_WOLFOS,       { DrawWolfos,       { ACTOR_EN_WF }, RANDO_INF_OBTAINED_SOUL_OF_WOLFOS } },
};
// clang-format on

RandoItemId GetRandoItemIdByActor(int16_t actorId) {
    for (auto& soul : soulMap) {
        for (auto& actor : std::get<1>(soul.second)) {
            if (actor == actorId) {
                return soul.first;
            }
        }
    }
    return RI_UNKNOWN;
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
    bool shouldEnemyRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_SHUFFLE_ENEMY_SOULS] == RO_GENERIC_YES;

    // ShouldActorDraw & ShouldActorUpdate for Enemy Souls
    COND_HOOK(ShouldActorDraw, shouldEnemyRegister, [](Actor* actor, bool* should) {
        if (actor->category != ACTORCAT_ENEMY || actor->category == ACTORCAT_BOSS) {
            return;
        }

        RandoItemId randoItemId = GetRandoItemIdByActor(actor->id);
        if (randoItemId == RI_UNKNOWN) {
            return;
        }

        auto findSoulFlag = soulMap.find(randoItemId);
        if (findSoulFlag != soulMap.end()) {
            ShouldActorDraw(actor, should, std::get<2>(findSoulFlag->second));
        }
    });

    COND_HOOK(ShouldActorUpdate, shouldEnemyRegister, [](Actor* actor, bool* should) {
        if (actor->category != ACTORCAT_ENEMY || actor->category == ACTORCAT_BOSS) {
            return;
        }

        RandoItemId randoItemId = GetRandoItemIdByActor(actor->id);
        if (randoItemId == RI_UNKNOWN) {
            return;
        }

        auto findSoulFlag = soulMap.find(randoItemId);
        if (findSoulFlag != soulMap.end()) {
            ShouldActorUpdate(actor, should, std::get<2>(findSoulFlag->second));
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
    COND_VB_SHOULD(VB_ITEM_BE_RESTRICTED, shouldRegister, {
        ItemId itemId = *va_arg(args, ItemId*);
        if (itemId == ITEM_MASK_GIANT && gPlayState->sceneId == SCENE_INISIE_BS &&
            !Flags_GetRandoInf(RANDO_INF_OBTAINED_SOUL_OF_TWINMOLD)) {
            *should = true;
        }
    });
}
