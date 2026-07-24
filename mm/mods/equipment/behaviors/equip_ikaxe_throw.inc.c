// ---------------------------------------------------------------------------
// Tomahawk Throw — first-person aim + equipped-C launch, exactly like the magic rods.
//
// Flow (mirrors FireRod_UpdateFirstPerson in item_rod_fire.c):
//   1. While the hammer/axe is held, C-Up enters first-person aim (the aim camera + pose).
//   2. In aim, pressing the EQUIPPED C-button (the hammer's own button) LAUNCHES the axe in the
//      aimed direction — not B. ItemInput_Update gives us that button + its press, like the rods.
//   3. The axe flies 1:1 like the boomerang: spawned with the aim yaw/pitch and moveTo = NULL so
//      EnBoom_Fly carries it straight in the AIMED direction (Actor_SetProjectileSpeed uses
//      world.rot), then returns to Link when returnTimer hits 0. It draws the axe DL and deals
//      hammer damage (En_Boom params 99).
//   4. C-Up toggles aim off; any other action button / damage / cutscene cancels it.
// ---------------------------------------------------------------------------

// Enter the OoT boomerang's AIM MODE for the axe (same stance + camera as the boomerang, but the
// axe stays in hand). USING_ZORA_BOOMERANG + unk_ACC make func_8082EF20 true → the boomerang aim
// stance (link_boom_throw_wait pose) + the CAM_MODE_SLINGSHOT held camera (func_8083868C, which we
// extended to cover the hammer). func_80831010 sets unk_AA5 = 3 for that camera; because unk_AA5
// != 0, Player_ActionHandler_0 skips the vanilla C-Up first-person peek on its own.
static void IKAxe_AimEnter(Player* player, PlayState* play) {
    player->stateFlags1 |= PLAYER_STATE1_USING_ZORA_BOOMERANG;
    player->unk_ACC = 10;
    func_80831010(player, play);
    player->speedXZ = 0.0f;
}

static void IKAxe_AimExit(Player* player) {
    player->stateFlags1 &= ~PLAYER_STATE1_USING_ZORA_BOOMERANG;
    player->unk_ACC = 0;
    player->unk_AA5 = PLAYER_UNKAA5_0;
}

static void IKAxe_UpdateThrow(Player* player, PlayState* play) {
    static s16 sFlyingFrames = 0;

    // Recover from FLYING: either the axe was caught (En_Boom clears BOOMERANG_THROWN), or it never
    // came back (despawned on a scene change / killed) and the flag is stuck. Either way → IDLE.
    if (sIKAxeThrowState == IKAXE_THROW_FLYING) {
        u8 caught = !(player->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN);
        u8 lost = (++sFlyingFrames > 480); // ~16s — the axe should have long since returned
        if (caught || lost) {
            sFlyingFrames = 0;
            sIKAxeThrowState = IKAXE_THROW_IDLE;
            if (lost) {
                player->stateFlags1 &= ~PLAYER_STATE1_ZORA_BOOMERANG_THROWN; // never leave it stuck
            }
            // The boomerang catch chain leaves the RANGED-item upper-action (func_80835800), so the
            // next C-press fires a bow. Restore the hammer's melee upper-action — but ONLY if the
            // hammer is STILL held. If the player swapped weapons mid-flight, forcing the hammer
            // back desyncs heldItemAction (the new weapon shows but acts as the hammer) and
            // permanently breaks its collisions (no recoil/bonk). Leave a swapped weapon alone.
            if (player->heldItemAction == PLAYER_IA_HAMMER) {
                Player_EndIKAxeThrow(player);
            }
        }
    } else {
        sFlyingFrames = 0;
    }

    // Aim only while the hammer/axe is the drawn weapon (like the rods only aim while held).
    if (player->heldItemAction != PLAYER_IA_HAMMER && sIKAxeThrowState != IKAXE_THROW_FLYING) {
        if (sIKAxeThrowState == IKAXE_THROW_CHARGING) {
            IKAxe_AimExit(player);
        }
        sIKAxeThrowState = IKAXE_THROW_IDLE;
        return;
    }

    // Which C-button the hammer is on + whether it was pressed this frame (rod-style input).
    ItemInputState in;
    ItemInput_Update(&in, ITEM_HAMMER, player, play);

    switch (sIKAxeThrowState) {
        case IKAXE_THROW_IDLE:
            // C-Up enters the boomerang-style aim (stance + held camera), NOT the vanilla peek.
            if (!(player->stateFlags1 & PLAYER_STATE1_ZORA_BOOMERANG_THROWN) && player->meleeWeaponState == 0 &&
                CHECK_BTN_ALL(play->state.input[0].press.button, BTN_CUP)) {
                IKAxe_AimEnter(player, play);
                sIKAxeThrowState = IKAXE_THROW_CHARGING;
            }
            break;

        case IKAXE_THROW_CHARGING:
            player->unk_ACC = 10;        // keep the boomerang aim stance active
            func_80831010(player, play); // keep the held (SLINGSHOT) aim camera each frame

            // Launch on the EQUIPPED C-button press (the hammer's button), exactly like the rod fires.
            if (in.isPressed) {
                // In the held (SLINGSHOT) aim Link's body faces the aim horizontally (shape.rot.y)
                // and the pitch is the look pitch (focus.rot.x) — same as the OoT boomerang throw.
                s16 aimYaw = player->actor.shape.rot.y;
                s16 aimPitch = player->actor.focus.rot.x;

                f32 posX = player->actor.world.pos.x + (Math_SinS(aimYaw) * 10.0f);
                f32 posZ = player->actor.world.pos.z + (Math_CosS(aimYaw) * 10.0f);
                // MM's native En_Boom, spawned as the IKAXE_TOMAHAWK variant: OoT boomerang flight/
                // return physics (catch clears ZORA_BOOMERANG_THROWN), but z_en_boom.c draws the axe
                // DL (gIKAxeInlineDL) instead of the boomerang mesh and deals Goron-punch damage.
                // moveTo = NULL → no Z-target homing, flies straight in the AIMED direction and
                // returns. unk_1CC = 36 outbound frames, same as item_oot_boomerang.c.
                EnBoom* axe =
                    (EnBoom*)Actor_Spawn(&play->actorCtx, play, ACTOR_EN_BOOM, posX, player->actor.world.pos.y + 30.0f,
                                         posZ, aimPitch, aimYaw, 0, IKAXE_TOMAHAWK);
                if (axe != NULL) {
                    axe->moveTo = NULL;
                    axe->unk_1CC = 36;
                    player->zoraBoomerangActor = &axe->actor;
                    player->stateFlags1 |= PLAYER_STATE1_ZORA_BOOMERANG_THROWN;
                    player->stateFlags3 &= ~PLAYER_STATE3_ZORA_BOOMERANG_CAUGHT;
                    sIKAxeThrown = 1;
                    Player_PlaySfx(player, NA_SE_IT_BOOMERANG_THROW);
                }
                IKAxe_AimExit(player); // leave the aim mode → handsfree throw
                // Play the real boomerang throw animation and go handsfree (Link can move while
                // the axe is in the air), exactly like the boomerang.
                Player_StartIKAxeThrow(player, play);
                sIKAxeThrowState = IKAXE_THROW_FLYING;
                break;
            }

            // C-Up toggles aim off; any OTHER action button (besides the hammer's) / damage / cutscene cancels.
            {
                u16 exitButtons = BTN_A | BTN_B | BTN_CLEFT | BTN_CRIGHT | BTN_CDOWN | BTN_CUP;
                if (in.equippedButton) {
                    exitButtons &= ~in.equippedButton;
                }
                if (CHECK_BTN_ANY(play->state.input[0].press.button, exitButtons) ||
                    (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_DAMAGED))) {
                    IKAxe_AimExit(player);
                    sIKAxeThrowState = IKAXE_THROW_IDLE;
                }
            }
            break;

        case IKAXE_THROW_FLYING:
            // Wait for the axe to come back (handled by the FLYING→IDLE sync at the top).
            break;
    }
}

// Aim state for the crash-free BowReticle.cpp reticle (the FirstPerson_DrawReticle path NaN-crashes
// for the melee hammer). True from C-Up aim entry until launch/cancel. The aim itself uses the SAME
// mechanism as the magic rods (FirstPerson_Init + MM's vanilla first-person camera via func_80831814
// on C-Up), so no separate camera plumbing is needed.
u8 IKAxe_IsAiming(void) {
    return (sIKAxeThrowState == IKAXE_THROW_CHARGING) ? 1 : 0;
}
