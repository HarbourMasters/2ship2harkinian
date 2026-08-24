// =============================================================================
// HarpoonDummyPlayer — remote players: an invisible collider actor + a Link draw.
//
// TWO HALVES, on purpose:
//
//   ACTOR_HARPOON_PEER (this file, extern "C" block below) is one real engine
//   actor per remote client. It has NO draw — it exists to carry an AC
//   ColliderCylinder so the engine resolves OUR attacks against peers exactly
//   like it resolves them against an enemy. Sword, form attacks
//   (Player_SetCylinderForAttack), bombs, arrows and every NEI custom item that
//   registers an AT collider all land for free, with the right damage AND the
//   right element, via gHarpoonPeerDamageTable. Being a real actor also buys
//   projectedPos (3D audio + culling), colorFilter, hookshot targeting and
//   automatic cleanup on scene change / ObjectExtension_Free on delete.
//
//   HarpoonDummyPlayer_DrawAll (bottom) draws the visible body from
//   OnPlayDrawWorldEnd, as before.
//
// Why a new actor id and not ACTOR_PLAYER like SoH does: in MM, Actor_Spawn
// does NOT run init — it defers it to Actor_UpdateActor (z_actor.c:2960). A
// dummy spawned as ACTOR_PLAYER therefore sits in ACTORCAT_PLAYER for at least
// one frame, where GET_PLAYER can return IT instead of the real player. OoT
// runs init inline, which is why SoH gets away with it.
//
// Drawing, primary path — "recycled player": we keep a fully-valid Player
// template for every form the LOCAL player has visited this session (you always
// start Human, and each transform caches that form). To draw a peer in form F we
// copy the F template, override the networked fields (pos, rot, jointTable,
// weapon / shield / mask / boots) and draw through the real player path
// (Player_DrawImpl + Player_PostLimbDrawGameplay) — so equipment and masks
// render exactly like the engine, EVEN WHEN the peer's form differs from ours.
// The template's skeleton / ageProperties / DL-group pointers are engine statics
// that stay valid after we transform away. The matrix uses the same yOffset
// Actor_Draw applies (z_actor.c) so the model is grounded.
//
// Two things MUST be fixed up on that copy (see SanitizePeerDraw): the copied
// colliders still point at the LOCAL Player, and so do the sword-trail effect
// indices — both are engine-global state that peers would otherwise stomp.
//
// Fallback (a form we've never been in): resolve that form's skeleton through
// the engine ResourceMgr (form skeletons are o2r resources, not raw segmented
// pointers) and draw the body with Player_DrawImpl (no postLimbDraw, per-form
// rootAnimScale grounding). Only the body shows. Rare in practice — Human is
// always visited and MM forms carry no OoT-style equipment to miss.
//
// Each remote uses a stable per-client struct so Fast3D frame interpolation
// smooths motion between 30Hz packets.
// =============================================================================

#include "Harpoon.h"
#include "Combat/CombatSync.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "2s2h/ObjectExtension/ObjectExtension.h"
#include <spdlog/spdlog.h>
#include <map>
#include <vector>
#include <cstring>

extern "C" {
#include "z64.h"
#include "z64actor.h"
#include "z64player.h"
#include "z64scene.h"
#include "z64effect.h"
#include "sys_matrix.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
#include "main.h"
#include "mods/items/custom_items.h" // CustomItemVisualSync, Apply, OverrideDraw
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
int ResourceMgr_OTRSigCheck(char* imgData);
SkeletonHeader* ResourceMgr_LoadSkeletonByName(const char* path, SkelAnime* skelAnime);
// Same EffectBlure init params the local player uses for its sword trail
// (z_player.c:11793) — reused so peer trails look identical.
extern EffectBlureInit2 D_8085D30C;
}

// Effect_GetByIndex returns NULL for this index (z_effect.c:90), and Effect_Add
// writes exactly this value when it fails. Used as "this peer has no trail".
// = SPARK_COUNT(3) + BLURE_COUNT(25) + SHIELD_PARTICLE_COUNT(3) + TIRE_MARK_COUNT(15).
static constexpr s32 kNoEffectIndex = 46;

static std::map<uint32_t, Player> sDummyPlayers; // recycled players (per remote)
static std::map<uint32_t, Actor> sCrossActors;   // cross-form fallback actors

// A fully-valid Player snapshot for every form the LOCAL player has visited this
// session. You always start as Human (so the Human template is always available)
// and each transform caches that form's template. This lets us draw a peer in a
// DIFFERENT form than ours through the real player path (equipment + mask via
// Player_PostLimbDrawGameplay) — the skeleton/ageProperties/DL-group pointers it
// holds are engine statics that stay valid after we transform away.
static Player sFormTemplate[PLAYER_FORM_MAX];
static bool sFormTemplateValid[PLAYER_FORM_MAX] = { false };

// Per-form root-bone scale that grounds a foreign-form skeleton (values copied
// 1:1 from SoH's HarpoonDummyPlayer rootAnimScale / mm_player_form sFormProps).
// Index by PlayerTransformation: FD, Goron, Zora, Deku, Human.
static const f32 sFormRootAnimScale[PLAYER_FORM_MAX] = { 1.5f, 0.74f, 1.0f, 0.3f, 1.0f };
static s32 sCurrentDrawForm = PLAYER_FORM_HUMAN; // set before each cross-form draw

// Cross-form root grounding: scale the root limb translation by the drawn form's
// rootAnimScale (matches SoH's RemoteMmForm_OverrideLimbDraw). Without this the
// foreign-form skeleton floats.
static s32 HarpoonCrossOverride(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* actor) {
    if (limbIndex == PLAYER_LIMB_ROOT && sCurrentDrawForm >= 0 && sCurrentDrawForm < PLAYER_FORM_MAX) {
        f32 s = sFormRootAnimScale[sCurrentDrawForm];
        pos->x *= s;
        pos->y *= s;
        pos->z *= s;
    }
    return false;
}

// Per-form skeleton resolved via the engine resource manager (mirrors
// SkelAnime_InitPlayer). Pointers are stable once loaded → cache them.
static void** sFormSkel[PLAYER_FORM_MAX] = { 0 };
static s32 sFormDList[PLAYER_FORM_MAX] = { 0 };
static bool sFormTried[PLAYER_FORM_MAX] = { 0 };

static bool ResolveFormSkeleton(s32 form, void*** outSkel, s32* outDList) {
    if (!sFormTried[form]) {
        sFormTried[form] = true;
        FlexSkeletonHeader* hdrSeg = gPlayerSkeletons[form];
        if (hdrSeg != NULL) {
            if (ResourceMgr_OTRSigCheck((char*)hdrSeg) != 0) {
                hdrSeg = (FlexSkeletonHeader*)ResourceMgr_LoadSkeletonByName((const char*)hdrSeg, NULL);
            }
            if (hdrSeg != NULL) {
                FlexSkeletonHeader* hdr = (FlexSkeletonHeader*)Lib_SegmentedToVirtual(hdrSeg);
                if (hdr != NULL) {
                    sFormSkel[form] = (void**)Lib_SegmentedToVirtual(hdr->sh.segment);
                    sFormDList[form] = hdr->dListCount;
                }
            }
        }
    }
    if (sFormSkel[form] == NULL)
        return false;
    *outSkel = sFormSkel[form];
    *outDList = sFormDList[form];
    return true;
}

// =============================================================================
// ACTOR_HARPOON_PEER — the collider carrier.
// =============================================================================

// Maps an engine Actor back to the client it represents. ObjectExtension frees
// itself in Actor_Delete (z_actor.c:4198), so there is nothing to clean up.
struct HarpoonPeerClientId {
    uint32_t clientId = 0;
};
static ObjectExtension::Register<HarpoonPeerClientId> sHarpoonPeerClientIdRegister;

uint32_t HarpoonPeer_GetClientId(const Actor* actor) {
    HarpoonPeerClientId* d = ObjectExtension::GetInstance().Get<HarpoonPeerClientId>(actor);
    return d != nullptr ? d->clientId : 0;
}

void HarpoonPeer_SetClientId(const Actor* actor, uint32_t clientId) {
    ObjectExtension::GetInstance().Set<HarpoonPeerClientId>(actor, HarpoonPeerClientId{ clientId });
}

extern "C" {

typedef struct HarpoonPeerActor {
    /* 0x000 */ Actor actor;
    /* 0x144 */ ColliderCylinder cylinder;
    /*       */ u32 clientId;
    /*       */ s16 invincibilityTimer;
} HarpoonPeerActor;

void HarpoonPeer_Init(Actor* thisx, PlayState* play);
void HarpoonPeer_Destroy(Actor* thisx, PlayState* play);
void HarpoonPeer_Update(Actor* thisx, PlayState* play);

// How much a hit on a remote player hurts, and which reaction it triggers.
// Row order is MM's (z_collision_btltbls.c); the effect nibble carries a
// HarpoonHitResponse, which the victim's client turns into a func_80833B18
// call. Damage is in DamageTable units (quarter hearts) — the receiver scales.
//
// The elemental rows are what makes the NEI custom items work over the network
// without any per-item wiring: Fire Rod / Lantern / fire Gust Jar already emit
// DMG_FIRE_ARROW, the Ice Rod DMG_ICE_ARROW, the Light Rod DMG_LIGHT_ARROW, and
// Gust Jar absorb DMG_GORON_POUND.
DamageTable gHarpoonPeerDamageTable = { {
    /* Deku Nut       */ DMG_ENTRY(0, HARPOON_HIT_STUN),
    /* Deku Stick     */ DMG_ENTRY(1, HARPOON_HIT_NORMAL),
    /* Horse trample  */ DMG_ENTRY(1, HARPOON_HIT_KNOCKBACK_LARGE),
    /* Explosives     */ DMG_ENTRY(2, HARPOON_HIT_KNOCKBACK_LARGE),
    /* Zora boomerang */ DMG_ENTRY(0, HARPOON_HIT_STUN),
    /* Normal arrow   */ DMG_ENTRY(1, HARPOON_HIT_NORMAL),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, HARPOON_HIT_NONE),
    /* Hookshot       */ DMG_ENTRY(1, HARPOON_HIT_STUN),
    /* Goron punch    */ DMG_ENTRY(2, HARPOON_HIT_KNOCKBACK_LARGE),
    /* Sword          */ DMG_ENTRY(2, HARPOON_HIT_NORMAL),
    /* Goron pound    */ DMG_ENTRY(4, HARPOON_HIT_KNOCKBACK_LARGE),
    /* Fire arrow     */ DMG_ENTRY(2, HARPOON_HIT_FIRE),
    /* Ice arrow      */ DMG_ENTRY(0, HARPOON_HIT_FROZEN),
    /* Light arrow    */ DMG_ENTRY(4, HARPOON_HIT_LIGHT),
    /* Goron spikes   */ DMG_ENTRY(2, HARPOON_HIT_KNOCKBACK_SMALL),
    /* Deku spin      */ DMG_ENTRY(1, HARPOON_HIT_NORMAL),
    /* Deku bubble    */ DMG_ENTRY(1, HARPOON_HIT_NORMAL),
    /* Deku launch    */ DMG_ENTRY(2, HARPOON_HIT_KNOCKBACK_LARGE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, HARPOON_HIT_NONE),
    /* Zora barrier   */ DMG_ENTRY(2, HARPOON_HIT_ELECTRIFIED),
    /* Normal shield  */ DMG_ENTRY(0, HARPOON_HIT_NONE),
    /* Light ray      */ DMG_ENTRY(0, HARPOON_HIT_LIGHT),
    /* Thrown object  */ DMG_ENTRY(1, HARPOON_HIT_KNOCKBACK_SMALL),
    /* Zora punch     */ DMG_ENTRY(2, HARPOON_HIT_NORMAL),
    /* Spin attack    */ DMG_ENTRY(2, HARPOON_HIT_NORMAL),
    /* Sword beam     */ DMG_ENTRY(4, HARPOON_HIT_NORMAL),
    /* Normal Roll    */ DMG_ENTRY(1, HARPOON_HIT_KNOCKBACK_SMALL),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, HARPOON_HIT_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, HARPOON_HIT_NONE),
    /* Unblockable    */ DMG_ENTRY(4, HARPOON_HIT_NORMAL),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, HARPOON_HIT_WIND_BLOW),
    /* Powder Keg     */ DMG_ENTRY(8, HARPOON_HIT_KNOCKBACK_LARGE),
} };

// AC_TYPE_PLAYER is the half that matters: CollisionCheck_AC pairs colliders on
// (acFlags & atFlags & AC_TYPE_ALL), and every player attack is AT_TYPE_PLAYER.
// OC2_TYPE_1 rather than OC2_TYPE_PLAYER so peers don't shove blocks and bushes
// around on OUR machine (same reasoning as SoH); OC1_ON with no type bits means
// the peer pushes nothing, but the local player (OC1_TYPE_ALL) still bumps into
// them.
static ColliderCylinderInit sHarpoonPeerColliderInit = {
    {
        COL_MATERIAL_HIT5,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK1,
        { 0x00000000, 0x00, 0x00 },
        { 0xF7CFFFFF, 0x00, 0x00 },
        ATELEM_NONE | ATELEM_SFX_NORMAL,
        ACELEM_ON | ACELEM_HOOKABLE | ACELEM_NO_HITMARK,
        OCELEM_ON,
    },
    { 30, 60, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sHarpoonPeerCcInfoInit = { 0, 30, 60, 0, 50 };

ActorProfile HarpoonPeer_Profile = {
    ACTOR_HARPOON_PEER,
    ACTORCAT_NPC,
    ACTOR_FLAG_UPDATE_CULLING_DISABLED | ACTOR_FLAG_DRAW_CULLING_DISABLED | ACTOR_FLAG_ATTENTION_ENABLED |
        ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER,
    GAMEPLAY_KEEP,
    sizeof(HarpoonPeerActor),
    HarpoonPeer_Init,
    HarpoonPeer_Destroy,
    HarpoonPeer_Update,
    NULL, // no draw — HarpoonDummyPlayer_DrawAll renders the body
    NULL,
};

void HarpoonPeer_Init(Actor* thisx, PlayState* play) {
    HarpoonPeerActor* self = (HarpoonPeerActor*)thisx;

    Collider_InitAndSetCylinder(play, &self->cylinder, &self->actor, &sHarpoonPeerColliderInit);
    CollisionCheck_SetInfo2(&self->actor.colChkInfo, &gHarpoonPeerDamageTable, &sHarpoonPeerCcInfoInit);
    self->actor.colChkInfo.mass = 50;
    // room -1 or Actor_UpdateAll kills us the moment the player walks into a
    // different room of the same scene (z_actor.c:3783).
    self->actor.room = -1;
    self->clientId = HarpoonPeer_GetClientId(&self->actor);
    self->invincibilityTimer = 0;
}

void HarpoonPeer_Destroy(Actor* thisx, PlayState* play) {
    HarpoonPeerActor* self = (HarpoonPeerActor*)thisx;
    Collider_DestroyCylinder(play, &self->cylinder);
}

void HarpoonPeer_Update(Actor* thisx, PlayState* play) {
    HarpoonPeerActor* self = (HarpoonPeerActor*)thisx;
    Harpoon* h = Harpoon::Instance();

    if (self->clientId == 0) {
        self->clientId = HarpoonPeer_GetClientId(&self->actor);
    }
    if (self->clientId == 0 || h->State() != HarpoonConnState::InRoom) {
        Actor_Kill(&self->actor);
        return;
    }

    // Snapshot the networked state under the lock, then drop it before touching
    // the engine. Nothing engine-side may be called while holding it — note that
    // even Actor_Kill runs GameInteractor hooks, one of which takes this very
    // mutex, so the "client vanished" kill happens after the scope closes.
    f32 px = 0.0f, py = 0.0f, pz = 0.0f;
    s16 rotY = 0;
    s16 peerScene = -1;
    bool gone = false;
    {
        std::lock_guard<std::mutex> lk(h->StateMutex());
        auto it = h->ClientsRaw().find(self->clientId);
        if (it == h->ClientsRaw().end()) {
            gone = true;
        } else {
            const HarpoonClient& c = it->second;
            px = c.posX;
            py = c.posY;
            pz = c.posZ;
            rotY = c.rotY;
            peerScene = c.sceneId;
        }
    }
    if (gone) {
        Actor_Kill(&self->actor);
        return;
    }

    if (peerScene != play->sceneId) {
        // Elsewhere in the world: stay alive (they may come back) but register
        // nothing, so we can't be hit and don't block anything.
        return;
    }

    // Read AC_HIT BEFORE CollisionCheck_SetAC — SetAC runs the shape's reset
    // function first (z_collision_check.c:1272) and would clear the flag.
    if ((self->cylinder.base.acFlags & AC_HIT) && self->invincibilityTimer == 0) {
        u8 effect = self->actor.colChkInfo.damageEffect;
        u8 damage = self->actor.colChkInfo.damage;
        if (damage != 0 || effect != HARPOON_HIT_NONE) {
            HarpoonCombat_SendDamage(self->clientId, effect, damage);
            self->invincibilityTimer = 20;
            Actor_SetColorFilter(&self->actor, 0x4000, 0xFF, 0, 12);
        }
    }
    CollisionCheck_ResetDamage(&self->actor.colChkInfo);

    if (self->invincibilityTimer > 0) {
        self->invincibilityTimer--;
    }

    self->actor.world.pos.x = px;
    self->actor.world.pos.y = py;
    self->actor.world.pos.z = pz;
    self->actor.shape.rot.y = rotY;
    self->actor.world.rot.y = rotY;
    self->actor.room = -1;

    // Tatl perches on anything attention-enabled that isn't hostile; in coop
    // that's just noise, and in geoguessr it would give peers away.
    if (h->IsPvpActive()) {
        self->actor.flags &= ~ACTOR_FLAG_LOCK_ON_DISABLED;
    } else {
        self->actor.flags |= ACTOR_FLAG_LOCK_ON_DISABLED;
    }

    Collider_UpdateCylinder(&self->actor, &self->cylinder);
    CollisionCheck_SetOC(play, &play->colChkCtx, &self->cylinder.base);
    if (self->invincibilityTimer <= 0) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &self->cylinder.base);
    }
}

} // extern "C"

// -----------------------------------------------------------------------------
// Peer actor lifecycle: spawn one per remote client, kill the leftovers.
// -----------------------------------------------------------------------------

static std::map<uint32_t, Actor*> sPeerActors;

// Game-thread mirror of who owns which EffectBlure slot (see the trail section
// below). HarpoonClient entries are dropped by the network thread when someone
// leaves the room, which would otherwise strand their two slots until the next
// scene change — this is what lets us hand them back.
static std::map<uint32_t, std::pair<s32, s32>> sPeerFxSlots;

Actor* HarpoonPeer_FindByClientId(uint32_t clientId) {
    auto it = sPeerActors.find(clientId);
    return it != sPeerActors.end() ? it->second : nullptr;
}

// Called on scene teardown: every actor is gone and every effect slot has been
// recycled, so drop all our handles rather than let them dangle.
void HarpoonPeer_OnPlayDestroy() {
    sPeerActors.clear();
    sPeerFxSlots.clear(); // Effect_DestroyAll already recycled the pool
    auto* h = Harpoon::Instance();
    std::lock_guard<std::mutex> lk(h->StateMutex());
    for (auto& [cid, c] : h->ClientsRaw()) {
        c.peerActor = nullptr;
        c.fxBlure[0] = -1;
        c.fxBlure[1] = -1;
    }
}

void HarpoonPeer_OnActorDestroyed(Actor* actor) {
    if (actor == nullptr || actor->id != ACTOR_HARPOON_PEER)
        return;
    uint32_t cid = HarpoonPeer_GetClientId(actor);
    sPeerActors.erase(cid);
    auto* h = Harpoon::Instance();
    std::lock_guard<std::mutex> lk(h->StateMutex());
    auto it = h->ClientsRaw().find(cid);
    if (it != h->ClientsRaw().end() && it->second.peerActor == actor) {
        it->second.peerActor = nullptr;
    }
}

void HarpoonPeer_RefreshActors(PlayState* play) {
    if (play == nullptr)
        return;
    auto* h = Harpoon::Instance();
    if (h->State() != HarpoonConnState::InRoom) {
        sPeerActors.clear();
        return;
    }

    // Collect what we need under the lock, spawn outside it.
    struct Pending {
        uint32_t cid;
        f32 x, y, z;
        s16 rotY;
    };
    std::vector<Pending> toSpawn;
    {
        std::lock_guard<std::mutex> lk(h->StateMutex());
        for (auto& [cid, c] : h->ClientsRaw()) {
            if (c.sceneId != play->sceneId)
                continue;
            if (c.peerActor != nullptr)
                continue;
            // Spawning at the origin drops the actor outside the world; wait
            // until a real position packet has landed.
            if (c.posX == 0.0f && c.posY == 0.0f && c.posZ == 0.0f)
                continue;
            toSpawn.push_back({ cid, c.posX, c.posY, c.posZ, c.rotY });
        }
    }

    for (const Pending& p : toSpawn) {
        Actor* actor = Actor_Spawn(&play->actorCtx, play, ACTOR_HARPOON_PEER, p.x, p.y, p.z, 0, p.rotY, 0, 0);
        if (actor == nullptr)
            continue;
        // MM's Actor_Spawn returns before init runs, so the id is set here and
        // read again in Init (and defensively on the first Update).
        HarpoonPeer_SetClientId(actor, p.cid);
        sPeerActors[p.cid] = actor;
        std::lock_guard<std::mutex> lk(h->StateMutex());
        auto it = h->ClientsRaw().find(p.cid);
        if (it != h->ClientsRaw().end()) {
            it->second.peerActor = actor;
        }
    }
}

// -----------------------------------------------------------------------------
// Per-peer sword-trail slots.
//
// The bug this fixes: sFormTemplate is a whole-Player copy, so every peer
// inherited the LOCAL player's meleeWeaponEffectIndex. EffectBlure_AddVertex is
// a flat append with no notion of an owner and the draw always stitches
// elem[i]<->elem[i+1], so two players swinging at once produced one ribbon
// running from one sword to the other. Each peer gets its own slot out of the
// 25-entry blure pool (the local player uses 2, so ~11 peers with two trails).
//
// Nothing else is needed to make them show: Effect_UpdateAll / Effect_DrawAll
// walk all 25 slots on their own, so peer trails animate and draw for free.
//
// Colours come from D_8085D30C, which carries the same values as the local
// player's colorType 0. Known cosmetic gap: a peer's charged spin attack won't
// get the bluish colorType-1 tint, and Deku form won't get its shorter element
// duration — Player_OverrideBlureColors dereferences its two blures without a
// NULL check, so calling it for peers whose Effect_Add failed would crash.
// -----------------------------------------------------------------------------

static void HarpoonFx_Acquire(PlayState* play, HarpoonClient& c, uint32_t cid) {
    if (c.fxBlure[0] >= 0)
        return;
    // On failure Effect_Add writes TOTAL_EFFECT_COUNT, which Effect_GetByIndex
    // maps to NULL and EffectBlure_AddVertex ignores — that peer just gets no
    // trail. No special-casing needed.
    Effect_Add(play, &c.fxBlure[0], EFFECT_BLURE2, 0, 0, &D_8085D30C);
    Effect_Add(play, &c.fxBlure[1], EFFECT_BLURE2, 0, 0, &D_8085D30C);
    sPeerFxSlots[cid] = { c.fxBlure[0], c.fxBlure[1] };
}

// Give back the slots of anyone who has left. Caller must hold StateMutex (it
// reads the client map) — Effect_Destroy only touches the effect pool, no hooks.
static void HarpoonFx_ReleaseDeparted(PlayState* play, Harpoon* h) {
    for (auto it = sPeerFxSlots.begin(); it != sPeerFxSlots.end();) {
        if (h->ClientsRaw().count(it->first) != 0) {
            ++it;
            continue;
        }
        if (it->second.first >= 0)
            Effect_Destroy(play, it->second.first);
        if (it->second.second >= 0)
            Effect_Destroy(play, it->second.second);
        it = sPeerFxSlots.erase(it);
    }
}

// -----------------------------------------------------------------------------
// Fix up a Player struct copied from the form template before drawing it.
//
// Two classes of engine-global state ride along in that memcpy and must not be
// left pointing at the local player:
//
//  1. Colliders. Collider_InitAndSetQuad stored &localPlayer->actor in
//     .base.actor, so Player_PostLimbDrawGameplay -> func_80126440 would call
//     CollisionCheck_SetAT with a quad whose owner is US: SetATvsAC then writes
//     atHitEffect into the LOCAL player's colChkInfo and the anti-self-hit test
//     thinks we are the attacker. Same for the shield (SetAC *and* SetAT).
//     Re-point them at the peer actor and switch them off; each quad also costs
//     a slot out of colAT[50]/colAC[60].
//     The trail itself is unaffected — that call passes a NULL collider.
//
//  2. Sword-trail effect indices (see above).
// -----------------------------------------------------------------------------
static void SanitizePeerDraw(Player& dp, HarpoonClient& c, Actor* peerActor) {
    Actor* owner = peerActor != nullptr ? peerActor : &dp.actor;
    dp.cylinder.base.actor = owner;
    dp.shieldCylinder.base.actor = owner;
    dp.shieldQuad.base.actor = owner;
    dp.meleeWeaponQuads[0].base.actor = owner;
    dp.meleeWeaponQuads[1].base.actor = owner;
    dp.meleeWeaponQuads[0].base.atFlags = AT_NONE;
    dp.meleeWeaponQuads[1].base.atFlags = AT_NONE;
    dp.cylinder.base.atFlags = AT_NONE;
    dp.cylinder.base.acFlags = AC_NONE;
    dp.shieldQuad.base.atFlags = AT_NONE;
    dp.shieldQuad.base.acFlags = AC_NONE;
    // Gates the shield collider registration entirely (z_player_lib.c:3303).
    dp.stateFlags1 &= ~PLAYER_STATE1_400000;

    dp.meleeWeaponEffectIndex[0] = c.fxBlure[0] >= 0 ? c.fxBlure[0] : kNoEffectIndex;
    dp.meleeWeaponEffectIndex[1] = c.fxBlure[1] >= 0 ? c.fxBlure[1] : kNoEffectIndex;
    dp.meleeWeaponEffectIndex[2] = kNoEffectIndex; // tire mark: not wanted on peers

    // The template's hand/sheath display lists belong to whatever WE are holding.
    // Recompute them from the peer's own heldItemAction, or e.g. a peer with a
    // Fire Rod renders the Biggoron Sword in the same hand (the rods use
    // PLAYER_MODELGROUP_BGS and Player_SetModels has the fist-override for them).
    Player_SetModels(&dp, Player_ActionToModelGroup(&dp, (PlayerItemAction)dp.heldItemAction));
}

// Draw whatever NEI custom item this peer is holding / using.
//
// gCustomItemState is a singleton, so it is saved and restored around every
// peer — otherwise applying their state would leave the LOCAL player's rods and
// Gust Jar stuck in whatever the last remote was doing.
//
// Order matters: the per-item draws build their matrix from
// player->bodyPartsPos[PLAYER_BODYPART_L_HAND] (not from the limb matrix), and
// those are filled in by Player_PostLimbDrawGameplay during Player_DrawImpl — so
// this has to run AFTER the body draw.
static void DrawPeerCustomItems(PlayState* play, Player& dp, const HarpoonClient& c) {
    if (c.customItemBlob.size() != sizeof(CustomItemVisualSync))
        return;

    CustomItemVisualSync sync;
    memcpy(&sync, c.customItemBlob.data(), sizeof(sync));

    CustomItemState saved = gCustomItemState;
    CustomItems_ApplyVisualSync(&sync);
    CustomItems_OverrideDraw(&dp, play);
    gCustomItemState = saved;
}

void HarpoonDummyPlayer_DrawAll(PlayState* play) {
    if (!play)
        return;
    auto* h = Harpoon::Instance();
    if (h->State() != HarpoonConnState::InRoom)
        return;

    Player* player = GET_PLAYER(play);
    if (player == NULL || player->skelAnime.skeleton == NULL)
        return;

    // While a (de)transformation is in progress, the target form
    // (gSaveContext.save.playerForm, read by FastTransformation to load the new
    // object) is already the NEW form while player->transformation is still the
    // OLD one until re-init. They differ ONLY during the transition window — the
    // exact frames where the form object/skelAnime are inconsistent and drawing
    // crashes the GPU. Skip those frames (this also covers the pre-`transformation`
    // frames a form-change check misses).
    if (GET_PLAYER_FORM != player->transformation)
        return;

    s16 myScene = play->sceneId;
    s32 myForm = player->transformation;
    if (myForm < 0 || myForm >= PLAYER_FORM_MAX)
        return;
    if (player->actor.objectSlot < 0 || !Object_IsLoaded(&play->objectCtx, player->actor.objectSlot)) {
        return;
    }

    // Extra settle window after the transformation finishes (skelAnime/object
    // may take a couple more frames to fully re-init).
    static s32 sLastForm = -1;
    static int sSettleFrames = 0;
    if (myForm != sLastForm) {
        sLastForm = myForm;
        sSettleFrames = 10;
    }
    if (sSettleFrames > 0) {
        sSettleFrames--;
        return;
    }

    void* objSegment = play->objectCtx.slots[player->actor.objectSlot].segment;
    if (objSegment == NULL)
        return;
    f32 yOffset = player->actor.shape.yOffset * player->actor.scale.y;

    // Snapshot the local player as the template for this form (stable here). Peers
    // in this form — now or later, even after we transform away — draw from it
    // with full equipment.
    sFormTemplate[myForm] = *player;
    sFormTemplateValid[myForm] = true;

    std::lock_guard<std::mutex> lk(h->StateMutex());

    HarpoonFx_ReleaseDeparted(play, h);

    static int diagCounter = 0;
    bool logThisFrame = ((diagCounter++ % 120) == 0);
    int drawnSame = 0, drawnCross = 0, skipped = 0;

    for (auto& [cid, c] : h->ClientsRaw()) {
        if (c.sceneId != myScene)
            continue;
        s32 form = c.transformation;
        if (form < 0 || form >= PLAYER_FORM_MAX)
            continue;

        HarpoonFx_Acquire(play, c, cid);

        // Bind the form's object on segment 6 if it's loaded: the current form's
        // object always is, Human's OBJECT_LINK_CHILD always is, other forms only
        // when they happen to be in the pool. MM-form skeletons are self-contained
        // resources and don't read segment 6, so a missing bind is harmless there.
        void* seg06 = NULL;
        {
            s32 fslot = (form == myForm) ? player->actor.objectSlot
                                         : Object_GetSlot(&play->objectCtx, gPlayerFormObjectIds[form]);
            if (fslot >= 0 && Object_IsLoaded(&play->objectCtx, fslot)) {
                seg06 = play->objectCtx.slots[fslot].segment;
            }
        }

        if (sFormTemplateValid[form]) {
            // We've been in this form, so we have a fully-valid Player template
            // for it. Draw the peer through the real player path → full equipment
            // and mask, regardless of which form WE are currently in.
            Player& dp = sDummyPlayers[cid];
            dp = sFormTemplate[form];
            dp.actor.world.pos.x = c.posX;
            dp.actor.world.pos.y = c.posY;
            dp.actor.world.pos.z = c.posZ;
            dp.actor.shape.rot.x = c.rotX;
            dp.actor.shape.rot.y = c.rotY;
            dp.actor.shape.rot.z = c.rotZ;
            dp.skelAnime.jointTable = (Vec3s*)c.jointTable;
            dp.currentBoots = c.currentBoots;
            dp.currentShield = c.currentShield;
            dp.itemAction = c.itemAction;
            dp.heldItemAction = c.heldItemAction;
            dp.currentMask = c.currentMask;
            SanitizePeerDraw(dp, c, (Actor*)c.peerActor);
            f32 yOff = dp.actor.shape.yOffset * dp.actor.scale.y;

            OPEN_DISPS(play->state.gfxCtx);
            if (seg06 != NULL) {
                gSPSegment(POLY_OPA_DISP++, 0x06, (uintptr_t)seg06);
                gSPSegment(POLY_XLU_DISP++, 0x06, (uintptr_t)seg06);
            }
            gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)gCullBackDList);
            gSPSegment(POLY_XLU_DISP++, 0x0C, (uintptr_t)gCullBackDList);
            Matrix_SetTranslateRotateYXZ(c.posX, c.posY + yOff, c.posZ, &dp.actor.shape.rot);
            Matrix_Scale(dp.actor.scale.x, dp.actor.scale.y, dp.actor.scale.z, MTXMODE_APPLY);
            Player_DrawImpl(play, dp.skelAnime.skeleton, dp.skelAnime.jointTable, dp.skelAnime.dListCount, 0,
                            (PlayerTransformation)form, dp.currentBoots, dp.actor.shape.face,
                            Player_OverrideLimbDrawGameplayDefault, Player_PostLimbDrawGameplay, &dp.actor);
            CLOSE_DISPS(play->state.gfxCtx);
            DrawPeerCustomItems(play, dp, c);
            if (form == myForm) {
                drawnSame++;
            } else {
                drawnCross++;
            }
            continue;
        }

        // Fallback: a form we've never been in this session (no template). Draw
        // the body from the resource skeleton — no equipment, rootAnimScale
        // grounding. (Rare: Human is always visited, MM forms have no OoT-style
        // equipment to miss.)
        void** skel = NULL;
        s32 dListCount = 0;
        if (!ResolveFormSkeleton(form, &skel, &dListCount)) {
            skipped++;
            continue;
        }

        Actor& da = sCrossActors[cid];
        da.world.pos.x = c.posX;
        da.world.pos.y = c.posY;
        da.world.pos.z = c.posZ;
        Vec3s rot = { c.rotX, c.rotY, c.rotZ };

        sCurrentDrawForm = form;
        OPEN_DISPS(play->state.gfxCtx);
        gSPSegment(POLY_OPA_DISP++, 0x0C, (uintptr_t)gCullBackDList);
        gSPSegment(POLY_XLU_DISP++, 0x0C, (uintptr_t)gCullBackDList);
        if (seg06 != NULL) {
            gSPSegment(POLY_OPA_DISP++, 0x06, (uintptr_t)seg06);
            gSPSegment(POLY_XLU_DISP++, 0x06, (uintptr_t)seg06);
        }
        Matrix_SetTranslateRotateYXZ(c.posX, c.posY + yOffset, c.posZ, &rot);
        Matrix_Scale(player->actor.scale.x, player->actor.scale.y, player->actor.scale.z, MTXMODE_APPLY);
        Player_DrawImpl(play, skel, (Vec3s*)c.jointTable, dListCount, 0, (PlayerTransformation)form, 0, 0,
                        HarpoonCrossOverride, NULL, &da);
        CLOSE_DISPS(play->state.gfxCtx);
        drawnCross++;
    }

    if (logThisFrame) {
        SPDLOG_INFO("[Harpoon] DrawAll: clients={} myForm={} same={} cross={} skipped={}", (int)h->ClientsRaw().size(),
                    myForm, drawnSame, drawnCross, skipped);
    }
}

// Nametags previously anchored to a spawned actor; coop nametags are a TODO.
void HarpoonDummyPlayer_SyncNametags() {
}
