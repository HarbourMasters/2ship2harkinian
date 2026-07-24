// ── Flame Tracking + AT Collider System ─────────────────────────────────────
// Every spawned flame gets a collider that lights torches and does small damage.

#define LANTERN_FLAME_LIFETIME 40 // ~2 seconds at 20fps
#define LANTERN_MAX_FLAMES 8

typedef struct {
    Actor* actor;
    s16 timer;
    ColliderCylinder col;
    u8 colInited;
    u8 fireType;
} FlameEntry;

static FlameEntry sFlames[LANTERN_MAX_FLAMES];

static const ColliderCylinderInit sFlameColInit = {
    { COL_MATERIAL_NONE, AT_ON | AT_TYPE_PLAYER, AC_NONE, OC1_NONE, OC2_NONE, COLSHAPE_CYLINDER },
    { ELEM_MATERIAL_UNK2, { DMG_FIRE_ARROW, 0x01, 0x02 }, { 0, 0, 0 }, ATELEM_ON | ATELEM_SFX_NORMAL, ACELEM_NONE, OCELEM_NONE },
    { 30, 80, 0, { 0, 0, 0 } }
};

static void Lantern_TrackFlame(Actor* flame, u8 fireType, PlayState* play) {
    if (flame == NULL) return;
    s32 slot = -1;
    for (s32 i = 0; i < LANTERN_MAX_FLAMES; i++) {
        if (sFlames[i].timer <= 0) { slot = i; break; }
    }
    if (slot < 0) {
        slot = 0;
        if (sFlames[0].actor != NULL && sFlames[0].actor->update != NULL) {
            Actor_Kill(sFlames[0].actor);
        }
        sFlames[0].actor = NULL;
        sFlames[0].timer = 0;
        sFlames[0].colInited = 0;
    }

    sFlames[slot].actor = flame;
    sFlames[slot].timer = LANTERN_FLAME_LIFETIME;
    sFlames[slot].fireType = fireType;

    if (!sFlames[slot].colInited) {
        Collider_InitCylinder(play, &sFlames[slot].col);
        sFlames[slot].colInited = 1;
    }
    Collider_SetCylinder(play, &sFlames[slot].col, flame, &sFlameColInit);
    // All fire types: magic fire damage (lights torches, burns things)
    sFlames[slot].col.elem.atDmgInfo.dmgFlags = DMG_FIRE_ARROW;
    sFlames[slot].col.elem.atDmgInfo.damage = 2;
}

void Lantern_UpdateFlames(PlayState* play) {
    if (play == NULL) return;

    for (s32 i = 0; i < LANTERN_MAX_FLAMES; i++) {
        if (sFlames[i].timer <= 0) continue;

        if (sFlames[i].actor == NULL || sFlames[i].actor->update == NULL) {
            sFlames[i].timer = 0;
            sFlames[i].actor = NULL;
            continue;
        }

        sFlames[i].timer--;

        // Update collider position to follow flame + register AT
        if (sFlames[i].colInited) {
            sFlames[i].col.dim.pos.x = (s16)sFlames[i].actor->world.pos.x;
            sFlames[i].col.dim.pos.y = (s16)sFlames[i].actor->world.pos.y;
            sFlames[i].col.dim.pos.z = (s16)sFlames[i].actor->world.pos.z;
            CollisionCheck_SetAT(play, &play->colChkCtx, &sFlames[i].col.base);
        }

        if (sFlames[i].timer <= 0) {
            if (sFlames[i].actor->update != NULL) {
                Actor_Kill(sFlames[i].actor);
            }
            sFlames[i].actor = NULL;
        }
    }
}
