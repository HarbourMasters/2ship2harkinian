/*
 * nei_link_stubs.cpp — link-time definitions for OoT/SoH symbols the ported NEI item code
 * references but 2ship (MM) doesn't provide.
 *
 * NOTE: the player/pose helpers that used to be stubbed here (Player_AnimPlayLoop,
 * Player_ZeroSpeedXZ, Player_SetIntangibility, Player_UnsetMask, func_8083485C,
 * func_80853080, AnimationContext_SetLoadFrame, ResourceMgr_LoadPlayerAnimAsHeader,
 * gPlayerLimbToBodyPart) are now REAL MM implementations in
 * mods/items/helpers/nei_player_shims.c (z_player.c TU). The vanilla item-action table is
 * MM's own sItemItemActions (extended_player.c now references it directly).
 *
 * What remains here are deep OoT internals with no MM equivalent — all called in STATEMENT
 * context (verified), so `int f(){return 0;}` is safe regardless of the caller's assumed
 * return type (0 == false == NULL). Behaviour degrades for that specific feature (logged in
 * OOT_ADAPT_TODO.md); everything else runs.
 *
 * extern "C" gives C linkage; parameter lists are intentionally empty — the linker matches
 * by name only. Lives under mods/*.cpp so the CONFIGURE_DEPENDS glob auto-compiles it.
 */
struct MessageTableEntry; // forward decl — only the pointer type is needed

extern "C" {

/* ── OoT z_player.c internals (by address) — no MM equivalent. Purpose from the port's
 *    own call-site comments. No-op. ──────────────────────────────────────────────────── */
// func_8002836C (OoT dust/soft-sprite spawn) is now a REAL effect via a function-like macro in
// nei_oot_compat.h → EffectSsDust_Spawn. Removing the stub makes Deku Leaf / Gust Jar wind + Ivan
// sparkle actually render. (The macro only affects the C item TU; this .cpp doesn't include it.)
int func_8002829C() {
    return 0;
} // effect spawn
int func_80033480() {
    return 0;
} // spark/dust ring spawn
int func_8083821C() {
    return 0;
} // set Link's body on fire
int func_8002F6D4() {
    return 0;
} // apply knockback
// func_80837948 (OoT "setup melee-weapon attack") → forward to MM's REAL equivalent func_80833864
// (z_player.c:6088 — func_8083375C damage/cylinder + Player_SetAction(Player_Action_84) + sets
// meleeWeaponAnimation). Without this the custom rods' charged SPIN attack never triggered Link's
// spin animation, so the spin-fire cylinder never activated. Args pass through as void*/int — the C
// ABI makes PlayState*/Player*/s32 compatible and extern "C" matches func_80833864 by name. NEI
void func_80833864(void* play, void* thisx, int meleeWeaponAnim);
void func_80837948(void* play, void* thisx, int meleeWeaponAnim) {
    func_80833864(play, thisx, meleeWeaponAnim);
}
int func_80837C0C() {
    return 0;
} // player hit response + freeze
int func_8005B1A4() {
    return 0;
} // camera helper
// OoT func_800AA000(f32 distSq, u16 sourceIntensity, u16 decayTimer, u16 decayStep) — controller
// rumble. Forward to MM's REAL rumble system (Rumble_Request, z_rumble.c) instead of a no-op.
// The C callers have no prototype in scope (see file header), so the arguments arrive
// default-promoted (f32 → double, u16 → int) — receive them that way, then clamp/cast to
// MM's u8 parameters. Skijer's NEI
void Rumble_Request(float distSq, unsigned char sourceIntensity, unsigned char decayTimer, unsigned char decayStep);
int func_800AA000(double distSq, int sourceIntensity, int decayTimer, int decayStep) {
    Rumble_Request((float)distSq, (unsigned char)(sourceIntensity > 255 ? 255 : sourceIntensity),
                   (unsigned char)(decayTimer > 255 ? 255 : decayTimer),
                   (unsigned char)(decayStep > 255 ? 255 : decayStep));
    return 0;
}
int func_8009728C() {
    return 0;
} // room request/load
int func_80097534() {
    return 0;
} // room finish/free
int func_80077D10() {
    return 0;
} // stick input processing
int func_80839FFC() {
    return 0;
} // player state helper

/* ── OoT-only player internals still without an MM port ── */
int Player_UpperAction_Sword() {
    return 0;
} // NeiItem updateFn for sword-like items (TODO)
// Player_StartLanternSwing / Player_Action_SwingLantern: now REAL MM implementations at the end of
// mods/items/logic/item_lantern.c (z_player.c TU) — ported from SoH on top of MM's own bottle-swing
// action, so the lantern catches fire through the same animation + catch window it does in OoT.
// Player_StartIKAxeThrow / Player_EndIKAxeThrow: now REAL MM implementations at the end of
// mods/items/logic/item_oot_boomerang.c (z_player.c TU) — the Iron Knuckle's Axe reuses OoT's
// boomerang throw pose + handsfree wait-return, exactly like OoT.

/* EffectSsKiraKira_Spawn{Small,Dispersed,Focused} are now REAL effects via function-like macros in
 * nei_oot_compat.h → EffectSsGSpk_SpawnAccel (MM's spark star). Removing these no-op stubs restores
 * sparkles globally (fire rod sparks, the whole light rod, desire sensor, hylia's grace, etc.). */

/* ── Misc engine helpers with no clean MM map (functional → TODO) ── */
int Sfx_PlaySfxCentered() {
    return 0;
}
int Magic_RequestChange() {
    return 0;
} // magic cost/restore (items free for now)
int SkelAnime_DrawSkeleton2() {
    return 0;
} // skeletal draw
int Entrance_OverrideNextIndex() {
    return 0;
} // warp override
int Scene_SetTransitionForNextEntrance() {
    return 0;
}
int Flags_GetItemGetInf() {
    return 0;
} // one-time item flags: always "not obtained"
int Flags_SetItemGetInf() {
    return 0;
}
// Randomizer_GetSceneHint stub removed: the Desire Sensor was reworked into the
// Desire Compass, whose real brain lives in 2s2h/Rando/DesireCompass.cpp.
// Picto_SyncWrite stub removed with the NEI pictobox: MM's pictograph is vanilla here, so a future
// OoT<->MM photo bridge writes gSaveContext.pictoPhotoI5 directly.
// Play_CameraSetAtEye (OoT name) → the beetle now calls MM's real Play_SetCameraAtEye directly, so
// its flying subcam actually tracks the beetle. Stub removed.
int SwitchAge() {
    return 0;
} // MM has no child/adult age

/* ── Data globals ─────────────────────────────────────────────────────────────────────── */
// Tunic colors (kokiri/goron/zora). Layout matches Color_RGB8 (3 bytes). Only [0] read now.
struct NeiRGB8 {
    unsigned char r, g, b;
};
NeiRGB8 sTunicColors[3] = { { 30, 105, 27 }, { 200, 50, 0 }, { 0, 60, 200 } };

// gEnPartnerId moved: it's now the REAL actor id (ACTOR_EN_PARTNER), defined in
// mods/actors/z_en_partner.inc.c next to the ported Ivan the Fairy actor. Skijer's NEI

// Custom OoT/SoH message-entry tables (NES/Ger/Fra/Jpn/Staff). 2ship doesn't build these and
// mm_asset_loader nulls them anyway → nullptr defaults are safe.
char* _message_0xFFFC_nes = nullptr;
MessageTableEntry* sNesMessageEntryTablePtr = nullptr;
MessageTableEntry* sGerMessageEntryTablePtr = nullptr;
MessageTableEntry* sFraMessageEntryTablePtr = nullptr;
MessageTableEntry* sJpnMessageEntryTablePtr = nullptr;
MessageTableEntry* sStaffMessageEntryTablePtr = nullptr;

} // extern "C"
