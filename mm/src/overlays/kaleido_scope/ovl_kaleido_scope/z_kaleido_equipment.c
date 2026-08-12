/*
 * File: z_kaleido_equipment.c
 * Overlay: ovl_kaleido_scope
 * Description: Pause Menu - Equipment Page (Skijer's NEI port, 1:1 with soh's
 *              z_kaleido_equipment.c structure adapted to MM's kaleido).
 *
 * REPLACES the vanilla Mask page (the 24 MM masks live on page 2 of the extended item kaleido).
 *
 * OoT layout on MM's mask-page grid (maskVtx, 6 cols x 4 rows). The equipment grid takes the
 * RIGHT FOUR columns (2..5) — exactly OoT's 4x4 equipment cursor space:
 *   col 0 = upgrades column (quiver / bomb bag / strength / scale — OoT rows 1:1)
 *   cols 1..3 = equipment (row 0 swords, row 1 shields, row 2 tunics, row 3 boots)
 * The Link doll fills the LEFT area (player's LIVE skeleton rendered into an isolated
 * framebuffer — gfx_create_framebuffer + gsSPSetFB/gsSPResetFB, the SoH pause-Link technique).
 *
 * Three sub-pages cycled with L (SoH freed-shoulder-button logic; the NES.c page-toggle
 * handler is patched to leave L to us on this page):
 *   0 "vanilla":   MM swords (Kokiri/Razor/Gilded) + MM shields (Hero/Mirror) functional;
 *                  OoT tunics/boots display-only (behavior per-item later).
 *   1 "extended":  the full NEI ExtEquip grid (3 swords / 3 shields / 3 tunics / 3 boots).
 *                  A equips on the body, C-left/down/right assigns to that C button.
 *   2 "transform": Broken-Modes form selector (Link / Mario / Pikachu).
 */

#include "z_kaleido_scope.h"
#include "interface/parameter_static/parameter_static.h"
#include "archives/icon_item_static/icon_item_static_yar.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/FleetShipCombo/FleetComboIds.h" // FC_SHIELD_* / FC_OOT_TUNIC/BOOTS ownership bits
#include "2s2h/FleetShipCombo/FleetShipCombo.h" // FleetShipCombo_GetActiveGame (combo ownership gate)
#include <libultraship/bridge/consolevariablebridge.h>

#include "mods/extended_equipment.h"        // ExtEquip_* (Skijer's NEI)
#include "mods/broken_items/broken_items.h" // BrokenItems_* form selector (Skijer's NEI)
#include "mods/nei_save.h"
#include "expansions/sm64/sm64_mario.h"

// MM natively has SWORD=0/SHIELD=1/TUNIC=2; BOOTS is the NEI accessory tag (3) — same value
// as mods/nei_oot_compat.h (that header is z_player-TU-only, so define it here too).
#ifndef EQUIP_TYPE_BOOTS
#define EQUIP_TYPE_BOOTS 3
#endif

void KaleidoScope_SetCursorVtxPos(PauseContext* pauseCtx, u16 vtxIndex, Vtx* vtx);
void KaleidoScope_MoveCursorToSpecialPos(PlayState* play, s16 cursorSpecialPos);
void KaleidoScope_MoveCursorFromSpecialPos(PlayState* play);
void KaleidoScope_DrawTexQuadRGBA32(GraphicsContext* gfxCtx, TexturePtr texture, u16 width, u16 height, u16 point);
Gfx* Gfx_DrawTexQuadIA8(Gfx* gfx, TexturePtr texture, s16 textureWidth, s16 textureHeight, u16 point);
void Interface_LoadItemIconImpl(PlayState* play, u8 btn);
void* ExtInv_GetItemIcon(u16 itemId);
void Player_DrawImpl(PlayState* play, void** skeleton, Vec3s* jointTable, s32 dListCount, s32 lod,
                     PlayerTransformation playerForm, s32 boots, s32 face, OverrideLimbDrawFlex overrideLimbDraw,
                     PostLimbDrawFlex postLimbDraw, Actor* actor);
s32 Player_OverrideLimbDrawGameplayDefault(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                           Actor* actor);
extern Gfx gCullBackDList[]; // segment 0x0C cull DL the player limb DLs reference (Player_DrawGameplay)
int gfx_create_framebuffer(uint32_t width, uint32_t height, uint32_t native_width, uint32_t native_height,
                           unsigned char upscale);
// NEI weapon-upgrade bits (mods/items/logic/weapon_upgrades.c): Kokiri->Razor->Gilded chain +
// the Biggoron->Great-Fairy longsword upgrade (drives the col-3 line icon/equipped state).
u8 WeaponUpgrade_KokiriLevel(void);
u8 WeaponUpgrade_HasRazor(void);
u8 WeaponUpgrade_HasGilded(void);
u8 WeaponUpgrade_HasGreatFairy(void);

// ---------------------------------------------------------------------------
// Doll framebuffer (OoT pause-Link dimensions)
// ---------------------------------------------------------------------------
#define EQUIP_DOLL_WIDTH 64   // on-screen size (OoT pause-Link dimensions)
#define EQUIP_DOLL_HEIGHT 112
#define EQUIP_DOLL_RES 3 // framebuffer supersampling factor (renders at 3x, shown at 1x)
#define EQUIP_DOLL_FBW (EQUIP_DOLL_WIDTH * EQUIP_DOLL_RES)
#define EQUIP_DOLL_FBH (EQUIP_DOLL_HEIGHT * EQUIP_DOLL_RES)

int gPauseLinkFrameBuffer = -1;

// ---------------------------------------------------------------------------
// Sub-page + cursor state (SoH: vanilla / ExtEquip page / Broken-Modes transform)
// ---------------------------------------------------------------------------
#define EQUIP_SUBPAGE_VANILLA 0
#define EQUIP_SUBPAGE_EXT 1
#define EQUIP_SUBPAGE_TRANSFORM 2

static s16 sEquipSubPage = EQUIP_SUBPAGE_VANILLA;
static s16 sEquipCursorX = 1; // 0 = upgrades column, 1..3 = equipment — OoT cursorX space
static s16 sEquipCursorY = 0; // 0..3 — OoT cursorY space
static s16 sTransformCursor = 0;

// The 4x4 grid lives on the mask page's own vertices: equipment cols 1..3 on maskVtx columns 3..5,
// and the UPGRADE column (col 0: Cape/Pendant toggles + strength/scale) on maskVtx column 0 — the
// free carved blocks at the far LEFT of the page (columns 1-2 sit under the Link doll panel).
// Keeping stock grid cells means the shared cursor tables (sItemMaskCursorsX/Y, indexed by
// cursorXIndex/cursorYIndex) position the drawn cursor for free.
#define EQUIP_GRID_COL_BASE 2
#define EQUIP_MASK_COL(col) ((col) == 0 ? 0 : ((col) + EQUIP_GRID_COL_BASE))
#define EQUIP_CELL(row, col) (((row) * MASK_GRID_COLS) + EQUIP_MASK_COL(col))

// Upgrade column (col 0) OWN vertices — OoT places its upgrade column at page x = -114 (the carved
// sockets on the page's left edge, z_kaleido_scope_PAL.c D_8082B12C), further left than maskVtx
// column 0 (-96). maskVtx can't be moved (the real mask page shares it), so the upgrade cells get
// their own 4 quads, built each draw with the mask-grid row formula. The drawn cursor on this page
// comes from sItemMaskCursorsX[cursorXIndex] (shared table, col-0 entry = -62 which pairs with
// maskVtx -96); for our shifted column the cursor X is overridden in the draw with the value below
// (cursor-space ≈ 0.8125 * cellCenterX + 3, from the col-0/col-3 table anchor points).
#define EQUIP_UPGRADE_COL_X (-112)      // quad x0, page space (OoT -114 + 2 margin)
#define EQUIP_UPGRADE_CURSOR_X (-76.5f) // cursor-space X centered on that column
static Vtx sUpgradeColVtx[4 * 4];

// Called from KaleidoScope_UpdateCursorSize (z_kaleido_scope_NES.c) AFTER it reads the shared
// sItemMaskCursorsX table and right before KaleidoScope_DrawCursor: when the equipment cursor sits
// on the shifted upgrade column, hand back its custom cursor X. Returns true when the override
// applies. (The table can't hold this column — it's shared with the real mask/item pages.)
s32 KaleidoEquip_GetUpgradeCursorX(f32* outX) {
    if ((sEquipSubPage != EQUIP_SUBPAGE_TRANSFORM) && (sEquipCursorX == 0)) {
        *outX = EQUIP_UPGRADE_CURSOR_X;
        return true;
    }
    return false;
}

static void KaleidoEquip_BuildUpgradeColVtx(PauseContext* pauseCtx) {
    s16 row;

    for (row = 0; row < 4; row++) {
        Vtx* v = &sUpgradeColVtx[row * 4];
        s16 y0 = (MASK_GRID_ROWS * MASK_GRID_CELL_HEIGHT) / 2 - 6 - (row * MASK_GRID_CELL_HEIGHT) +
                 pauseCtx->offsetY - MASK_GRID_QUAD_MARGIN;

        v[0].v.ob[0] = v[2].v.ob[0] = EQUIP_UPGRADE_COL_X;
        v[1].v.ob[0] = v[3].v.ob[0] = EQUIP_UPGRADE_COL_X + MASK_GRID_QUAD_WIDTH;
        v[0].v.ob[1] = v[1].v.ob[1] = y0;
        v[2].v.ob[1] = v[3].v.ob[1] = y0 - MASK_GRID_QUAD_HEIGHT;
        v[0].v.ob[2] = v[1].v.ob[2] = v[2].v.ob[2] = v[3].v.ob[2] = 0;
        v[0].v.flag = v[1].v.flag = v[2].v.flag = v[3].v.flag = 0;
        v[0].v.tc[0] = v[0].v.tc[1] = v[1].v.tc[1] = v[2].v.tc[0] = 0;
        v[1].v.tc[0] = v[2].v.tc[1] = v[3].v.tc[0] = v[3].v.tc[1] = MASK_GRID_QUAD_TEX_SIZE * (1 << 5);
        v[0].v.cn[0] = v[1].v.cn[0] = v[2].v.cn[0] = v[3].v.cn[0] = 255;
        v[0].v.cn[1] = v[1].v.cn[1] = v[2].v.cn[1] = v[3].v.cn[1] = 255;
        v[0].v.cn[2] = v[1].v.cn[2] = v[2].v.cn[2] = v[3].v.cn[2] = 255;
        v[0].v.cn[3] = v[1].v.cn[3] = v[2].v.cn[3] = v[3].v.cn[3] = 255;
    }
}

// entry: itemId shown for icon/name, or -1 = empty cell.
typedef struct {
    s16 item;            // icon/name item id (-1 = empty; ids >= 0xE0 are NEI customs, no name)
    s16 equipType;       // >=0 ExtEquip type; -1 empty; -2 upgrade; -3 sword; -4 shield; -5 display-only
    s16 index;           // equip value / ext index (1-based) / upgrade row
    const char* ootIcon; // OoT icon OTR path (used when the OoT archive is present in mods/)
} EquipCell;

#define CELL_EMPTY (-1)
#define CELL_UPGRADE (-2)
#define CELL_SWORD (-3)   // Kokiri progressive line (Kokiri -> Razor -> Gilded, MM native)
#define CELL_SHIELD (-4)
#define CELL_DISPLAY (-5)
#define CELL_SWORD_MASTER (-6) // Master line (Master -> True Master; own IA, OoT blade, full-HP beam)
#define CELL_SWORD_BGS (-7)    // Longsword line (Biggoron -> Great Fairy's Sword; two-handed IA)
#define CELL_TUNIC (-8)        // OoT tunic (Kokiri/Goron/Zora): Nei_Save()->vanillaTunic, index 0..2
#define CELL_BOOTS (-9)        // OoT boots (Kokiri/Iron/Hover): Nei_Save()->vanillaBoots, index 0..2
#define CELL_SHIELD_DEKU (-10)       // Deku shield: equips AS Hero (index 1) but sets the Deku skin flag
#define CELL_SHIELD_OOT_MIRROR (-11) // OoT Mirror: equips AS Mirror (2) + skin flag 2 (draws OoT mirror)

// OoT icon paths for the sword lines (fall back to the MM item icon when oot.o2r is absent).
#define OOT_ICON_SWORD_MASTER "__OTR__textures/icon_item_static/gItemIconSwordMasterTex"
#define OOT_ICON_SWORD_BIGGORON "__OTR__textures/icon_item_static/gItemIconSwordBiggoronTex"
#define OOT_ICON_SHIELD_DEKU "__OTR__textures/icon_item_static/gItemIconShieldDekuTex"
#define OOT_ICON_SHIELD_HYLIAN "__OTR__textures/icon_item_static/gItemIconShieldHylianTex"
#define OOT_ICON_SHIELD_MIRROR "__OTR__textures/icon_item_static/gItemIconShieldMirrorTex"

// OoT icon layer: these load when the player drops their SoH/OoT archive (soh.otr / oot.otr)
// into 2ship's mods/ folder — OTR path strings ARE texture pointers in LUS. FileExists-gated
// so absence costs nothing (falls back to the MM icons).
static void* KaleidoEquip_OotTex(const char* path) {
    extern u8 ResourceMgr_FileExists(const char* resName);
    if (path == NULL || !ResourceMgr_FileExists(path)) {
        return NULL;
    }
    return (void*)path;
}

// ---------------------------------------------------------------------------
// Left column (col 0) — Skijer 2026-07-29 (kaleido re-layout):
//   row 0 = MAGIC CAPE          (A toggles the cloth visibility; its half-cost passive is always on)
//   row 1 = PENDANT OF MEMORIES (A toggles its whole moveset)
//   row 2 = RESERVED ext slot 1 } freed when quiver / bomb bag / STRENGTH / SWIM all moved to the
//   row 3 = RESERVED ext slot 2 } quest page. Placeholder icon, hoverable, A error-beeps.
// "Value" here = the cell is populated (drives landability + draw).
// ---------------------------------------------------------------------------
#define EQUIP_UPGRADE_ROW_RESERVED_1 2
#define EQUIP_UPGRADE_ROW_RESERVED_2 3

// Page-2 grid rows. The BOOTS row is NOT C-assignable: all three are real boots (equipped with A),
// and its middle id (0xEA) belongs to the Pendant of Memories in the inventory/C-button id space.
#define EQUIP_EXT_ROW_BOOTS 3

static s16 KaleidoEquip_UpgradeValue(s16 row) {
    switch (row) {
        case 0:
            return ExtEquip_CapeOwned();
        case 1:
            return ExtEquip_PendantOwned();
        case EQUIP_UPGRADE_ROW_RESERVED_1:
        case EQUIP_UPGRADE_ROW_RESERVED_2:
            return 1; // reserved: always drawn/hoverable until the item that lands here is decided
    }
    return 0;
}

static void* KaleidoEquip_UpgradeIcon(s16 row, s16 value) {
    if (value <= 0) {
        return NULL;
    }
    switch (row) {
        // Rows 0/1 = Magic Cape / Pendant of Memories (cape from the NEI icon set, pendant from mm.o2r).
        case 0:
            return ExtEquip_GetCapeIcon();
        case 1:
            return ExtEquip_GetPendantIcon();
        // Rows 2/3 = the two reserved ext slots (see KaleidoEquip_UpgradeValue).
        case EQUIP_UPGRADE_ROW_RESERVED_1:
        case EQUIP_UPGRADE_ROW_RESERVED_2:
            return KaleidoEquip_OotTex("__OTR__textures/icon_item_custom/gItemIconReservedSlotTex");
    }
    return NULL;
}

// Item id whose NAME texture labels the hovered upgrade (MM name table only has the
// native rows; the OoT-only rows show no name).
static s16 KaleidoEquip_UpgradeNameItem(s16 row, s16 value) {
    if (value <= 0) {
        return -1;
    }
    // Skijer 2026-07-16: rows 0/1 are the Cape/Pendant passives now — their NEI ids (0xE6/0xEA)
    // have no vanilla name texture, so no name label (same as the strength/scale rows).
    return -1;
}

// ---------------------------------------------------------------------------
// Cell tables (cols 1..3 of the grid)
// ---------------------------------------------------------------------------
static void KaleidoEquip_GetCell(s16 subPage, s16 row, s16 col, EquipCell* out) {
    // OoT tunic/boot icon set for the display-only rows (loads from the player's OoT archive)
    static const char* sOotIcons[2][3] = {
        { "__OTR__textures/icon_item_static/gItemIconTunicKokiriTex",
          "__OTR__textures/icon_item_static/gItemIconTunicGoronTex",
          "__OTR__textures/icon_item_static/gItemIconTunicZoraTex" },
        { "__OTR__textures/icon_item_static/gItemIconBootsKokiriTex",
          "__OTR__textures/icon_item_static/gItemIconBootsIronTex",
          "__OTR__textures/icon_item_static/gItemIconBootsHoverTex" },
    };

    out->item = -1;
    out->equipType = CELL_EMPTY;
    out->index = col;
    out->ootIcon = NULL;

    if (col < 1 || col > 3 || row < 0 || row > 3) {
        return;
    }

    if (subPage == EQUIP_SUBPAGE_VANILLA) {
        if (row == 0) {
            // OoT-style 3 progressive sword LINES (one column each):
            //   col1 = Kokiri line   (Kokiri -> Razor -> Gilded; MM's native progressive sword)
            //   col2 = Master line   (Master -> True Master; own IA, OoT blade, full-HP beam)
            //   col3 = Longsword line (Biggoron -> Great Fairy's Sword; MM two-handed IA)
            out->index = col;
            if (col == 1) {
                out->equipType = CELL_SWORD;
                out->item = ITEM_SWORD_KOKIRI + WeaponUpgrade_KokiriLevel(); // icon = current tier
            } else if (col == 2) {
                out->equipType = CELL_SWORD_MASTER;
                out->item = ITEM_SWORD_MASTER;
                out->ootIcon = OOT_ICON_SWORD_MASTER;
            } else { // col == 3
                out->equipType = CELL_SWORD_BGS;
                // GFS icon once the Great Fairy upgrade is owned, else the Biggoron icon.
                out->item = WeaponUpgrade_HasGreatFairy() ? ITEM_SWORD_GREAT_FAIRY : ITEM_SWORD_BGS;
                out->ootIcon = WeaponUpgrade_HasGreatFairy() ? NULL : OOT_ICON_SWORD_BIGGORON;
            }
        } else if (row == 1) {
            // OoT shields: Deku (OoT DL, smaller, Hero IA) / Hero=Hylian / Mirror (MM mirror DL)
            if (col == 1) {
                out->equipType = CELL_SHIELD_DEKU;
                out->index = EQUIP_VALUE_SHIELD_HERO; // Deku equips AS Hero + the Deku skin flag
                out->item = ITEM_SHIELD_HERO;
                out->ootIcon = OOT_ICON_SHIELD_DEKU;
            } else if (col == 2) {
                out->equipType = CELL_SHIELD;
                out->index = EQUIP_VALUE_SHIELD_HERO;
                out->item = ITEM_SHIELD_HERO;
                out->ootIcon = OOT_ICON_SHIELD_HYLIAN;
            } else { // col == 3 — OoT Mirror shield (distinct from the MM Mirror = Shield of Ikana)
                out->equipType = CELL_SHIELD_OOT_MIRROR;
                out->index = EQUIP_VALUE_SHIELD_MIRROR;
                out->item = ITEM_SHIELD_MIRROR;
                out->ootIcon = OOT_ICON_SHIELD_MIRROR;
            }
        } else if (row == 2) {
            // OoT tunics (functional): Kokiri / Goron (fireproof) / Zora (gas-immune).
            out->equipType = CELL_TUNIC;
            out->index = col - 1; // 0=Kokiri, 1=Goron, 2=Zora
            out->item = ITEM_EXT_TUNIC_1 + (col - 1); // fallback icon when the OoT archive is absent
            out->ootIcon = sOotIcons[0][col - 1];
        } else if (row == 3) {
            // OoT boots (functional): Kokiri / Iron (anti-knockback) / Hover (air-float).
            out->equipType = CELL_BOOTS;
            out->index = col - 1; // 0=Kokiri, 1=Iron, 2=Hover
            out->item = ITEM_EXT_BOOTS_1 + (col - 1);
            out->ootIcon = sOotIcons[1][col - 1];
        }
    } else if (subPage == EQUIP_SUBPAGE_EXT) {
        // The FULL extended-equipment set, 1:1 with the OoT build:
        // rows = sword/shield/tunic/boots, cols = ext index 1..3.
        static const s16 sExtRowType[4] = { EQUIP_TYPE_SWORD, EQUIP_TYPE_SHIELD, EQUIP_TYPE_TUNIC, EQUIP_TYPE_BOOTS };
        // Skijer 2026-07-16: retired slots stay EMPTY (Cape/Pendant live on the upgrade column;
        // Water Dragon Scale deleted — Zora swim = Zora Tunic effect). Reserved for new boots.
        if (ExtEquip_SlotRetired(sExtRowType[row], (u8)col)) {
            return;
        }
        out->equipType = sExtRowType[row];
        out->index = col;
        out->item = (s16)ExtEquip_GetItemId(out->equipType, (u8)col);
    }
}

static u8 KaleidoEquip_CellOwned(EquipCell* cell) {
    switch (cell->equipType) {
        case CELL_EMPTY:
            return false;
        // Swords/shields: standalone keeps "always available" (no native owned-bitmask -> the
        // chicken-and-egg the comments warned about). In COMBO the ownership is synced before the
        // page opens (weaponUpgrades / comboObtained / shieldOwned), so gate on real ownership.
        case CELL_SWORD: // Kokiri line: owns at least the base Kokiri sword
            if (FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) >= EQUIP_VALUE_SWORD_KOKIRI) ||
                   (WeaponUpgrade_KokiriLevel() > 0);
        case CELL_SWORD_MASTER:
            if (FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return Nei_Save()->comboObtained[FC_OOT_SWORD_MASTER] != 0;
        case CELL_SWORD_BGS: // Biggoron line OR the Great Fairy's Sword upgrade
            if (FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return (Nei_Save()->comboObtained[FC_OOT_SWORD_BIGGORON] != 0) || WeaponUpgrade_HasGreatFairy();
        case CELL_SHIELD_DEKU:
            if (FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return (Nei_Save()->shieldOwned & FC_SHIELD_DEKU) != 0;
        case CELL_SHIELD_OOT_MIRROR:
            if (FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return (Nei_Save()->shieldOwned & FC_SHIELD_MIRROR_OOT) != 0;
        case CELL_SHIELD: // Hero (index 1) / MM Mirror = Ikana (index 2)
            if (FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return (Nei_Save()->shieldOwned & (cell->index == 1 ? FC_SHIELD_HYLIAN : FC_SHIELD_IKANA)) != 0;
        case CELL_TUNIC: // Kokiri (idx 0) always; in COMBO, Goron/Zora require synced ownership
                         // (comboObtained via FleetSync). Standalone keeps "always available".
            if (cell->index == 0 || FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return Nei_Save()->comboObtained[cell->index == 1 ? FC_OOT_TUNIC_GORON : FC_OOT_TUNIC_ZORA] != 0;
        case CELL_BOOTS: // Kokiri (idx 0) always; in COMBO, Iron/Hover require synced ownership.
            if (cell->index == 0 || FleetShipCombo_GetActiveGame() < 0) {
                return true;
            }
            return Nei_Save()->comboObtained[cell->index == 1 ? FC_OOT_BOOTS_IRON : FC_OOT_BOOTS_HOVER] != 0;
        case CELL_DISPLAY:
            return false; // shown greyed; not selectable-equippable yet
        default:
            return ExtEquip_HasItem(cell->equipType, (u8)cell->index);
    }
}

static u8 KaleidoEquip_CellEquipped(EquipCell* cell) {
    // Which sword line is on the B button right now (the B item is the reliable discriminator,
    // since the Master line reuses the Gilded equip value).
    u8 bItem = BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B);
    switch (cell->equipType) {
        case CELL_SWORD: // Kokiri line lit when a Kokiri/Razor/Gilded sword is on B (not Master)
            return (bItem == ITEM_SWORD_KOKIRI || bItem == ITEM_SWORD_RAZOR || bItem == ITEM_SWORD_GILDED) &&
                   ExtEquip_GetCurrent(EQUIP_TYPE_SWORD) == 0;
        case CELL_SWORD_MASTER:
            return (bItem == ITEM_SWORD_MASTER) && ExtEquip_GetCurrent(EQUIP_TYPE_SWORD) == 0;
        case CELL_SWORD_BGS:
            return (bItem == ITEM_SWORD_BGS || bItem == ITEM_SWORD_GREAT_FAIRY) &&
                   ExtEquip_GetCurrent(EQUIP_TYPE_SWORD) == 0;
        case CELL_SHIELD: // native Hero/Mirror lit only when NOT wearing the Deku skin
            return GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == cell->index &&
                   ExtEquip_GetCurrent(EQUIP_TYPE_SHIELD) == 0 && Nei_Save()->vanillaShieldSkin == 0;
        case CELL_SHIELD_DEKU: // Deku = Hero equip value + the Deku skin flag
            return GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == EQUIP_VALUE_SHIELD_HERO &&
                   ExtEquip_GetCurrent(EQUIP_TYPE_SHIELD) == 0 && Nei_Save()->vanillaShieldSkin == 1;
        case CELL_SHIELD_OOT_MIRROR: // OoT Mirror = Mirror equip value + skin flag 2
            return GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == EQUIP_VALUE_SHIELD_MIRROR &&
                   ExtEquip_GetCurrent(EQUIP_TYPE_SHIELD) == 0 && Nei_Save()->vanillaShieldSkin == 2;
        case CELL_TUNIC:
            return Nei_Save()->vanillaTunic == cell->index;
        case CELL_BOOTS:
            return Nei_Save()->vanillaBoots == cell->index;
        case CELL_EMPTY:
        case CELL_DISPLAY:
            return false;
        default:
            return ExtEquip_GetCurrent(cell->equipType) == cell->index;
    }
}

// Can the cursor land on this grid position?
//
// Skijer 2026-07-31 FIX — this is why page 2 "no dejaba equipar nada". The old comment claimed SoH
// only lets the cursor sit on OWNED ext cells; it does the opposite. soh z_kaleido_equipment.c:398:
//
//     // On extended page, cursor can land on any equipment slot (all owned)
//     if (extEquipPage) { pauseAnyCursor = true; }
//
// and the entry scans at :569 / :610 read `(gBitFlags[...] & equipment) || extEquipPage`. So in OoT
// the whole 4x3 ext grid is always hoverable; ownership only gates the ACTION (ExtEquip_Equip
// silently refuses) and the DRAW (unowned cells render greyscaled — which KaleidoScope_DrawEquipment
// here already does). Requiring ownership to LAND meant that with no extEquipOwnedBits set — the
// normal state, since nothing in 2ship grants them outside the save editor — not one of the 12 cells
// was reachable: the page showed 12 greyed icons and the cursor refused to enter. And because
// ExtEquip_Equip is what flips CVAR_EXT_EQUIP_ENABLED on, ExtEquip_UpdateBehavior kept early-returning
// too, so none of the 12 behaviors ever ran either. Now 1:1 with OoT.
static u8 KaleidoEquip_CursorCanSit(s16 row, s16 col) {
    EquipCell cell;

    if (col == 0) {
        return KaleidoEquip_UpgradeValue(row) != 0;
    }
    KaleidoEquip_GetCell(sEquipSubPage, row, col, &cell);
    if (cell.equipType == CELL_EMPTY) {
        return false;
    }
    if (cell.equipType == CELL_DISPLAY) {
        return true; // hoverable for the name/preview, A just error-beeps
    }
    if (sEquipSubPage == EQUIP_SUBPAGE_EXT) {
        return true; // soh parity: every populated ext cell is hoverable, owned or not
    }
    return KaleidoEquip_CellOwned(&cell);
}

// ---------------------------------------------------------------------------
// Equip actions (A button)
// ---------------------------------------------------------------------------
static void KaleidoEquip_EquipCell(PlayState* play, s16 row, s16 col) {
    EquipCell cell;

    KaleidoEquip_GetCell(sEquipSubPage, row, col, &cell);

    if (cell.equipType == CELL_EMPTY || cell.equipType == CELL_DISPLAY || !KaleidoEquip_CellOwned(&cell)) {
        Audio_PlaySfx(NA_SE_SY_ERROR);
        return;
    }

    if (cell.equipType == CELL_SWORD) {
        // Kokiri line: equip the highest owned progressive tier (Kokiri/Razor/Gilded).
        u8 level = WeaponUpgrade_KokiriLevel(); // 0/1/2
        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI + level);
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_KOKIRI + level;
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        ExtEquip_Unequip(EQUIP_TYPE_SWORD);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    if (cell.equipType == CELL_SWORD_MASTER) {
        // Master line: reuse the Gilded equip value (one-handed mechanics) but put ITEM_SWORD_MASTER
        // on B so heldItemAction resolves to PLAYER_IA_SWORD_MASTER (own IA -> OoT blade + full-HP
        // beam once True Master is owned). z_player_lib.c overrides the blade DL by heldItemId.
        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_GILDED);
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_MASTER;
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        ExtEquip_Unequip(EQUIP_TYPE_SWORD);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    if (cell.equipType == CELL_SWORD_BGS) {
        // Longsword line: two-handed IA (ITEM_SWORD_BGS -> PLAYER_IA_SWORD_BIGGORON). The two-hand
        // model group draws gPlayerLeftHandTwoHandSwordDLs (Great Fairy's Sword); a distinct
        // Biggoron blade DL (pre-Great-Fairy) is a follow-up. GFS HP/MP recover is already wired
        // (GreatFairySword_* via ExtEquip_OnMeleeHit) once WeaponUpgrade_HasGreatFairy().
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_BGS;
        Interface_LoadItemIconImpl(play, EQUIP_SLOT_B);
        ExtEquip_Unequip(EQUIP_TYPE_SWORD);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    if (cell.equipType == CELL_SHIELD) {
        // Remember the shield being swapped out stays owned (MM has no owned-shield bitmask;
        // the NEI/FleetCombo shieldOwned store is exactly for this).
        u16 curShield = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD);
        if (curShield == 1) {
            Nei_Save()->shieldOwned |= FC_SHIELD_HYLIAN;
        } else if (curShield == 2) {
            Nei_Save()->shieldOwned |= FC_SHIELD_IKANA;
        }
        Nei_Save()->shieldOwned |= (cell.index == 1) ? FC_SHIELD_HYLIAN : FC_SHIELD_IKANA;
        SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, cell.index);
        Nei_Save()->vanillaShieldSkin = 0; // native Hero/Mirror model, not the Deku skin
        ExtEquip_Unequip(EQUIP_TYPE_SHIELD);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    if (cell.equipType == CELL_SHIELD_DEKU) {
        // Deku shield: equip AS Hero (same IA / raise gate / collider), then flag the Deku skin so
        // the draw shows OoT's smaller Deku model instead of the Hylian/Hero one.
        Nei_Save()->shieldOwned |= FC_SHIELD_DEKU | FC_SHIELD_HYLIAN;
        SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_HERO);
        Nei_Save()->vanillaShieldSkin = 1;
        ExtEquip_Unequip(EQUIP_TYPE_SHIELD);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    if (cell.equipType == CELL_SHIELD_OOT_MIRROR) {
        // OoT Mirror shield: equip AS the Mirror value (reflects light / blocks like Mirror), flag
        // skin 2 so the draw shows OoT's mirror model (child-hand-patched). Distinct from the MM
        // Mirror (= Shield of Ikana on the ext page).
        Nei_Save()->shieldOwned |= FC_SHIELD_MIRROR_OOT;
        SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_MIRROR);
        Nei_Save()->vanillaShieldSkin = 2;
        ExtEquip_Unequip(EQUIP_TYPE_SHIELD);
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    if (cell.equipType == CELL_TUNIC) {
        Nei_Save()->vanillaTunic = (u8)cell.index; // 0=Kokiri, 1=Goron (fireproof), 2=Zora (gas-immune)
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    if (cell.equipType == CELL_BOOTS) {
        Nei_Save()->vanillaBoots = (u8)cell.index; // 0=Kokiri, 1=Iron (anti-KB/wind), 2=Hover (float)
        Audio_PlaySfx(NA_SE_SY_DECIDE);
        return;
    }

    // Extended equipment (toggle: equipping the current one unequips it — SoH behavior)
    if (ExtEquip_GetCurrent(cell.equipType) == cell.index) {
        ExtEquip_Unequip(cell.equipType);
    } else {
        ExtEquip_Equip(cell.equipType, (u8)cell.index);
    }
    Audio_PlaySfx(NA_SE_SY_DECIDE);
}

// Ext page: C-left/down/right assigns the ext item to that C button for gameplay
// toggling (SoH's extended-page C-assign, on MM's button tables).
static void KaleidoEquip_AssignCButton(PlayState* play, s16 row, s16 col, u16 pressed) {
    EquipCell cell;
    s16 slot;

    KaleidoEquip_GetCell(sEquipSubPage, row, col, &cell);
    if (cell.equipType < 0 || !KaleidoEquip_CellOwned(&cell)) {
        Audio_PlaySfx(NA_SE_SY_ERROR);
        return;
    }

    if (pressed & BTN_CLEFT) {
        slot = EQUIP_SLOT_C_LEFT;
    } else if (pressed & BTN_CDOWN) {
        slot = EQUIP_SLOT_C_DOWN;
    } else {
        slot = EQUIP_SLOT_C_RIGHT;
    }

    // Toggle: if the same ext item is already on this button, clear it back to empty.
    if (BUTTON_ITEM_EQUIP(0, slot) == (u8)cell.item) {
        BUTTON_ITEM_EQUIP(0, slot) = ITEM_NONE;
    } else {
        BUTTON_ITEM_EQUIP(0, slot) = (u8)cell.item;
    }
    Interface_LoadItemIconImpl(play, slot);
    Audio_PlaySfx(NA_SE_SY_DECIDE);
}

// ---------------------------------------------------------------------------
// Sub-page cycling (SoH freed-button logic, MM's free shoulder button is L):
// vanilla -> ext (if ExtEquip on) -> transform (if Broken Modes on) -> vanilla.
// NES.c's KaleidoScope_HandlePageToggles is patched to leave L to us here.
// ---------------------------------------------------------------------------
static void KaleidoEquip_CycleSubPage(void) {
    s16 next = sEquipSubPage;

    do {
        next = (next + 1) % 3;
        // The extended-equipment page (vanilla-OoT <-> extended) is ALWAYS reachable with L — the
        // equipment menu is a core feature now, not gated behind the ExtEquip cheat. (The ext items'
        // ownership / gameplay behaviors are still governed by ExtEquip_IsEnabled elsewhere.)
        if (next == EQUIP_SUBPAGE_TRANSFORM && !BrokenItems_Enabled()) {
            continue; // transform (Broken Modes) page only when that feature is on
        }
        break;
    } while (next != sEquipSubPage);

    if (next != sEquipSubPage) {
        sEquipSubPage = next;
        // Keep the ExtEquip module's page state in sync (gameplay code reads it)
        if (ExtEquip_IsEnabled()) {
            int wantExt = (sEquipSubPage == EQUIP_SUBPAGE_EXT);
            if (ExtEquip_GetPage() != wantExt) {
                ExtEquip_SwitchPage();
            }
        }
        if (sEquipSubPage == EQUIP_SUBPAGE_TRANSFORM) {
            sTransformCursor = BrokenItems_CurrentForm();
        } else {
            // Land on a valid cell of the new page
            if (!KaleidoEquip_CursorCanSit(sEquipCursorY, sEquipCursorX)) {
                sEquipCursorY = 0;
                sEquipCursorX = 1;
            }
        }
        Audio_PlaySfx(NA_SE_SY_DECIDE);
    }
}

// ---------------------------------------------------------------------------
// Cursor / input (runs from the kaleido update, replacing UpdateMaskCursor)
// ---------------------------------------------------------------------------
void KaleidoScope_UpdateEquipmentCursor(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    Input* input = CONTROLLER1(&play->state);
    EquipCell cell;
    s16 col;
    s16 row;

    pauseCtx->cursorColorSet = PAUSE_CURSOR_COLOR_SET_WHITE;

    if (pauseCtx->pageIndex != PAUSE_MASK) {
        return;
    }

    ExtEquip_Update(); // tick the page-switch cooldown (Player_Update doesn't run while paused)

    // L cycles the sub-pages (only while actually interacting with the page)
    if ((pauseCtx->state == PAUSE_STATE_MAIN) && (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) &&
        CHECK_BTN_ALL(input->press.button, BTN_L) && ExtEquip_CanSwitch()) {
        KaleidoEquip_CycleSubPage();
    }

    // --- Transform sub-page: forms on row 0, cols 1..count ---
    if (sEquipSubPage == EQUIP_SUBPAGE_TRANSFORM) {
        s32 count = BrokenItems_FormCount();

        if (sTransformCursor >= count) {
            sTransformCursor = count - 1;
        }
        if (pauseCtx->cursorSpecialPos != 0) {
            if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT && pauseCtx->stickAdjX > 30) {
                KaleidoScope_MoveCursorFromSpecialPos(play);
                sTransformCursor = 0;
            } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT && pauseCtx->stickAdjX < -30) {
                KaleidoScope_MoveCursorFromSpecialPos(play);
                sTransformCursor = count - 1;
            } else {
                return;
            }
        } else if (pauseCtx->stickAdjX < -30) {
            if (sTransformCursor > 0) {
                sTransformCursor--;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            } else {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            }
        } else if (pauseCtx->stickAdjX > 30) {
            if (sTransformCursor < count - 1) {
                sTransformCursor++;
                Audio_PlaySfx(NA_SE_SY_CURSOR);
            } else {
                KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
            }
        }

        if ((pauseCtx->state == PAUSE_STATE_MAIN) && (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE) &&
            CHECK_BTN_ALL(input->press.button, BTN_A)) {
            BrokenItems_EquipForm(play, sTransformCursor);
            Audio_PlaySfx(NA_SE_SY_DECIDE);
        }

        // Name-panel guard: custom ids (>= 0xE0) would index past the name-texture tables.
        {
            u16 formItem = BrokenItems_FormItem(sTransformCursor);
            pauseCtx->cursorItem[PAUSE_MASK] = (formItem < 0xE0) ? formItem : PAUSE_ITEM_NONE;
        }
        pauseCtx->cursorSlot[PAUSE_MASK] = EQUIP_CELL(0, sTransformCursor + 1);
        pauseCtx->cursorPoint[PAUSE_MASK] = pauseCtx->cursorSlot[PAUSE_MASK];
        pauseCtx->cursorXIndex[PAUSE_MASK] = sTransformCursor + 1 + EQUIP_GRID_COL_BASE;
        pauseCtx->cursorYIndex[PAUSE_MASK] = 0;
        KaleidoScope_SetCursorVtxPos(pauseCtx, pauseCtx->cursorSlot[PAUSE_MASK] * 4, pauseCtx->maskVtx);
        return;
    }

    // --- Grid sub-pages (vanilla / ext): OoT 4x4 cursor space ---

    // Entering the grid from the page-switch special position (vanilla-cursor behavior):
    // scan for the first cell the cursor can sit on, nearest the entered side.
    if (pauseCtx->cursorSpecialPos != 0) {
        s16 fromLeft;

        if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_LEFT && pauseCtx->stickAdjX > 30) {
            fromLeft = true;
        } else if (pauseCtx->cursorSpecialPos == PAUSE_CURSOR_PAGE_RIGHT && pauseCtx->stickAdjX < -30) {
            fromLeft = false;
        } else {
            return;
        }
        KaleidoScope_MoveCursorFromSpecialPos(play);
        for (col = fromLeft ? 0 : 3; col >= 0 && col <= 3; col += fromLeft ? 1 : -1) {
            for (row = 0; row < 4; row++) {
                if (KaleidoEquip_CursorCanSit(row, col)) {
                    sEquipCursorX = col;
                    sEquipCursorY = row;
                    goto CURSOR_PLACED;
                }
            }
        }
        // Nothing selectable at all — bounce to the opposite special position (OoT behavior)
        KaleidoScope_MoveCursorToSpecialPos(play,
                                            fromLeft ? PAUSE_CURSOR_PAGE_RIGHT : PAUSE_CURSOR_PAGE_LEFT);
        return;
    CURSOR_PLACED:;
    } else {
        // Horizontal: step, skipping cells the cursor can't sit on; off the edge -> page arrows.
        if (pauseCtx->stickAdjX < -30) {
            for (col = sEquipCursorX - 1; col >= 0; col--) {
                // Same-row first, then any row in that column (OoT walks the column)
                for (row = 0; row < 4; row++) {
                    s16 tryRow = (sEquipCursorY + row) % 4;
                    if (KaleidoEquip_CursorCanSit(tryRow, col)) {
                        sEquipCursorX = col;
                        sEquipCursorY = tryRow;
                        Audio_PlaySfx(NA_SE_SY_CURSOR);
                        goto MOVED_H;
                    }
                }
            }
            KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_LEFT);
            return;
        } else if (pauseCtx->stickAdjX > 30) {
            for (col = sEquipCursorX + 1; col <= 3; col++) {
                for (row = 0; row < 4; row++) {
                    s16 tryRow = (sEquipCursorY + row) % 4;
                    if (KaleidoEquip_CursorCanSit(tryRow, col)) {
                        sEquipCursorX = col;
                        sEquipCursorY = tryRow;
                        Audio_PlaySfx(NA_SE_SY_CURSOR);
                        goto MOVED_H;
                    }
                }
            }
            KaleidoScope_MoveCursorToSpecialPos(play, PAUSE_CURSOR_PAGE_RIGHT);
            return;
        }
    MOVED_H:
        // Vertical: step within the column, skipping unusable cells; stop at the edges.
        if (pauseCtx->stickAdjY > 30) {
            for (row = sEquipCursorY - 1; row >= 0; row--) {
                if (KaleidoEquip_CursorCanSit(row, sEquipCursorX)) {
                    sEquipCursorY = row;
                    Audio_PlaySfx(NA_SE_SY_CURSOR);
                    break;
                }
            }
        } else if (pauseCtx->stickAdjY < -30) {
            for (row = sEquipCursorY + 1; row <= 3; row++) {
                if (KaleidoEquip_CursorCanSit(row, sEquipCursorX)) {
                    sEquipCursorY = row;
                    Audio_PlaySfx(NA_SE_SY_CURSOR);
                    break;
                }
            }
        }
    }

    // --- A / C buttons ---
    if ((pauseCtx->state == PAUSE_STATE_MAIN) && (pauseCtx->mainState == PAUSE_MAIN_STATE_IDLE)) {
        if (CHECK_BTN_ALL(input->press.button, BTN_A)) {
            if (sEquipCursorX == 0) {
                // Skijer 2026-07-16 (OoT-rework 1:1): A on upgrade row 0 toggles the Magic Cape's
                // VISIBILITY (its half-cost passive stays active while owned); A on row 1 toggles the
                // Pendant of Memories moveset. Solid icon = ON, half-transparent = OFF.
                if ((sEquipCursorY == 0) && ExtEquip_CapeOwned()) {
                    ExtEquip_ToggleCapeVisibility();
                    Audio_PlaySfx(NA_SE_SY_DECIDE);
                } else if ((sEquipCursorY == 1) && ExtEquip_PendantOwned()) {
                    ExtEquip_TogglePendantEffect();
                    Audio_PlaySfx(NA_SE_SY_DECIDE);
                } else {
                    Audio_PlaySfx(NA_SE_SY_ERROR); // strength/scale rows: view-only (OoT behavior)
                }
            } else {
                KaleidoEquip_EquipCell(play, sEquipCursorY, sEquipCursorX);
            }
        } else if ((sEquipSubPage == EQUIP_SUBPAGE_EXT) && (sEquipCursorX != 0) &&
                   (sEquipCursorY != EQUIP_EXT_ROW_BOOTS) &&
                   CHECK_BTN_ANY(input->press.button, BTN_CLEFT | BTN_CDOWN | BTN_CRIGHT)) {
            KaleidoEquip_AssignCButton(play, sEquipCursorY, sEquipCursorX,
                                       input->press.button & (BTN_CLEFT | BTN_CDOWN | BTN_CRIGHT));
        }
    }

    // --- Publish cursor state (name panel + drawn cursor) ---
    // Flag the name resolver that any ext id it sees is a page-2 GRID slot: the one shared id (0xEA)
    // means Climb Boots here and Pendant of Memories everywhere else. Skijer 2026-07-29
    gExtEquipGridNameContext = (sEquipSubPage == EQUIP_SUBPAGE_EXT) && (sEquipCursorX != 0);

    if (sEquipCursorX == 0) {
        s16 nameItem = KaleidoEquip_UpgradeNameItem(sEquipCursorY, KaleidoEquip_UpgradeValue(sEquipCursorY));
        pauseCtx->cursorItem[PAUSE_MASK] = (nameItem >= 0) ? (u16)nameItem : PAUSE_ITEM_NONE;
    } else {
        KaleidoEquip_GetCell(sEquipSubPage, sEquipCursorY, sEquipCursorX, &cell);
        // Name-panel guard: NEI custom ids (>= 0xE0) have no vanilla name texture (ext names
        // wired later via ExtEquip_GetNameTex).
        pauseCtx->cursorItem[PAUSE_MASK] =
            (cell.item >= 0 && cell.item < 0xE0) ? (u16)cell.item : PAUSE_ITEM_NONE;
    }
    pauseCtx->cursorSlot[PAUSE_MASK] = EQUIP_CELL(sEquipCursorY, sEquipCursorX);
    // Keep the shared cursor machinery in sync — the kaleido re-derives the drawn cursor from
    // cursorXIndex/cursorYIndex each frame (sItemMaskCursorsX/Y), so those must track our grid.
    pauseCtx->cursorPoint[PAUSE_MASK] = pauseCtx->cursorSlot[PAUSE_MASK];
    pauseCtx->cursorXIndex[PAUSE_MASK] = EQUIP_MASK_COL(sEquipCursorX); // col 0 = far-left sockets
    pauseCtx->cursorYIndex[PAUSE_MASK] = sEquipCursorY;
    KaleidoScope_SetCursorVtxPos(pauseCtx, pauseCtx->cursorSlot[PAUSE_MASK] * 4, pauseCtx->maskVtx);
}

// ---------------------------------------------------------------------------
// Link doll: render the player's LIVE skeleton into the isolated framebuffer.
// SoH technique: gsSPSetFB redirects the OPA stream into the doll FB, so none of
// the pause pages' RDP state/segments are affected. Composite happens later as a
// page-space quad (see KaleidoScope_DrawEquipment).
// ---------------------------------------------------------------------------
static void KaleidoEquip_RenderDollFB(PlayState* play) {
    static Vp sDollViewport = { (EQUIP_DOLL_FBW / 2) << 2, (EQUIP_DOLL_FBH / 2) << 2, G_MAXZ / 2, 0,
                                (EQUIP_DOLL_FBW / 2) << 2, (EQUIP_DOLL_FBH / 2) << 2, G_MAXZ / 2, 0 };
    static Lights1 sDollLights = gdSPDefLights1(80, 80, 80, 255, 255, 255, 84, 84, -84);
    Player* player = GET_PLAYER(play);
    Mtx* perspMtx;
    Mtx* lookAtMtx;
    u16 perspNorm;
    Vec3s dollRot = { 0, 32300, 0 };

    if (player == NULL || player->skelAnime.skeleton == NULL) {
        return;
    }

    if (gPauseLinkFrameBuffer == -1) {
        // upscale=false → the FB is exactly 64x112 pixels, so the ImageRectangle composite
        // below can sample it with deterministic texel coords.
        gPauseLinkFrameBuffer =
            gfx_create_framebuffer(EQUIP_DOLL_FBW, EQUIP_DOLL_FBH, EQUIP_DOLL_FBW, EQUIP_DOLL_FBH, false);
    }

    /* Mario mode owns the same isolated MM doll framebuffer. */
    if (Sm64Kaleido_DrawForm(play)) {
        return;
    }

    perspMtx = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx));
    lookAtMtx = GRAPH_ALLOC(play->state.gfxCtx, sizeof(Mtx));

    OPEN_DISPS(play->state.gfxCtx);

    // SoH structure: the doll renders during the WORK pass (before the main scene), inside the
    // isolated FB. The doll's commands are written into the OPA stream but EXECUTED from WORK
    // via a display-list call; the main OPA pass BRANCHES OVER them (opaRef patch below), so
    // the page's own drawing is completely untouched. (Doing SetFB mid-OPA killed the page.)
    Gfx* opaRef = POLY_OPA_DISP;
    POLY_OPA_DISP++;

    gsSPSetFB(WORK_DISP++, gPauseLinkFrameBuffer);
    gSPDisplayList(WORK_DISP++, POLY_OPA_DISP);

    // Clear the doll FB to transparent black so the page shows through around Link.
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, 0, 0, EQUIP_DOLL_FBW, EQUIP_DOLL_FBH);
    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_FILL);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_NOOP, G_RM_NOOP2);
    gDPSetFillColor(POLY_OPA_DISP++, (GPACK_RGBA5551(0, 0, 0, 0) << 16) | GPACK_RGBA5551(0, 0, 0, 0));
    gDPFillRectangle(POLY_OPA_DISP++, 0, 0, EQUIP_DOLL_FBW - 1, EQUIP_DOLL_FBH - 1);
    gDPPipeSync(POLY_OPA_DISP++);

    // 3D state for the doll
    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_2CYCLE);
    gSPLoadGeometryMode(POLY_OPA_DISP++, G_ZBUFFER | G_SHADE | G_CULL_BACK | G_LIGHTING | G_SHADING_SMOOTH);
    gSPTexture(POLY_OPA_DISP++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gSPViewport(POLY_OPA_DISP++, &sDollViewport);

    guPerspective(perspMtx, &perspNorm, 60.0f, (f32)EQUIP_DOLL_FBW / (f32)EQUIP_DOLL_FBH, 10.0f, 4000.0f, 1.0f);
    gSPPerspNormalize(POLY_OPA_DISP++, perspNorm);
    gSPMatrix(POLY_OPA_DISP++, perspMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
    guLookAt(lookAtMtx, 0.0f, 0.0f, -100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    gSPMatrix(POLY_OPA_DISP++, lookAtMtx, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);

    gSPSetLights1(POLY_OPA_DISP++, sDollLights);

    // CRITICAL: the player limb DLs reference segment 0x0C for the cull display list (set by
    // Player_DrawGameplay before Player_DrawImpl). Without it, segment 0x0C is garbage and the
    // limb DL is malformed -> gfx_step crash — most visibly on transformation forms (Deku/Goron/
    // Zora/FD), whose skeletons lean on it. Set it on both streams like the real player draw.
    gSPSegment(POLY_OPA_DISP++, 0x0C, gCullBackDList);
    gSPSegment(POLY_XLU_DISP++, 0x0C, gCullBackDList);

    Matrix_Push();
    // Camera sits at z=-100 looking at the origin (guLookAt above) — Link must be IN FRONT of
    // it (z >= ~-40). Per-form scale/height: Goron & Zora are much bulkier/taller than Link and
    // Deku, so they need a smaller scale (and lower feet) to fit the 64x112 viewport.
    {
        f32 dollScale = 0.016f;
        f32 dollY = -45.0f;
        switch (player->transformation) {
            case PLAYER_FORM_GORON:
                dollScale = 0.0105f; // bulky + tall
                dollY = -52.0f;
                break;
            case PLAYER_FORM_ZORA:
                dollScale = 0.0120f; // tall/slim
                dollY = -50.0f;
                break;
            case PLAYER_FORM_DEKU:
                dollScale = 0.0190f; // small
                dollY = -42.0f;
                break;
            default: // HUMAN / FIERCE_DEITY
                break;
        }
        Matrix_SetTranslateRotateYXZ(0.0f, dollY, 0.0f, &dollRot);
        Matrix_Scale(dollScale, dollScale, dollScale, MTXMODE_APPLY);
    }

    // Match Player_DrawGameplay's call: real face index + the gameplay post-limb-draw (held item
    // / shield / sheath). postLimbDraw NULL previously skipped setup some forms expect.
    Player_DrawImpl(play, (void**)player->skelAnime.skeleton, player->skelAnime.jointTable,
                    player->skelAnime.dListCount, 0, player->transformation, 0, player->actor.shape.face,
                    Player_OverrideLimbDrawGameplayDefault, Player_PostLimbDrawGameplay, &player->actor);

    Matrix_Pop();

    gDPPipeSync(POLY_OPA_DISP++);

    // Terminate the WORK-executed sublist, patch the reserved slot so the main OPA pass
    // jumps over the doll commands, and restore the main framebuffer for the WORK stream.
    gSPEndDisplayList(POLY_OPA_DISP++);
    gSPBranchList(opaRef, POLY_OPA_DISP);
    gsSPResetFB(WORK_DISP++);

    CLOSE_DISPS(play->state.gfxCtx);
}

// Composite the doll FB on the OVERLAY stream via gDPImageRectangle — the exact pattern
// 2ship's own framebuffer_effects.c uses (FB_DrawFromFramebuffer), guaranteed to sample FBs.
// Screen-space (doesn't tilt with the page cube during transitions — acceptable v1).
static void KaleidoEquip_DrawDollImage(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    extern Mtx gIdentityMtx;
    // Screen rect (320x240 space) — OoT equipment layout order: [upgrade column | Link | grid].
    // The upgrade column lives on maskVtx column 0 (page x -96..-68 ≈ screen 64..92), the equipment
    // grid on maskVtx columns 3..5 (page x 0.. ≈ screen 160..). The doll fills the gap between them
    // (screen 96..160) so it no longer covers the upgrade cells (Skijer 2026-07-16 position fix).
    s32 x0 = 86; // centered in the upgrades(-112..-84)..grid(0..) gap, like OoT's Link at page ~-36
    s32 y0 = 68;
    s32 x1 = x0 + EQUIP_DOLL_WIDTH;
    s32 y1 = y0 + EQUIP_DOLL_HEIGHT;

    if (gPauseLinkFrameBuffer == -1 || pauseCtx->pageIndex != PAUSE_MASK) {
        return;
    }

    OPEN_DISPS(play->state.gfxCtx);

    gSPMatrix(OVERLAY_DISP++, &gIdentityMtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gDPSetEnvColor(OVERLAY_DISP++, 255, 255, 255, pauseCtx->alpha);
    gDPSetOtherMode(OVERLAY_DISP++,
                    G_AD_NOISE | G_CD_NOISE | G_CK_NONE | G_TC_FILT | G_TF_POINT | G_TT_NONE | G_TL_TILE |
                        G_TD_CLAMP | G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                    G_AC_NONE | G_ZS_PRIM | G_RM_CLD_SURF | G_RM_CLD_SURF2);
    gSPClearGeometryMode(OVERLAY_DISP++, G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR);
    gSPSetGeometryMode(OVERLAY_DISP++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    // clang-format off
    gDPSetCombineLERP(OVERLAY_DISP++, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, ENVIRONMENT);
    // clang-format on

    gDPSetTextureImageFB(OVERLAY_DISP++, 0, 0, 0, gPauseLinkFrameBuffer);
    gDPImageRectangle(OVERLAY_DISP++, x0 << 2, y0 << 2, 0, 0, x1 << 2, y1 << 2, EQUIP_DOLL_FBW, EQUIP_DOLL_FBH,
                      G_TX_RENDERTILE, EQUIP_DOLL_FBW, EQUIP_DOLL_FBH);

    gDPPipeSync(OVERLAY_DISP++);

    CLOSE_DISPS(play->state.gfxCtx);
}

// ---------------------------------------------------------------------------
// Page draw (replaces KaleidoScope_DrawMaskSelect at the dispatch sites)
// ---------------------------------------------------------------------------
void KaleidoScope_DrawEquipment(PlayState* play) {
    PauseContext* pauseCtx = &play->pauseCtx;
    s16 row;
    s16 col;
    EquipCell cell;

    // SoH parity: suppress ext icon overrides so vanilla sword/shield icons show here
    gExtEquipSuppressIconOverride = 1;

    OPEN_DISPS(play->state.gfxCtx);

    // Render the doll into its isolated FB first (only while this page is active).
    if (pauseCtx->pageIndex == PAUSE_MASK) {
        KaleidoEquip_RenderDollFB(play);
    }

    Gfx_SetupDL42_Opa(play->state.gfxCtx);
    gDPSetCombineMode(POLY_OPA_DISP++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    if (sEquipSubPage == EQUIP_SUBPAGE_TRANSFORM) {
        // Form selector: form icons on row 0, cols 1..count
        s32 i;
        for (i = 0; i < BrokenItems_FormCount(); i++) {
            void* tex = BrokenItems_FormIconTex(i);
            if (tex == NULL) {
                continue;
            }
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255,
                            (i == BrokenItems_CurrentForm()) ? pauseCtx->alpha : (pauseCtx->alpha * 2) / 3);
            gSPVertex(POLY_OPA_DISP++, &pauseCtx->maskVtx[EQUIP_CELL(0, i + 1) * 4], 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, tex, 32, 32, 0);
        }
    } else {
        // --- Upgrades column (col 0): Cape / Pendant toggles + strength / scale (OoT-rework 1:1) ---
        // Own vertices at the page's left-edge sockets (OoT x = -114), not maskVtx column 0.
        KaleidoEquip_BuildUpgradeColVtx(pauseCtx);
        for (row = 0; row < 4; row++) {
            s16 value = KaleidoEquip_UpgradeValue(row);
            void* icon = KaleidoEquip_UpgradeIcon(row, value);
            u8 upgAlpha;
            if (icon == NULL) {
                continue;
            }
            // Rows 0/1 show their toggle state spiritual-stones style: solid = ON, half = OFF.
            if (row == 0) {
                upgAlpha = ExtEquip_CapeVisible() ? pauseCtx->alpha : (pauseCtx->alpha >> 1);
            } else if (row == 1) {
                upgAlpha = ExtEquip_PendantActive() ? pauseCtx->alpha : (pauseCtx->alpha >> 1);
            } else {
                upgAlpha = pauseCtx->alpha;
            }
            gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, upgAlpha);
            gSPVertex(POLY_OPA_DISP++, &sUpgradeColVtx[row * 4], 4, 0);
            KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, icon, 32, 32, 0);
        }

        // (Cursor X for this shifted column is overridden inside KaleidoScope_UpdateCursorSize via
        // KaleidoEquip_GetUpgradeCursorX — that runs right before KaleidoScope_DrawCursor, AFTER the
        // shared sItemMaskCursorsX table write, so it can't be stomped.)

        // --- Equipment grid (cols 1..3) ---
        for (row = 0; row < 4; row++) {
            for (col = 1; col <= 3; col++) {
                void* icon;
                u8 owned;
                u8 equipped;

                KaleidoEquip_GetCell(sEquipSubPage, row, col, &cell);
                if (cell.equipType == CELL_EMPTY) {
                    continue;
                }

                owned = KaleidoEquip_CellOwned(&cell);
                equipped = KaleidoEquip_CellEquipped(&cell);

                // OoT icon first (visual 1:1 when the OoT archive is in mods/), then the
                // ext-equipment table, then the MM item icon.
                icon = KaleidoEquip_OotTex(cell.ootIcon);
                if (icon == NULL) {
                    if (cell.equipType >= 0) {
                        // FileExists-gated: ExtEquip_GetIcon hands back an OTR PATH, and a path that
                        // isn't in the archive does NOT draw nothing — gDPLoadTextureBlock leaves
                        // whatever was last in TMEM, so the cell shows the PREVIOUS cell's icon. That
                        // reads as "the whole grid is in the wrong order" (swords repeated, tunics on
                        // the boots row) whenever an icon PNG hasn't been packed yet. Skijer 2026-07-29
                        icon = KaleidoEquip_OotTex(ExtEquip_GetIcon(cell.equipType, (u8)cell.index));
                    } else {
                        icon = ExtInv_GetItemIcon((u16)cell.item);
                    }
                }
                if (icon == NULL) {
                    continue;
                }

                // OoT-style equipped square (white outline) behind the equipped cell
                if (equipped) {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255, pauseCtx->alpha);
                    gSPVertex(POLY_OPA_DISP++, &pauseCtx->maskVtx[EQUIP_CELL(row, col) * 4], 4, 0);
                    POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, gEquippedItemOutlineTex, 32, 32, 0);
                }
                if (!owned) {
                    gDPSetGrayscaleColor(POLY_OPA_DISP++, 109, 109, 109, 255);
                    gSPGrayscale(POLY_OPA_DISP++, true);
                }
                gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 255, 255, 255,
                                equipped ? pauseCtx->alpha : (pauseCtx->alpha * 3) / 4);
                gSPVertex(POLY_OPA_DISP++, &pauseCtx->maskVtx[EQUIP_CELL(row, col) * 4], 4, 0);
                KaleidoScope_DrawTexQuadRGBA32(play->state.gfxCtx, icon, 32, 32, 0);
                if (!owned) {
                    gSPGrayscale(POLY_OPA_DISP++, false);
                }
            }
        }
    }

    // Composite the Link doll framebuffer on the left (OoT layout).
    KaleidoEquip_DrawDollImage(play);

    CLOSE_DISPS(play->state.gfxCtx);

    gExtEquipSuppressIconOverride = 0;
}
