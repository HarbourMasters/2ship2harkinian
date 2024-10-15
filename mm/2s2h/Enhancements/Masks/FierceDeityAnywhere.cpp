#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "z64.h"
#include "overlays/actors/ovl_En_Bigokuta/z_en_bigokuta.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

#define HIT_BY_SWORD_BEAM 1
#define NOT_HIT_BY_SWORD_BEAM 0

/*
 * Stored to handle cases where the drawn actor is null (e.g. Leevers)
 */
static Actor* currentlyDrawnActor = nullptr;

void RegisterFierceDeityAnywhere() {
    REGISTER_VB_SHOULD(VB_DISABLE_FD_MASK, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            *should = false;
        }
    });

    REGISTER_VB_SHOULD(VB_DAMAGE_MULTIPLIER, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            int index = va_arg(args, int);
            DamageTable* damageTable = va_arg(args, DamageTable*);
            f32* damage = va_arg(args, f32*);
            f32* damageMultipliers = va_arg(args, f32*);

            /*
             * 25 is the index of the sword beam damage effect.
             */
            if (index == 25) {
                *should = false;
                /*
                 * If the existing damage multiplier for sword beams is 0, use a damage multiplier of 1.0 instead.
                 */
                u8 defaultMultiplier = damageTable->attack[index] & 0xF;
                *damage *= damageMultipliers[defaultMultiplier == 0 ? 1 : defaultMultiplier];
            }
        }
    });

    REGISTER_VB_SHOULD(VB_DAMAGE_EFFECT, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            int index = va_arg(args, int);
            DamageTable* damageTable = va_arg(args, DamageTable*);
            u32* effect = va_arg(args, u32*);
            Actor* actor = va_arg(args, Actor*);
            /*
             * 25 is the index of the sword beam damage effect.
             */
            if (index == 25) {
                *should = false;
                /*
                 * If the sword beam effect is 0, use the light arrow effect instead.
                 */
                u8 defaultEffect = (damageTable->attack[index] >> 4) & 0xF;
                if (defaultEffect == 0) {
                    // 13 is the index of the light arrow damage effect
                    *effect = (damageTable->attack[13] >> 4) & 0xF;
                } else {
                    *effect = defaultEffect;
                }
                /*
                 * shape.face is unused for any actor besides the player. We are hijacking this because we need to have
                 * some variable connected to the specific actor to indicate that the damage they received comes from a
                 * sword beam. Each stage of the pipeline (update, draw) goes through all actors in a batch.
                 */
                actor->shape.face = HIT_BY_SWORD_BEAM;
            } else if (index != 9) {
                /*
                 * 9 is the index of the sword damage effect. With how FD plays, it is possible for the sword to connect
                 * after sword beams have dealt damage. Without this check, the damage effect would revert back to the
                 * light arrows effect upon sword collision.
                 */
                actor->shape.face = NOT_HIT_BY_SWORD_BEAM;
            }
        }
    });

    /*
     * Use a hook before an actor draws to store which actor is being drawn. Some actors call Actor_DrawDamageEffects()
     * with a null actor (e.g. leevers), so we cannot just check the actor in the draw damage hook directly.
     */
    GameInteractor::Instance->RegisterGameHook<GameInteractor::ShouldActorDraw>(
        [](Actor* actor, bool* result) { currentlyDrawnActor = actor; });

    /*
     * If we're drawing the light arrow damage effect, but we know it's from a sword beam, then quietly change the type
     * to the blue lights effect.
     */
    REGISTER_VB_SHOULD(VB_DRAW_DAMAGE_EFFECT, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            if (currentlyDrawnActor->shape.face & HIT_BY_SWORD_BEAM) {
                u8* type = va_arg(args, u8*);
                if (*type == ACTOR_DRAW_DMGEFF_LIGHT_ORBS) {
                    *type = ACTOR_DRAW_DMGEFF_BLUE_LIGHT_ORBS;
                }
            }
        }
    });

    /*
     * If this is a sword beam collision, just hand wave it as a valid collision. This allows for sword beams to hit
     * enemies in a damaging way, such as Skulltulas and Big Octos.
     */
    REGISTER_VB_SHOULD(VB_CHECK_BUMPER_COLLISION, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            ColliderInfo* toucher = va_arg(args, ColliderInfo*);
            ColliderInfo* bumper = va_arg(args, ColliderInfo*);
            if (toucher->toucher.dmgFlags & DMG_SWORD_BEAM) {
                bumper->bumper.dmgFlags |= DMG_SWORD_BEAM;
                *should = false;
            }
        }
    });

    /*
     * Define a custom damage effect for sword beams for the Big Octo, which handles drawing damage effects differently
     * from most enemies. We cannot easily piggyback off of the light arrows effect like we do for everybody else.
     */
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldActorUpdate>(
        ACTOR_EN_BIGOKUTA, [](Actor* actor, bool* result) {
            if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
                EnBigokuta* enBigOkuta = (EnBigokuta*)actor;
                if (enBigOkuta->bodyCollider.base.acFlags & AC_HIT &&
                    enBigOkuta->bodyCollider.info.acHitInfo->toucher.dmgFlags & DMG_SWORD_BEAM) {
                    enBigOkuta->drawDmgEffType = ACTOR_DRAW_DMGEFF_BLUE_LIGHT_ORBS;
                    enBigOkuta->drawDmgEffScale = 1.2f;
                    enBigOkuta->drawDmgEffAlpha = 4.0f;
                    Actor_Spawn(&gPlayState->actorCtx, gPlayState, ACTOR_EN_CLEAR_TAG,
                                enBigOkuta->bodyCollider.info.bumper.hitPos.x,
                                enBigOkuta->bodyCollider.info.bumper.hitPos.y,
                                enBigOkuta->bodyCollider.info.bumper.hitPos.z, 0, 0, 0,
                                CLEAR_TAG_PARAMS(CLEAR_TAG_LARGE_LIGHT_RAYS));
                }
            }
        });
}
