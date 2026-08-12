# OoT-actor adaptation TODO (NEI items → MM)

Several ported NEI items reference **OoT actors that don't exist in MM**. To reach
compile+boot, the missing OoT actor IDs are defined as **never-matching sentinels**
in [`mm/mods/nei_oot_compat.h`](../nei_oot_compat.h) (`0x7F01…0x7F21`), so every
`actor->id == ACTOR_*` check compiles but **never fires in MM** — the OoT-specific
interaction is INERT until adapted.

When adapting each item, look up the MM actor list, swap these for the MM
equivalent(s), and **preserve any reward drops** (keys / rupees / hearts) — we always
want to reclaim their rewards.

> Ball & Chain is already MM-adapted (smashes MM pots/crates/grass/rocks with a small
> drop — see `logic/item_ballchain.c`). Ice Rod's OoT red-ice melt was dropped (no MM
> analog); it keeps its freeze-enemies function.

| Item | OoT actors referenced | Interaction (inferred — verify when adapting) | Reward? |
|------|-----------------------|-----------------------------------------------|---------|
| `logic/item_beetle.c` | `EN_G_SWITCH` | Beetle triggers a Gerudo silver-rupee/eye switch → MM crystal/eye switches | rupee? |
| `logic/item_dominionrod.c` | `EN_ANUBICE`, `EN_ANUBICE_FIRE` | Rod animates/controls Anubis statues → no MM equiv (pick a statue/enemy) | no |
| `logic/item_hylias_grace.h` | `BG_HAKA_HUTA`, `BG_MIZU_SHUTTER`, `BG_SPOT18_FUTA`, `BG_SPOT18_SHUTTER`, `DOOR_GERUDO`, `DOOR_KILLER`, `DOOR_TOKI` | Fairy passes through these doors/shutters → map to MM door/shutter actors | no |
| `logic/item_lantern.c` | `BG_PO_SYOKUDAI`, `EN_BW`, `EN_ICE_HONO`, `EN_PO_DESERT`, `EN_PO_FIELD` | Lantern lights torches / ignites torch-slugs / burns Poes → MM torches + flame actors | maybe |
| `logic/item_rod_light.c` | `EN_DH`, `EN_DHA`, `EN_PO_DESERT`, `EN_PO_FIELD`, `EN_PO_RELAY` | Light rod does bonus damage to undead (Dead Hand, Poes) → MM undead (ReDead, etc.) | no |
| `logic/item_shovel.c` (+`.h`) | `EN_HINTNUTS`, `EN_REEBA` | Shovel digs up hint scrub / Leevers → MM buried actors (Leevers, grotto diggers) | **yes (dig rewards)** |
| `logic/item_switchhook.h` | `EN_ANUBICE`, `EN_KAKASI2`, `EN_KAKASI3`, `EN_ZF`, `ACTOR_FLAG_SWITCHHOOKABLE` | Switch-hook grabs hookable targets → mark MM hookable actors + set the flag | no |
| `logic/item_whip.h` | `EN_ZF` | Whip grabs/pulls Lizalfos → MM Lizalfos actor | no |
| `logic/power_keg.c` | `BG_HEAVY_BLOCK` | Keg blows the silver-gauntlet heavy block → MM bombable walls / heavy objects | maybe |
| ~~`logic/snap.c` (pictograph)~~ | — | **DELETED** — MM owns the Pictograph Box natively (`src/code/z_snap.c`, `z_parameter.c` PICTO_BOX_STATE, the 0xF8 textbox). The whole NEI pictobox (`snap.c/.h`, `picto_box.c`, `picto_message.cpp`) is gone from this repo. | done |
| `mm_bottles_behavior.h` | `EN_ICE_HONO` | Bottle catches blue fire → MM blue-fire actor (if present) | n/a |
| `helpers/mailbox_actor.c` (+`.h`), `helpers/mushroom_spot_actor.c` (+`.h`) | `EN_LIGHTBOX` | — **both slated for deletion** (postman / mushroom cleanup) | — |

## Actor-flags (already native in MM — no action)
`ACTOR_FLAG_ATTENTION_ENABLED`, `ACTOR_FLAG_TALK`, `ACTOR_FLAG_HOOKSHOT_ATTACHED`,
`ACTOR_FLAG_DRAW_CULLING_DISABLED`, `ACTOR_FLAG_UPDATE_CULLING_DISABLED` all exist in
MM's `z64actor.h`. Only `ACTOR_FLAG_SWITCHHOOKABLE` is compat-defined (as `0`, inert).
