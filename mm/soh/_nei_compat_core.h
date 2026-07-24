/*
 * _nei_compat_core.h — central soh→2s2h compatibility shim for the NEI port.
 *
 * The copied NEI (Ocarina of Time / Ship of Harkinian) code uses SoH-isms that
 * 2ship spells differently. This header provides drop-in equivalents so the
 * ported sources compile unchanged. It is pulled in by every `mm/soh/*` redirect
 * shim, so any file that includes ANY `soh/...` header gets these for free.
 *
 * Every macro is guarded so it never clobbers a 2ship definition that is already
 * in scope (e.g. CVAR_COSMETIC from 2s2h/BenGui/CosmeticEditor.h).
 *
 * NOTE: this only maps SYMBOLS/SPELLING. Behavioural OoT↔MM differences (collider
 * system, item enums, damage model, …) are translated in the sources themselves.
 */
#ifndef NEI_COMPAT_CORE_H
#define NEI_COMPAT_CORE_H

/* OoT-only DMG_* damage-flag names → MM equivalents (guarded; see header). */
#include "soh/_nei_dmg_compat.h"

/* ── CVar prefix macros (SoH spells these as macros; 2ship uses raw strings) ─── */
#ifndef CVAR_ENHANCEMENT
#define CVAR_ENHANCEMENT(var) "gEnhancements." var
#endif
#ifndef CVAR_SETTING
#define CVAR_SETTING(var) "gSettings." var
#endif
#ifndef CVAR_CHEAT
#define CVAR_CHEAT(var) "gCheats." var
#endif
#ifndef CVAR_GENERAL
#define CVAR_GENERAL(var) "gGeneral." var
#endif
#ifndef CVAR_DEVELOPER
#define CVAR_DEVELOPER(var) "gDeveloperTools." var
#endif
#ifndef CVAR_REMOTE
#define CVAR_REMOTE(var) "gRemote." var
#endif
/* 2ship's cosmetic prefix is the singular "gCosmetic" (see CosmeticEditor.h). */
#ifndef CVAR_COSMETIC
#define CVAR_COSMETIC(var) "gCosmetic." var
#endif

/* ── Audio API: SoH (OoT) name → MM name. Argument lists are identical. ─────── */
/* AudioSfx_PlaySfx(sfxId, pos, token, freqScale, volume, reverbAdd) — sfx.h     */
#ifndef Audio_PlaySoundGeneral
#define Audio_PlaySoundGeneral AudioSfx_PlaySfx
#endif
/* Actor_PlaySfx(actor, sfxId) — z64actor.h */
#ifndef Audio_PlayActorSound2
#define Audio_PlayActorSound2 Actor_PlaySfx
#endif
#ifndef Audio_QueueSeqCmd
#define Audio_QueueSeqCmd AudioSeq_QueueSeqCmd
#endif

#endif /* NEI_COMPAT_CORE_H */
