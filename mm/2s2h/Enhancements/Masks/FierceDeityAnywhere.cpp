#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "overlays/actors/ovl_En_M_Thunder/z_en_m_thunder.h"
#include "z64.h"

void RegisterFierceDeityAnywhere() {
    REGISTER_VB_SHOULD(VB_DISABLE_FD_MASK, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            *should = false;
        }
    });

    REGISTER_VB_SHOULD(GI_VB_DAMAGE_MULTIPLIER, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            DamageAndEffectHookInfo damageInfo = *((DamageAndEffectHookInfo*)(opt));
            // 25 is the index of the sword beam damage effect
            if (damageInfo.index == 25) {
                *should = false;
                /*
                 * If the existing damage multiplier for sword beams is 0, use a damage multiplier of 1.0 instead.
                 */
                u8 defaultMultiplier = damageInfo.damageTable->attack[damageInfo.index] & 0xF;
                (*damageInfo.damage) *= damageInfo.multipliers[defaultMultiplier == 0 ? 1 : defaultMultiplier];
            }
        }
    });

    REGISTER_VB_SHOULD(GI_VB_DAMAGE_EFFECT, {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            DamageAndEffectHookInfo damageInfo = *((DamageAndEffectHookInfo*)(opt));
            // 25 is the index of the sword beam damage effect
            if (damageInfo.index == 25) {
                *should = false;
                /*
                 * If the default effect is 0, prefer the light arrow effect.
                 * If the light arrow effect is also 0, prefer the basic sword effect.
                 */
                u8 defaultEffect = (damageInfo.damageTable->attack[damageInfo.index] >> 4) & 0xF;
                if (defaultEffect == 0) {
                    // 13 is the index of the light arrow damage effect
                    u8 lightArrowEffect = (damageInfo.damageTable->attack[13] >> 4) & 0xF;
                    // 9 is the index of the sword damage effect
                    u8 swordEffect = (damageInfo.damageTable->attack[9] >> 4) & 0xF;
                    (*damageInfo.effect) = lightArrowEffect == 0 ? swordEffect : lightArrowEffect;
                } else {
                    (*damageInfo.effect) = defaultEffect;
                }
            }
        }
    });

    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(ACTOR_EN_M_THUNDER, [](Actor* actor) {
        if (CVarGetInteger("gEnhancements.Masks.FierceDeitysAnywhere", 0)) {
            /*
             * This additional flag makes sword beams effective against enemies that have special collision processing,
             * such as Big Octos and Skulltulas.
             */
            EnMThunder* enMThunder = (EnMThunder*)actor;
            enMThunder->collider.info.toucher.dmgFlags |= DMG_LIGHT_ARROW;
        }
    });
}
