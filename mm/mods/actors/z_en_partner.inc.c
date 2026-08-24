/*
 * z_en_partner.inc.c — Ivan the Fairy co-op partner (SoH ovl_En_Partner) ported 1:1 to MM.
 *
 * Source: Shipwright soh/src/overlays/actors/ovl_En_Partner/z_en_partner.c/.h
 * Included into the z_player.c TU from mods/items/logic/custom_items.c (unity build),
 * same pattern as somaria_cubes.c (included from item_cane_of_somaria.c).
 *
 * Registered as ACTOR_EN_PARTNER (0x2B8, mm/include/tables/actor_table.h); the
 * En_Partner_Profile at the bottom of this file is what the actor-table row binds to.
 * Spawned by mods/items/logic/item_hylias_grace.c HGrace_StateIvan (Spirit Medallion
 * possess mode): params = 0 → reads play->state.input[0] (Player 1); a second player
 * can drive Ivan with params = 1 while P1 keeps Link.
 *
 * OoT→MM adaptations (names only, behavior 1:1 — see report):
 *   gFairySkel/gFairyAnim(15 limbs)  → gameplay_keep_Skel_02AF58/Anim_029140 (FAIRY_LIMB_MAX=7, MM Tatl)
 *   Player_PlaySfx(actor)/Audio_PlayActorSound2 → Actor_PlaySfx
 *   func_8002F974                     → Actor_PlaySfx_Flagged
 *   Sfx_PlaySfxCentered               → Audio_PlaySfx
 *   Magic_RequestChange               → Magic_Consume
 *   Actor_MoveXZGravity               → Actor_MoveWithGravity
 *   Player_RequestQuake(play,s,y,c)   → Actor_RequestQuakeWithSpeed(play,y,c,s)
 *   func_8002F71C (player knockback)  → func_800B8D50(..., 0)
 *   func_8002836C (flame effect)      → func_800B3030
 *   EffectSsKiraKira_SpawnDispersed   → EffectSsKirakira_SpawnDispersed
 *   SkelAnime_DrawSkeleton2           → SkelAnime_Draw (MM Gfx*-returning variant)
 *   Graph_Alloc                       → GRAPH_ALLOC; MATRIX_NEWMTX → MATRIX_FINALIZE_AND_LOAD
 *   Math_Atan2S(x, y)                 → Math_Atan2S(y, x) (MM argument order is swapped)
 *   GET_PLAYER(play)->unk_A73 = 4     → player->unk_D57 = 4 (MM arrow-in-flight window)
 *   ARROW_NORMAL/FIRE/ICE/LIGHT/SEED/NUT → ARROW_TYPE_* (spawned with the final type directly
 *                                       so EnArrow_Init sets up the magic-arrow effects)
 *   ITEM_STICK/ITEM_NUT/ITEM_LENS     → ITEM_DEKU_STICK/ITEM_DEKU_NUT/ITEM_LENS_OF_TRUTH
 *   ITEM_DINS_FIRE/NAYRUS_LOVE/FARORES_WIND → ITEM_MEDALLION_FIRE/SHADOW/FOREST (NEI sentinel
 *                                       ids; matches the fork's SW97 medallion↔spell mapping)
 */

#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_En_Boom/z_en_boom.h"
#include "overlays/actors/ovl_En_Si/z_en_si.h"

typedef struct EnPartner {
    Actor actor;

    SkelAnime skelAnime;

    Vec3s jointTable[FAIRY_LIMB_MAX]; // OoT: 15 limbs (gFairySkel); MM fairy has FAIRY_LIMB_MAX (7)
    Vec3s morphTable[FAIRY_LIMB_MAX];

    ColliderCylinder collider;

    Color_RGBAf innerColor;
    Color_RGBAf outerColor;
    LightInfo lightInfoGlow;
    LightNode* lightNodeGlow;
    LightInfo lightInfoNoGlow;
    LightNode* lightNodeNoGlow;

    f32 yVelocity;

    u8 canMove;
    u8 usedItem;
    u8 usedItemButton;
    u8 usedSpell;
    u8 damageTimer;
    s16 magicTimer;

    u8 shouldDraw;
    s16 itemTimer;

    // TODO(MM): SoH had `GetItemEntry entry` (randomizer item table) here — MM has no
    // GetItemEntry; UseBeans gives ITEM_MAGIC_BEANS via Item_Give instead.
    WeaponInfo stickWeaponInfo;

    EnBoom* boomerangActor;
    Actor* hookshotTarget;
    Actor* windEffect; // Skijer's NEI: Sw97 MagicWind cast visual (real OoT Magic_Wind port) —
                       // spawned in UseFaroresWind, killed (if still alive) in EndFaroresWind
} EnPartner;

void EnPartner_Init(Actor* thisx, PlayState* play);
void EnPartner_Destroy(Actor* thisx, PlayState* play);
void EnPartner_Update(Actor* thisx, PlayState* play);
void EnPartner_Draw(Actor* thisx, PlayState* play);

// Runtime actor id consumed by item_hylias_grace.c (HGRACE_STATE_IVAN spawns it via
// `extern s16 gEnPartnerId`). Was a `= 0` stub in nei_link_stubs.cpp while the actor
// was unported; now it's the real table id. Skijer's NEI
s16 gEnPartnerId = ACTOR_EN_PARTNER;

// 2ship resource manager (BenPort.cpp) — frees the per-instance skeleton registration on destroy.
extern void ResourceMgr_UnregisterSkeleton(SkelAnime* skelAnime);
static void EnPartner_SpawnSparkles(EnPartner* this, PlayState* play, s32 sparkleLife);
static void UseItem(u8 usedItem, u8 started, Actor* thisx, PlayState* play);

static InitChainEntry sEnPartnerInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 8, ICHAIN_STOP),
};

static ColliderCylinderInit sEnPartnerCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_PLAYER,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        // OoT toucher dmgFlags 0x00020002 (deku-stick class hit) → MM DMG_DEKU_STICK
        { DMG_DEKU_STICK, 0x00, 0x01 },
        // OoT bumper dmgFlags 0x4FC00748 — MM damage bits are laid out differently;
        // accept-everything is the standard MM bumper mask. TODO(MM): curate per-bit.
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 12, 27, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit sEnPartnerCCInfoInit = { 0, 12, 60, MASS_HEAVY };

void EnPartner_Init(Actor* thisx, PlayState* play) {
    EnPartner* this = (EnPartner*)thisx;

    this->usedItem = 0xFF;
    this->canMove = 1;
    this->shouldDraw = 1;
    this->hookshotTarget = NULL;
    this->windEffect = NULL;
    this->boomerangActor = NULL;

    this->innerColor.r = 255.0f;
    this->innerColor.g = 255.0f;
    this->innerColor.b = 255.0f;
    this->innerColor.a = 255.0f;

    this->outerColor.r = 0.0f;
    this->outerColor.g = 255.0f;
    this->outerColor.b = 0.0f;
    this->outerColor.a = 255.0f;

    this->usedItemButton = 0xFF;

    Collider_InitCylinder(play, &this->collider);
    Collider_SetCylinder(play, &this->collider, &this->actor, &sEnPartnerCylinderInit);
    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetInfo(&this->actor.colChkInfo, NULL, &sEnPartnerCCInfoInit);
    this->actor.colChkInfo.mass = MASS_HEAVY;
    this->collider.base.ocFlags1 |= OC1_TYPE_PLAYER;
    this->collider.elem.atDmgInfo.damage = 1;
    GET_PLAYER(play)->ivanDamageMultiplier = 1;

    Actor_ProcessInitChain(thisx, sEnPartnerInitChain);
    SkelAnime_Init(play, &this->skelAnime, &gameplay_keep_Skel_02AF58, &gameplay_keep_Anim_029140, this->jointTable,
                   this->morphTable, FAIRY_LIMB_MAX);
    ActorShape_Init(&thisx->shape, 1000.0f, ActorShadow_DrawCircle, 15.0f);
    thisx->shape.shadowAlpha = 0xFF;

    Lights_PointGlowSetInfo(&this->lightInfoGlow, thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z, 200, 255,
                            200, 0);
    this->lightNodeGlow = LightContext_InsertLight(play, &play->lightCtx, &this->lightInfoGlow);

    Lights_PointNoGlowSetInfo(&this->lightInfoNoGlow, thisx->world.pos.x, thisx->world.pos.y, thisx->world.pos.z, 200,
                              255, 200, 0);
    this->lightNodeNoGlow = LightContext_InsertLight(play, &play->lightCtx, &this->lightInfoNoGlow);

    thisx->room = -1;
}

void EnPartner_Destroy(Actor* thisx, PlayState* play) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->hookshotTarget != NULL) {
        Actor_Kill(this->hookshotTarget);
        this->hookshotTarget = NULL;
    }

    if (this->windEffect != NULL) {
        Actor_Kill(this->windEffect);
        this->windEffect = NULL;
    }

    Player* player = GET_PLAYER(play);
    if (player) {
        player->ivanDamageMultiplier = 1;
    }

    LightContext_RemoveLight(play, &play->lightCtx, this->lightNodeGlow);
    LightContext_RemoveLight(play, &play->lightCtx, this->lightNodeNoGlow);

    Collider_DestroyCylinder(play, &this->collider);

    ResourceMgr_UnregisterSkeleton(&this->skelAnime);
}

static void EnPartner_UpdateLights(EnPartner* this, PlayState* play) {
    s16 glowLightRadius = 100;
    s16 lightRadius = 200;
    Player* player;

    if (this->shouldDraw == 0) {
        glowLightRadius = 0;
        lightRadius = 0;
    }

    player = GET_PLAYER(play);
    Lights_PointNoGlowSetInfo(&this->lightInfoNoGlow, player->actor.world.pos.x, (s16)(player->actor.world.pos.y) + 69,
                              player->actor.world.pos.z, 200, 255, 200, lightRadius);

    Lights_PointGlowSetInfo(&this->lightInfoGlow, this->actor.world.pos.x, this->actor.world.pos.y + 9,
                            this->actor.world.pos.z, 200, 255, 200, glowLightRadius);

    Actor_SetScale(&this->actor, this->actor.scale.x);
}

static void EnPartner_SpawnSparkles(EnPartner* this, PlayState* play, s32 sparkleLife) {
    static Vec3f sparkleVelocity = { 0.0f, -0.05f, 0.0f };
    static Vec3f sparkleAccel = { 0.0f, -0.025f, 0.0f };
    Vec3f sparklePos;
    Color_RGBA8 primColor;
    Color_RGBA8 envColor;

    sparklePos.x = Rand_CenteredFloat(6.0f) + this->actor.world.pos.x;
    sparklePos.y = (Rand_ZeroOne() * 6.0f) + this->actor.world.pos.y + 5;
    sparklePos.z = Rand_CenteredFloat(6.0f) + this->actor.world.pos.z;

    primColor.r = this->innerColor.r;
    primColor.g = this->innerColor.g;
    primColor.b = this->innerColor.b;
    primColor.a = 255;

    envColor.r = this->outerColor.r;
    envColor.g = this->outerColor.g;
    envColor.b = this->outerColor.b;
    envColor.a = 255;

    EffectSsKirakira_SpawnDispersed(play, &sparklePos, &sparkleVelocity, &sparkleAccel, &primColor, &envColor, 1500,
                                    sparkleLife);
}

static Vec3f EnPartner_Vec3fNormalize(Vec3f vec) {
    f32 norm = sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));

    if (norm != 0.0f) {
        vec.x /= norm;
        vec.y /= norm;
        vec.z /= norm;
    } else {
        vec.x = vec.y = vec.z = 0.0f;
    }

    return vec;
}

static void CenterIvanOnLink(Actor* thisx, PlayState* play) {
    EnPartner* this = (EnPartner*)thisx;
    this->actor.world.pos = GET_PLAYER(play)->actor.world.pos;
    this->actor.world.pos.y += Player_GetHeight(GET_PLAYER(play)) + 5.0f;
}

static u8 sEnPartnerMagicArrowCosts[] = { 0, 4, 4, 8 };

static void UseBow(Actor* thisx, PlayState* play, u8 started, u8 arrowType) {
    EnPartner* this = (EnPartner*)thisx;

    if (started == 1) {
        Actor_PlaySfx(&this->actor, NA_SE_PL_CHANGE_ARMS);
        this->canMove = 0;
    } else if (started == 0) {
        if (this->itemTimer <= 0) {
            if (AMMO(ITEM_BOW) > 0) {
                if (arrowType >= 1 && !Magic_Consume(play, sEnPartnerMagicArrowCosts[arrowType], MAGIC_CONSUME_NOW)) {
                    Audio_PlaySfx(NA_SE_SY_ERROR);
                    this->canMove = 1;
                    return;
                }

                this->itemTimer = 10;

                // OoT spawned ARROW_NORMAL then overwrote params; MM EnArrow_Init sets up the
                // elemental effects at init time, so spawn with the final type directly.
                static s16 sArrowTypes[] = { ARROW_TYPE_NORMAL, ARROW_TYPE_FIRE, ARROW_TYPE_ICE, ARROW_TYPE_LIGHT };
                Actor* newarrow =
                    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_ARROW, this->actor.world.pos.x,
                                       this->actor.world.pos.y + 7, this->actor.world.pos.z, 0, this->actor.world.rot.y,
                                       0, sArrowTypes[arrowType]);

                if (newarrow != NULL) {
                    GET_PLAYER(play)->unk_D57 = 4; // OoT unk_A73 = 4: keeps parentless EnArrow alive to fire
                    newarrow->parent = NULL;
                    Inventory_ChangeAmmo(ITEM_BOW, -1);
                }
            }
        }
    }
}

static void UseSlingshot(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (started == 1) {
        Actor_PlaySfx(&this->actor, NA_SE_PL_CHANGE_ARMS);
        this->canMove = 0;
    } else if (started == 0) {
        if (this->itemTimer <= 0) {
            if (AMMO(ITEM_SLINGSHOT) > 0) {
                this->itemTimer = 10;
                Actor* newarrow =
                    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_ARROW, this->actor.world.pos.x,
                                       this->actor.world.pos.y + 7, this->actor.world.pos.z, 0, this->actor.world.rot.y,
                                       0, ARROW_TYPE_SLINGSHOT);
                if (newarrow != NULL) {
                    GET_PLAYER(play)->unk_D57 = 4;
                    newarrow->parent = NULL;
                    Inventory_ChangeAmmo(ITEM_SLINGSHOT, -1);
                }
            } else {
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
        }
    }
}

static void UseBombs(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            if (AMMO(ITEM_BOMB) > 0 && play->actorCtx.actorLists[ACTORCAT_EXPLOSIVES].length < 3) {
                this->itemTimer = 10;
                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOM, this->actor.world.pos.x, this->actor.world.pos.y + 7,
                            this->actor.world.pos.z, 0, 0, 0, BOMB_TYPE_BODY);
                Inventory_ChangeAmmo(ITEM_BOMB, -1);
            } else {
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
        }
    }
}

static void UseHammer(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            static Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
            Vec3f shockwavePos = this->actor.world.pos;

            this->itemTimer = 10;

            // OoT Player_RequestQuake(play, speed=27767, y=7, countdown=20)
            Actor_RequestQuakeWithSpeed(play, 7, 20, 27767);
            Actor_PlaySfx(&this->actor, NA_SE_IT_HAMMER_HIT);

            EffectSsBlast_SpawnWhiteShockwave(play, &shockwavePos, &zeroVec, &zeroVec);

            if (this->actor.xzDistToPlayer < 100.0f && this->actor.playerHeightRel > -35.0f &&
                this->actor.playerHeightRel < 35.0f) {
                // OoT func_8002F71C: knock the player back away from Ivan
                func_800B8D50(play, &this->actor, 8.0f, this->actor.yawTowardsPlayer, 8.0f, 0);
            }
        }
    }
}

static void UseBombchus(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            if (AMMO(ITEM_BOMBCHU) > 0) {
                this->itemTimer = 10;
                // SoH parity: bombchus are an instantly-exploding bomb (timer = 0)
                EnBom* bomb =
                    (EnBom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOM, this->actor.world.pos.x,
                                        this->actor.world.pos.y + 7, this->actor.world.pos.z, 0, 0, 0, BOMB_TYPE_BODY);
                if (bomb != NULL) {
                    bomb->timer = 0;
                }
                Inventory_ChangeAmmo(ITEM_BOMBCHU, -1);
            } else {
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
        }
    }
}

static void UseDekuStick(Actor* thisx, PlayState* play, u8 started) {
    static Vec3f sStickFlameVel = { 0.0f, 0.5f, 0.0f };
    static Vec3f sStickFlameAccel = { 0.0f, 0.5f, 0.0f };
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            if (AMMO(ITEM_DEKU_STICK) > 0) {
                Actor_PlaySfx(&this->actor, NA_SE_EV_FLAME_IGNITION);
            } else {
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
        }

        if (started == 2) {
            if (AMMO(ITEM_DEKU_STICK) > 0) {
                this->stickWeaponInfo.tip = this->actor.world.pos;
                this->stickWeaponInfo.tip.y += 7.0f;

                // OoT func_8002836C(play, pos, vel, accel, prim, env, 200.0f, 0, 8) — MM's flame
                // spawner takes no colors (always the standard flame ramp).
                func_800B3030(play, &this->stickWeaponInfo.tip, &sStickFlameVel, &sStickFlameAccel, 200, 0, 0);

                CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);

                if (this->damageTimer <= 0) {
                    Inventory_ChangeAmmo(ITEM_DEKU_STICK, -1);
                    this->damageTimer = 20;
                } else {
                    this->damageTimer--;
                }
            }
        }
    }
}

// #region IvanWindEffect
// Skijer's NEI: SoH built a custom DemoEffect (OoT ACTOR_DEMO_EFFECT, timewarp pillar from
// gTimeWarpSkel/gTimeWarpAnim via SkelCurve) as the Farore's Wind visual. MM has no Demo_Effect
// actor, but the Sw97 MagicWind actor IS the real OoT Magic_Wind overlay port — UseFaroresWind
// spawns it at Ivan (params 0 = cast grow look, with the no-tornado one-shot latched so the
// Forest-medallion tornado extension stays off), on top of the fully functional wind PHYSICS,
// magic drain and sfx below.
// #endregion

static void EndFaroresWind(EnPartner* this, PlayState* play) {
    if (this->windEffect != NULL) {
        // Skijer's NEI: MagicWind self-kills at the end of its fade (unlike SoH's DemoEffect
        // pillar, which lived until killed) — only Actor_Kill the cached pointer if it is still
        // in the ITEMACTION actor list (Sw97_MagicWind_Profile's category).
        Actor* actor = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;

        while (actor != NULL) {
            if (actor == this->windEffect) {
                Actor_Kill(actor);
                break;
            }
            actor = actor->next;
        }
        this->windEffect = NULL;
    }
    gSaveContext.magicState = MAGIC_STATE_RESET;
    this->itemTimer = 5;
    this->usedItem = 0xFF;
}

static void UseFaroresWind(EnPartner* this, PlayState* play, u8 started) {
    Player* player = GET_PLAYER(play);

    if (started == 1) {
        if (gSaveContext.save.saveInfo.playerData.magic <= 0 || gSaveContext.magicState != MAGIC_STATE_IDLE) {
            Audio_PlaySfx(NA_SE_SY_ERROR);
            this->usedItem = 0xFF;
            return;
        }

        // Skijer's NEI: Farore's Wind visual at Ivan's position — the VANILLA Farore's Wind actor
        // (ACTOR_OOT_FARORES_WIND, params 0 = cast-grow swirl) is the pure OoT wind with NO tornado
        // (the actor branches on its own id; the Forest Medallion's ACTOR_SW97_MAGIC_WIND is the one
        // with the tornado). No more gOotSpellsWindNoTornado latch.
        this->windEffect = Actor_Spawn(&play->actorCtx, play, ACTOR_OOT_FARORES_WIND, this->actor.world.pos.x,
                                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, 0);
        this->magicTimer = 0;
    }

    if (started == 1 || started == 2) {
        Actor_PlaySfx_Flagged(&this->actor, NA_SE_EV_WIND_TRAP - SFX_FLAG);

        // based on BgHakaTrap_FanBlade_UpdateFanRotation
        Vec3f playerRel;
        Actor_WorldToActorCoords(&this->actor, &playerRel, &player->actor.world.pos);
        // TODO(MM): OoT also required (player->currentBoots != PLAYER_BOOTS_IRON); MM has no iron boots.
        if ((fabsf(playerRel.x) < 70.0f) && (fabsf(playerRel.y) < 100.0f) && (playerRel.z < 400.0f) &&
            (playerRel.z > 0)) {
            f32 factor = (1.0f - (playerRel.z / 400.0f));
            factor = sqrtf(factor);
            player->pushedSpeed = factor * 10.0f;
            player->pushedYaw = this->actor.shape.rot.y;
        }

        gSaveContext.magicState = MAGIC_STATE_METER_FLASH_1;
        this->magicTimer--;
        if (this->magicTimer <= 0) {
            if (gSaveContext.save.saveInfo.playerData.magic <= 0) {
                gSaveContext.save.saveInfo.playerData.magic = 0;
                EndFaroresWind(this, play);
                return;
            }
            gSaveContext.save.saveInfo.playerData.magic--; // Note: after the `if` statement so that the last
            this->magicTimer = 20;                         // tick of magic gives the full count of frames
        }
    }

    if (started == 0) {
        EndFaroresWind(this, play);
        return;
    }
}

static void UseNuts(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            if (AMMO(ITEM_DEKU_NUT) > 0) {
                this->itemTimer = 10;
                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_ARROW, this->actor.world.pos.x, this->actor.world.pos.y + 7,
                            this->actor.world.pos.z, 0x1000, this->actor.world.rot.y, 0, ARROW_TYPE_DEKU_NUT);
                Inventory_ChangeAmmo(ITEM_DEKU_NUT, -1);
            } else {
                Audio_PlaySfx(NA_SE_SY_ERROR);
            }
        }
    }
}

static void UseHookshot(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            Actor_PlaySfx(&this->actor, NA_SE_PL_CHANGE_ARMS);
            this->canMove = 0;
            // MM has the same Obj_Hsblock actor family; params 2 = hookshot post, as in OoT.
            this->hookshotTarget =
                Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_OBJ_HSBLOCK, this->actor.world.pos.x,
                                   this->actor.world.pos.y + 7.5f, this->actor.world.pos.z, this->actor.world.rot.x,
                                   this->actor.world.rot.y, this->actor.world.rot.z, 2);
            if (this->hookshotTarget != NULL) {
                this->hookshotTarget->scale.x = 0.05f;
                this->hookshotTarget->scale.y = 0.05f;
                this->hookshotTarget->scale.z = 0.05f;
            }
        } else if (started == 0) {
            if (this->hookshotTarget != NULL) {
                Actor_Kill(this->hookshotTarget);
                this->hookshotTarget = NULL;
            }
            Actor_PlaySfx(&this->actor, NA_SE_PL_CHANGE_ARMS);
            this->canMove = 1;
        } else if (started == 2) {
            if (this->hookshotTarget != NULL) {
                this->hookshotTarget->shape.rot.y = this->actor.world.rot.y;
            }
        }
    }
}

static void UseOcarina(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            // TODO(MM): SoH used Audio_PlaySoundTransposed(&projectedPos, sfx, -6); MM has no
            // transpose helper — plays Navi's "hello" untransposed.
            Actor_PlaySfx(&this->actor, NA_SE_VO_NA_HELLO_2);
        }
    }
}

static void UseBoomerang(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            this->itemTimer = 20;

            f32 posX = (Math_SinS(this->actor.shape.rot.y) * 1.0f) + this->actor.world.pos.x;
            f32 posZ = (Math_CosS(this->actor.shape.rot.y) * 1.0f) + this->actor.world.pos.z;
            s32 yaw = this->actor.shape.rot.y;
            // OOT_BOOMERANG param = NEI's OoT single-boomerang behavior (same as the
            // item_oot_boomerang.c port). OoT returnTimer=20 → MM unk_1CC = 20 + 16 = 36
            // (MM's En_Boom starts returning at unk_1CC <= 16).
            EnBoom* boomerang =
                (EnBoom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOOM, posX, this->actor.world.pos.y + 7.0f, posZ,
                                     this->actor.focus.rot.x, yaw, 0, OOT_BOOMERANG);

            this->boomerangActor = boomerang;
            if (boomerang != NULL) {
                boomerang->unk_1CC = 36;
                Actor_PlaySfx(&this->actor, NA_SE_IT_BOOMERANG_THROW);
            }
        }
    }
}

static void UseLens(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            Audio_PlaySfx(NA_SE_SY_GLASSMODE_ON);
            this->shouldDraw = 0;
        }

        if (started == 0) {
            Audio_PlaySfx(NA_SE_SY_GLASSMODE_OFF);
            this->shouldDraw = 1;
        }
    }
}

static void UseBeans(Actor* thisx, PlayState* play, u8 started) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->itemTimer <= 0) {
        if (started == 1) {
            if (play->actorCtx.titleCtx.alpha <= 0) {
                // SoH: GiveItemEntryWithoutActor(GI_BEAN) for 100 rupees → MM: Item_Give a magic bean.
                if (gSaveContext.save.saveInfo.playerData.rupees >= 100) {
                    Item_Give(play, ITEM_MAGIC_BEANS);
                    Rupees_ChangeBy(-100);
                } else {
                    Audio_PlaySfx(NA_SE_SY_ERROR);
                }
            }
        }
    }
}

static void UseSpell(Actor* thisx, PlayState* play, u8 started, u8 spellType) {
    EnPartner* this = (EnPartner*)thisx;

    if (gSaveContext.save.saveInfo.playerData.magic > 0) {
        if (this->itemTimer <= 0 && this->usedSpell == 0) {
            if (started == 1) {
                this->usedSpell = spellType;
            }
        }

        if (started == 0 && this->usedSpell != 0) {
            this->itemTimer = 10;
            gSaveContext.magicState = MAGIC_STATE_RESET;

            switch (this->usedSpell) {
                case 1:
                    GET_PLAYER(play)->ivanDamageMultiplier = 1;
                    break;
            }

            this->usedSpell = 0;
        }

        if (started == 2) {
            if (this->usedSpell != 0) {
                switch (this->usedSpell) {
                    case 1: // Din's — double damage on Ivan-shared hits (z_collision_check.c)
                        GET_PLAYER(play)->ivanDamageMultiplier = 2;
                        break;
                    case 2: // Nayru's — player invincibility while held
                        GET_PLAYER(play)->invincibilityTimer = -10;
                        break;
                }

                gSaveContext.magicState = MAGIC_STATE_METER_FLASH_1;
                this->magicTimer--;
                if (this->magicTimer <= 0) {
                    gSaveContext.save.saveInfo.playerData.magic--;
                    this->magicTimer = 20;
                    if (gSaveContext.save.saveInfo.playerData.magic <= 0) {
                        gSaveContext.save.saveInfo.playerData.magic = 0;

                        this->itemTimer = 10;
                        this->usedSpell = 0;
                        gSaveContext.magicState = MAGIC_STATE_RESET;
                    }
                }
            }
        }
    }
}

static void UseItem(u8 usedItem, u8 started, Actor* thisx, PlayState* play) {
    EnPartner* this = (EnPartner*)thisx;

    if (this->usedItem != 0xFF && this->itemTimer <= 0) {
        switch (usedItem) {
            case ITEM_DEKU_STICK: // OoT ITEM_STICK
                UseDekuStick(thisx, play, started);
                break;
            case ITEM_BOMB:
                UseBombs(thisx, play, started);
                break;
            case ITEM_BOMBCHU:
                UseBombchus(thisx, play, started);
                break;
            case ITEM_DEKU_NUT: // OoT ITEM_NUT
                UseNuts(thisx, play, started);
                break;
            case ITEM_BOW:
                UseBow(thisx, play, started, 0);
                break;
            case ITEM_ARROW_FIRE:
                UseBow(thisx, play, started, 1);
                break;
            case ITEM_ARROW_ICE:
                UseBow(thisx, play, started, 2);
                break;
            case ITEM_ARROW_LIGHT:
                UseBow(thisx, play, started, 3);
                break;
            case ITEM_SLINGSHOT:
                UseSlingshot(thisx, play, started);
                break;
            case ITEM_OCARINA_FAIRY:
            case ITEM_OCARINA_OF_TIME: // OoT ITEM_OCARINA_TIME
                UseOcarina(thisx, play, started);
                break;
            case ITEM_HOOKSHOT: // OoT also ITEM_LONGSHOT — MM has a single hookshot item
                UseHookshot(thisx, play, started);
                break;
            // OoT spell items don't exist in MM; the NEI fork drives spells through the SW97
            // medallions (Fire=Din's, Shadow=Nayru's, Forest=Farore's — see sw97 mapping).
            case ITEM_MEDALLION_FIRE: // OoT ITEM_DINS_FIRE
                UseSpell(thisx, play, started, 1);
                break;
            case ITEM_MEDALLION_SHADOW: // OoT ITEM_NAYRUS_LOVE
                UseSpell(thisx, play, started, 2);
                break;
            case ITEM_MEDALLION_FOREST: // OoT ITEM_FARORES_WIND
                UseFaroresWind(this, play, started);
                break;
            case ITEM_HAMMER:
                UseHammer(thisx, play, started);
                break;
            case ITEM_BOOMERANG:
                UseBoomerang(thisx, play, started);
                break;
            case ITEM_LENS_OF_TRUTH: // OoT ITEM_LENS
                UseLens(thisx, play, started);
                break;
            case ITEM_MAGIC_BEANS: // OoT ITEM_BEAN
                UseBeans(thisx, play, started);
                break;
        }
    }

    if (started == 0) {
        this->usedItem = 0xFF;
    }
}

void EnPartner_Update(Actor* thisx, PlayState* play) {
    EnPartner* this = (EnPartner*)thisx;

    Input sControlInput = play->state.input[this->actor.params];

    f32 relX = sControlInput.cur.stick_x / 10.0f * (CVarGetInteger("gModes.MirroredWorld.Mode", 0) ? -1 : 1);
    f32 relY = sControlInput.cur.stick_y / 10.0f;

    Vec3f camForward = { GET_ACTIVE_CAM(play)->at.x - GET_ACTIVE_CAM(play)->eye.x, 0.0f,
                         GET_ACTIVE_CAM(play)->at.z - GET_ACTIVE_CAM(play)->eye.z };
    camForward = EnPartner_Vec3fNormalize(camForward);

    Vec3f camRight = { -camForward.z, 0.0f, camForward.x };

    this->actor.velocity.x = 0;
    this->actor.velocity.y = 0;
    this->actor.velocity.z = 0;

    this->actor.velocity.x += camRight.x * relX;
    this->actor.velocity.z += camRight.z * relX;
    this->actor.velocity.x += camForward.x * relY;
    this->actor.velocity.z += camForward.z * relY;

    if (this->actor.velocity.x != 0 || this->actor.velocity.z != 0) {
        // OoT Math_Atan2S(-vx, vz) — MM's Math_Atan2S has swapped (y, x) argument order.
        s16 finalDir = Math_Atan2S(this->actor.velocity.z, -this->actor.velocity.x) - 0x4000;
        Math_SmoothStepToS(&this->actor.world.rot.y, finalDir, 2, 10000, 0);
        Math_SmoothStepToS(&this->actor.shape.rot.y, finalDir, 2, 10000, 0);
    }

    if (!this->canMove) {
        relX = 0;
        relY = 0;
    }

    Math_SmoothStepToF(&this->actor.speed, sqrtf(SQ(relX) + SQ(relY)), 1.0f, 1.3f, 0.0f);

    if (this->shouldDraw == 1) {
        thisx->shape.shadowAlpha = 0xFF;
        EnPartner_SpawnSparkles(this, play, 12);
    } else {
        thisx->shape.shadowAlpha = 0;
    }

    if (CHECK_BTN_ALL(sControlInput.cur.button, BTN_A) && this->canMove) {
        Math_SmoothStepToF(&this->yVelocity, 6.0f, 1.0f, 1.5f, 0.0f);
    } else if (CHECK_BTN_ALL(sControlInput.cur.button, BTN_B) && this->canMove) {
        Math_SmoothStepToF(&this->yVelocity, -6.0f, 1.0f, 1.5f, 0.0f);
    } else {
        Math_SmoothStepToF(&this->yVelocity, 0.0f, 1.0f, 1.5f, 0.0f);
    }

    this->actor.gravity = this->yVelocity;

    if (this->canMove == 1) {
        Actor_MoveWithGravity(&this->actor); // OoT Actor_MoveXZGravity
        Actor_UpdateBgCheckInfo(play, &this->actor, 19.0f, 20.0f, 0.0f, 5);
    }

    if (this->usedSpell != 0) {
        Actor_PlaySfx_Flagged(thisx, NA_SE_PL_MAGIC_SOUL_NORMAL - SFX_FLAG);
    }

    if (!Player_InCsMode(play)) {
        // Collect drops & rupees near Ivan by teleporting them onto Link
        Actor* itemActor = play->actorCtx.actorLists[ACTORCAT_MISC].first;
        while (itemActor != NULL) {
            if (itemActor->id == ACTOR_EN_ITEM00) {
                s32 dropType = itemActor->params & 0xFF; // MM packs the collectible flag in the high bits
                // OoT list adapted to MM's Item00 set. TODO(MM): OoT also collected
                // ITEM00_BOMBCHU and ITEM00_SEEDS — no MM drop equivalents.
                if (dropType == ITEM00_RUPEE_GREEN || dropType == ITEM00_RUPEE_BLUE || dropType == ITEM00_RUPEE_RED ||
                    dropType == ITEM00_RUPEE_PURPLE || dropType == ITEM00_RUPEE_HUGE ||
                    dropType == ITEM00_RECOVERY_HEART || dropType == ITEM00_BOMBS_A || dropType == ITEM00_BOMBS_B ||
                    dropType == ITEM00_ARROWS_10 || dropType == ITEM00_ARROWS_30 || dropType == ITEM00_ARROWS_40 ||
                    dropType == ITEM00_ARROWS_50 || dropType == ITEM00_MAGIC_JAR_SMALL ||
                    dropType == ITEM00_MAGIC_JAR_BIG || dropType == ITEM00_DEKU_NUTS_1 ||
                    dropType == ITEM00_DEKU_NUTS_10 || dropType == ITEM00_DEKU_STICK) {
                    f32 distanceToObject = Actor_WorldDistXYZToActor(&this->actor, itemActor);
                    if (distanceToObject <= 20.0f) {
                        itemActor->world.pos = GET_PLAYER(play)->actor.world.pos;
                        break;
                    }
                }
            }
            itemActor = itemActor->next;
        }

        // Gold Skulltula tokens: let Ivan's touch count as the player's
        itemActor = play->actorCtx.actorLists[ACTORCAT_ITEMACTION].first;
        while (itemActor != NULL) {
            if (itemActor->id == ACTOR_EN_SI) {
                f32 distanceToObject = Actor_WorldDistXYZToActor(&this->actor, itemActor);
                if (distanceToObject <= 20.0f) {
                    EnSi* ensi = (EnSi*)itemActor;
                    ensi->collider.base.ocFlags2 = OC2_HIT_PLAYER;
                    break;
                }
            }
            itemActor = itemActor->next;
        }
    }

    if (this->itemTimer > 0) {
        this->itemTimer--;
        if (this->itemTimer <= 0) {
            this->canMove = 1;
        }
    }

    if (!Player_InCsMode(play)) {
        u8 pressed = 0;
        u8 released = 0;
        u8 current = 0;
        u8 i;

        static u16 partnerButtons[7] = { BTN_CLEFT, BTN_CDOWN, BTN_CRIGHT, BTN_DUP, BTN_DDOWN, BTN_DLEFT, BTN_DRIGHT };
        // 2S2H stores D-pad equips separately (dpadItems); map button index → dpad slot.
        static u8 partnerDpadSlots[4] = { EQUIP_SLOT_D_UP, EQUIP_SLOT_D_DOWN, EQUIP_SLOT_D_LEFT, EQUIP_SLOT_D_RIGHT };
        u8 buttonMax = 3;
        if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0) != 0) {
            buttonMax = 7;
        }

        if (this->usedItem == 0xFF && this->itemTimer <= 0) {
            for (i = 0; i < buttonMax; i++) {
                if (CHECK_BTN_ALL(sControlInput.press.button, partnerButtons[i])) {
                    // OoT: gSaveContext.equips.buttonItems[i + 1] (C-lefts then dpad).
                    this->usedItem =
                        (i < 3) ? GET_CUR_FORM_BTN_ITEM(i + 1) : DPAD_GET_CUR_FORM_BTN_ITEM(partnerDpadSlots[i - 3]);
                    this->usedItemButton = i;
                    pressed = 1;
                }
            }
        }

        if (this->usedItem != 0xFF) {
            for (i = 0; i < buttonMax; i++) {
                if (CHECK_BTN_ALL(sControlInput.cur.button, partnerButtons[i]) && this->usedItemButton == i) {
                    current = 1;
                }
            }
        }

        if (this->usedItem != 0xFF) {
            for (i = 0; i < buttonMax; i++) {
                if (CHECK_BTN_ALL(sControlInput.rel.button, partnerButtons[i]) && this->usedItemButton == i) {
                    released = 1;
                }
            }
        }

        if (pressed == 1) {
            UseItem(this->usedItem, 1, thisx, play);
        } else if (released == 1) {
            UseItem(this->usedItem, 0, thisx, play);
            this->usedItemButton = 0xFF;
        } else if (current == 1) {
            UseItem(this->usedItem, 2, thisx, play);
        }
    } else {
        UseItem(this->usedItem, 0, thisx, play);
        this->usedItem = 0xFF;
        this->itemTimer = 10;
    }

    if (CHECK_BTN_ALL(sControlInput.press.button, BTN_Z) && this->canMove) {
        Actor_PlaySfx(&this->actor, NA_SE_EV_WHITE_FAIRY_DASH); // OoT NA_SE_EV_FAIRY_DASH
    }

    if (CHECK_BTN_ALL(sControlInput.cur.button, BTN_Z) && this->canMove) {
        CenterIvanOnLink(thisx, play);
    } else if (this->canMove == 1 && this->hookshotTarget == NULL) {
        Collider_UpdateCylinder(&this->actor, &this->collider);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);
    }

    if (CVarGetInteger("gCosmetics.Ivan.IdlePrimary.Changed", 0)) {
        Color_RGB8 ivanColor1 = CVarGetColor24("gCosmetics.Ivan.IdlePrimary.Value", (Color_RGB8){ 255, 255, 255 });
        this->innerColor.r = ivanColor1.r;
        this->innerColor.g = ivanColor1.g;
        this->innerColor.b = ivanColor1.b;
    } else {
        this->innerColor.r = 255;
        this->innerColor.g = 255;
        this->innerColor.b = 255;
    }

    if (CVarGetInteger("gCosmetics.Ivan.IdleSecondary.Changed", 0)) {
        Color_RGB8 ivanColor2 = CVarGetColor24("gCosmetics.Ivan.IdleSecondary.Value", (Color_RGB8){ 0, 255, 0 });
        this->outerColor.r = ivanColor2.r;
        this->outerColor.g = ivanColor2.g;
        this->outerColor.b = ivanColor2.b;
    } else {
        this->outerColor.r = 0;
        this->outerColor.g = 255;
        this->outerColor.b = 0;
    }

    SkelAnime_Update(&this->skelAnime);

    EnPartner_UpdateLights(this, play);
}

static s32 EnPartner_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx,
                                      Gfx** gfx) {
    static Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };
    f32 scale;
    Vec3f mtxMult;
    EnPartner* this = (EnPartner*)thisx;

    // OoT gFairySkel glow limb was 8; MM's fairy skeleton glow limb is FAIRY_LIMB_6 (see EnElf).
    if (limbIndex == FAIRY_LIMB_6) {
        scale = ((Math_SinS(4096) * 0.1f) + 1.0f) * 0.012f;
        scale *= (this->actor.scale.x * 124.99999f);
        Matrix_MultVec3f(&zeroVec, &mtxMult);
        Matrix_Translate(mtxMult.x, mtxMult.y, mtxMult.z, MTXMODE_NEW);
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    }

    return false;
}

static void DrawOrb(Actor* thisx, PlayState* play, u8 color) {
    EnPartner* this = (EnPartner*)thisx;
    Vec3f pos;
    f32 sp6C = play->state.frames & 0x1F;

    pos = this->actor.world.pos;
    pos.y += 5.0f;

    pos.x -= (this->actor.scale.x * 300.0f * Math_SinS(Camera_GetCamDirYaw(GET_ACTIVE_CAM(play))) *
              Math_CosS(Camera_GetCamDirPitch(GET_ACTIVE_CAM(play))));
    pos.y -= (this->actor.scale.x * 300.0f * Math_SinS(Camera_GetCamDirPitch(GET_ACTIVE_CAM(play))));
    pos.z -= (this->actor.scale.x * 300.0f * Math_CosS(Camera_GetCamDirYaw(GET_ACTIVE_CAM(play))) *
              Math_CosS(Camera_GetCamDirPitch(GET_ACTIVE_CAM(play))));

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Xlu(play->state.gfxCtx);

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0x80, 255, 255, 255, 255);

    switch (color) {
        case 1:
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 0, 0, 255);
            break;
        case 2:
            gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 255, 255);
            break;
        case 3:
            gDPSetEnvColor(POLY_XLU_DISP++, 0, 255, 0, 255);
            break;
    }

    Matrix_Translate(pos.x, pos.y, pos.z, MTXMODE_NEW);
    Matrix_Scale(this->actor.scale.x * 3.0f, this->actor.scale.y * 3.0f, this->actor.scale.z * 3.0f, MTXMODE_APPLY);
    Matrix_Mult(&play->billboardMtxF, MTXMODE_APPLY);
    Matrix_Push();
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx); // OoT gSPMatrix(MATRIX_NEWMTX)
    Matrix_RotateZF(sp6C * (M_PI / 32), MTXMODE_APPLY);
    gSPDisplayList(POLY_XLU_DISP++, gEffFlash1DL);
    Matrix_Pop();
    Matrix_RotateZF(-sp6C * (M_PI / 32), MTXMODE_APPLY);
    MATRIX_FINALIZE_AND_LOAD(POLY_XLU_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_XLU_DISP++, gEffFlash1DL);

    CLOSE_DISPS(play->state.gfxCtx);
}

void EnPartner_Draw(Actor* thisx, PlayState* play) {
    f32 alphaScale;
    s32 envAlpha;
    EnPartner* this = (EnPartner*)thisx;
    Gfx* dListHead;

    if (play->pauseCtx.state != PAUSE_STATE_OFF && this->usedItem != 0xFF) {
        UseItem(this->usedItem, 0, thisx, play);
        this->usedItem = 0xFF;
    }

    if (this->shouldDraw == 0) {
        return;
    }

    dListHead = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Gfx) * 4);

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL27_Xlu(play->state.gfxCtx);

    envAlpha = (50) & 0x1FF;
    envAlpha = (envAlpha > 255) ? 511 - envAlpha : envAlpha;

    alphaScale = 1.0f;

    gSPSegment(POLY_XLU_DISP++, 0x08, dListHead);
    gDPPipeSync(dListHead++);
    gDPSetPrimColor(dListHead++, 0, 0x01, (u8)this->innerColor.r, (u8)this->innerColor.g, (u8)this->innerColor.b,
                    (u8)(this->innerColor.a * alphaScale));

    gDPSetRenderMode(dListHead++, G_RM_PASS, G_RM_ZB_CLD_SURF2);

    gSPEndDisplayList(dListHead++);
    gDPSetEnvColor(POLY_XLU_DISP++, (u8)this->outerColor.r, (u8)this->outerColor.g, (u8)this->outerColor.b,
                   (u8)(envAlpha * alphaScale));
    POLY_XLU_DISP = SkelAnime_Draw(play, this->skelAnime.skeleton, this->skelAnime.jointTable,
                                   EnPartner_OverrideLimbDraw, NULL, &this->actor, POLY_XLU_DISP);

    CLOSE_DISPS(play->state.gfxCtx);

    if (this->usedSpell > 0) {
        DrawOrb(thisx, play, this->usedSpell);
    }
}

// ============================================================
// Actor profile — bound by the ACTOR_EN_PARTNER row in
// mm/include/tables/actor_table.h (DEFINE_ACTOR name → En_Partner_Profile).
// Same registration pattern as the Sw97_Magic* profiles at the end of
// sw97_router.c. Flags mirror SoH's ActorDB entry using MM's names.
// ============================================================
ActorProfile En_Partner_Profile = {
    /**/ ACTOR_EN_PARTNER,
    /**/ ACTORCAT_ITEMACTION,
    /**/
    (ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER |
     ACTOR_FLAG_CAN_PRESS_SWITCHES),
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnPartner),
    /**/ EnPartner_Init,
    /**/ EnPartner_Destroy,
    /**/ EnPartner_Update,
    /**/ EnPartner_Draw,
};
