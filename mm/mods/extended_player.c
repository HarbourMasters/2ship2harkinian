/**
 * extended_player.c - Extended player item action system
 *
 * Maps custom ITEM_xxx values to PLAYER_IA_xxx actions, and each custom
 * PLAYER_IA_xxx to its model group / update func / init func.
 *
 * MM-PORT BOUNDARY: every custom item is one row in sNeiItems[] below. To port
 * an item to MM (2ship), copy its logic module + its single descriptor row.
 * The four ExtPlayer_* getters are thin lookups over that table with a vanilla
 * fallback, so there is exactly one place that describes an item's engine glue.
 *
 * Items whose action is a *vanilla* PLAYER_IA (bow combos, swords, medallions ->
 * spells, Chateau Romani -> blue potion) or is chosen dynamically (SW97 arrows ->
 * bow/slingshot by age) are NOT table rows: they alias vanilla behavior and are
 * resolved in ExtPlayer_GetItemAction before the table lookup.
 */

#include "extended_player.h"
#include "extended_inventory.h" // SLOT_*, AGE_REQ_*, NeiItem (Skijer's NEI)
#include "z64.h"
#include "mods/items/custom_items.h"
#include "assets/2s2h_assets.h" // custom item icon OTR paths (Skijer's NEI)
#include "soh/Enhancements/randomizer/randomizerTypes.h" // RG_* (Skijer's NEI)
#include "soh/Enhancements/randomizer/draw.h"            // Randomizer_Draw* (Skijer's NEI)
#include <stddef.h> // NULL

// Skijer's NEI hookshot overhaul: does the currently-held item draw the OoT hookshot model in the
// hand? True for the Switch Hook and for the OoT Hookshot/Longshot/Ultrashot chain; false for the
// MM-native Clawshot (which keeps MM's own hookshot model). z_player_lib.c calls this to pick the
// held right-hand DL — it can't see the NEI item-action defines that live in extended_player.h.
u8 Nei_HeldItemUsesOotHookshotModel(Player* player) {
    if (player == NULL) {
        return 0;
    }
    // Selection is by the HELD ITEM (the kaleido hookshot-cell wheel picks it): the OoT chain items
    // and the Switch Hook wear the OoT model; MM's own ITEM_HOOKSHOT (= the Clawshot) keeps MM's model.
    switch (player->heldItemId) {
        case ITEM_SWITCH_HOOK:
        case ITEM_HOOKSHOT_OOT:
        case ITEM_LONGSHOT_OOT:
            return 1;
        default:
            return 0;
    }
}

// Skijer's NEI: which hookshot-actor variant is firing, resolved from the HELD ITEM (the kaleido
// hookshot-cell wheel is the selector — ITEM_HOOKSHOT is MM's native item and IS the Clawshot):
//   0 Hookshot (OoT L1)   1 Longshot (OoT L2)   2 Ultrashot (OoT L3)
//   3 Clawshot (MM item)  4 Switch Hook (own slot; swaps positions, no damage)
// z_arms_hook.c calls this — it can't see the NEI item ids.
u8 Nei_ArmsHookVariant(Player* player) {
    extern u8 Nei_HookshotVariant(void);
    if (player != NULL) {
        switch (player->heldItemId) {
            case ITEM_SWITCH_HOOK:
                return 4; // NEI_HOOK_VARIANT_SWITCHHOOK
            case ITEM_HOOKSHOT: // MM-native hookshot item = the Clawshot (once owned)
                return Nei_Save()->clawshotOwned ? 3 : 1; // unowned: behave like the vanilla MM hookshot
            case ITEM_HOOKSHOT_OOT:
                return 0;
            case ITEM_LONGSHOT_OOT:
                return (Nei_Save()->ootHookshotLevel >= 3) ? 2 : 1;
            default:
                break;
        }
    }
    return Nei_HookshotVariant();
}

// External reference to vanilla arrays
extern int8_t sItemItemActions[];
extern uint8_t sActionModelGroups[];
extern s32 (*sItemActionUpdateFuncs[])(Player* this, PlayState* play);
extern void (*sItemActionInitFuncs[])(PlayState* play, Player* this);

// External vanilla functions used by custom items
extern s32 func_8083485C(Player* this, PlayState* play);
// MM's real sword-swing upper action (func_80830B88 || func_80830DF0). This is the verified 1:1
// equivalent of SoH's Player_UpperAction_Sword (which the elemental rods used) — see
// z_player.c:14582. The old `Player_UpperAction_Sword` stub was a no-op (return 0), so the rods
// had no sword upper action; the real melee swing is enabled by classifying the rod IAs as a
// melee weapon in Player_MeleeWeaponFromIA (z_player_lib.c). Skijer's NEI
extern s32 Player_UpperAction_1(Player* this, PlayState* play);
extern void Player_InitDefaultIA(PlayState* play, Player* this);

// External custom item upper action functions
extern s32 Player_UpperAction_Beetle(Player* this, PlayState* play);
extern s32 Player_UpperAction_OotBoomerang(Player* this, PlayState* play); // Skijer's NEI: OoT boomerang 1:1
extern void Player_InitOotBoomerangIA(PlayState* play, Player* this);      // Skijer's NEI: OoT boomerang 1:1
extern s32 Player_UpperAction_BombArrows(Player* this, PlayState* play);
extern s32 Player_UpperAction_CaneOfSomaria(Player* this, PlayState* play);
extern s32 Player_UpperAction_DekuLeaf(Player* this, PlayState* play);
extern s32 Player_UpperAction_Shovel(Player* this, PlayState* play);
extern s32 Player_UpperAction_SwitchHook(Player* this, PlayState* play);

// External custom item init functions (not declared in custom_items.h)
extern void Player_InitHyliasGraceIA(PlayState* play, Player* this);
extern void Player_InitZonaiPermafrostIA(PlayState* play, Player* this);
extern void Player_InitSwitchHookIA(PlayState* play, Player* this);
extern void Player_InitMogmaMittsIA(PlayState* play, Player* this);
extern void Player_InitWhipIA(PlayState* play, Player* this);
extern void Player_InitDominionRodIA(PlayState* play, Player* this);
extern void Player_InitTimeGateIA(PlayState* play, Player* this);
extern void Player_InitMinishCapIA(PlayState* play, Player* this);
extern void Player_InitLanternIA(PlayState* play, Player* this);
extern void Player_InitPokeballIA(PlayState* play, Player* this);

// NOTE (NEI slingshot pass): Sw97_PreferBow is kept only for reference-parity with the SoH
// fork; since the arrows/bullets split (arrows = bow wheel, bullets = slingshot wheel) the
// SW97 arrow items always resolve to bow IAs and this predicate is no longer consulted.
#if 0
// Decide whether an SW97 elemental arrow item should fire from bow or slingshot.
// Default: bow for adult, slingshot for child (vanilla age-based weapon).
// With BowSlingshotAmmoFix + TimelessEquipment both enabled, child can own and
// fire the bow — so prefer bow whenever the bow is in inventory regardless of age.
static s32 Sw97_PreferBow(void) {
    s32 useBow = LINK_IS_ADULT;
    if (CVarGetInteger(CVAR_ENHANCEMENT("BowSlingshotAmmoFix"), 0) &&
        CVarGetInteger(CVAR_CHEAT("TimelessEquipment"), 0)) {
        useBow = (INV_CONTENT(ITEM_BOW) == ITEM_BOW);
    }
    return useBow;
}
#endif

// Real OoT slingshot IA glue (defined in z_player.c, same TU — forward declarations).
s32 Player_UpperAction_6(Player* this, PlayState* play);       // bow/slingshot idle upper action
void Player_InitSlingshotIA(PlayState* play, Player* this);    // OoT Player_InitBowOrSlingshotIA, slingshot arm

// ---------------------------------------------------------------------------
// Custom item descriptor table — single source of truth for engine glue.
//
//   item       ITEM_xxx, or NEI_NO_ITEM for IA-only rows (no inventory item).
//   ia         PLAYER_IA_xxx (unique per row).
//   modelGroup PLAYER_MODELGROUP_xxx.
//   slot       page-2 inventory slot (SLOT_*), or NEI_NO_SLOT.
//   ageReq     AGE_REQ_* (AGE_REQ_NONE for slotless rows).
//   icon       page-2 icon texture (NULL = dynamic, getter handles it).
//   updateFn   upper-action update (func_8083485C = generic "no special update").
//   initFn     action init (Player_InitDefaultIA = generic).
//
// Only items whose IA is a *custom* action live here; vanilla-IA aliases are
// resolved separately in ExtPlayer_GetItemAction. Skijer's NEI
// ---------------------------------------------------------------------------
// Skijer's NEI — extra columns: drawFunc (rando GI 3D model), rg (RandomizerGet,
// NEI_NO_RG if non-uniform / none), and name strings (relocated from
// customItemMessages[] so each item lives in one row). Roc's Feather Skijer keeps
// rg=NEI_NO_RG: ITEM_ROCS_FEATHER_SKIJER maps to two RGs (progressive + vanilla),
// so its give/draw/name stay on the old per-RG path.
static const NeiItem sNeiItems[] = {
    // item                          ia                              modelGroup                  slot                       ageReq         icon                                       update                          init                       drawFunc                          rg                       nameEn / nameFr / nameDe
    { ITEM_ROCS_FEATHER_SKIJER,      PLAYER_IA_ROCS_FEATHER_SKIJER,  PLAYER_MODELGROUP_DEFAULT,  SLOT_ROCS,                 AGE_REQ_NONE,  (void*)gItemIconRocsFeatherTex,            func_8083485C,                  Player_InitDefaultIA,      NULL,                             NEI_NO_RG,               NULL, NULL, NULL },
    { ITEM_ROCS_CAPE,                PLAYER_IA_ROCS_CAPE,            PLAYER_MODELGROUP_DEFAULT,  SLOT_ROCS,                 AGE_REQ_NONE,  (void*)gItemIconRocsCapeTex,               func_8083485C,                  Player_InitDefaultIA,      NULL,                             RG_ROCS_CAPE,
      "You got %rRoc's Cape%w!&This magical cape enhances&your jumping ability.^Now you can perform a&%gdouble jump%w "
      "in midair.&Press %y\xA1%w again while&jumping to go higher!",
      "Vous obtenez la %rCape de Roc%w!&Cette cape magique améliore&vos capacités de saut.^Vous pouvez maintenant "
      "effectuer&un %gdouble saut%w en l'air.&Appuyez sur %y\xA1%w en sautant&pour aller plus haut!",
      "Du hast %rRocs Umhang%w erhalten!&Dieser magische Umhang&verbessert deine Sprungkraft.^Du kannst nun "
      "einen&%gDoppelsprung%w in der Luft&ausführen. Drücke %y\xA1%w&erneut während du springst!" },
    { ITEM_DESIRE_SENSOR,            PLAYER_IA_DESIRE_SENSOR,        PLAYER_MODELGROUP_DEFAULT,  SLOT_DESIRE_SENSOR,        AGE_REQ_NONE,  (void*)gItemIconDesireSensorTex,           func_8083485C,                  Player_InitDefaultIA,      Randomizer_DrawDesireSensor,      RG_DESIRE_SENSOR,
      "You got the %pDesire Sensor%w!&A cursed artifact that reveals&hidden treasures... at a cost.^Press %y\xA1%w to "
      "activate.&%rCosts 3 hearts%w per use!^%g(Randomizer only)%w:&%yGolden sparkles%w = Major items&remain in this "
      "area.&%rGanondorf laugh%w = Nothing left.",
      "Vous obtenez le %pDétecteur de Désir%w!&Un artefact maudit qui révèle&les trésors cachés... à un prix.^Appuyez "
      "sur %y\xA1%w pour activer.&%rCoûte 3 cœurs%w par utilisation!^%g(Randomizer uniquement)%w:&%yÉtincelles dorées%w "
      "= Objets majeurs&restent dans cette zone.&%rRire de Ganondorf%w = Plus rien.",
      "Du hast den %pWunschdetektor%w!&Ein verfluchtes Artefakt das&verborgene Schätze enthüllt...&für einen "
      "Preis.^Drücke %y\xA1%w zum Aktivieren.&%rKostet 3 Herzen%w pro Nutzung!^%g(Nur im Randomizer)%w:&%yGoldene "
      "Funken%w = Wichtige Items&sind noch in diesem Gebiet.&%rGanondorfs Lachen%w = Nichts mehr da." },
    { ITEM_HYLIAS_GRACE,             PLAYER_IA_HYLIAS_GRACE,         PLAYER_MODELGROUP_DEFAULT,  SLOT_HYLIAS_GRACE,         AGE_REQ_NONE,  (void*)gItemIconHyliaGraceTex,             func_8083485C,                  Player_InitHyliasGraceIA,  Randomizer_DrawHyliaGrace,        RG_HYLIAS_GRACE,
      "You got %pHylia's Grace%w!&A divine blessing that transforms&you into a %cfairy%w for 10 seconds.^Press %y\xA1%w "
      "to activate&(requires a %rFairy in a Bottle%w).^%yA%w = Ascend  %yB%w = Descend&%yL%w = Sprint&1 minute "
      "cooldown after use.",
      "Vous obtenez la %pGrâce d'Hylia%w!&Une bénédiction divine qui vous&transforme en %cfée%w pendant 10 "
      "secondes.^Appuyez sur %y\xA1%w pour activer&(nécessite une %rFée en Bouteille%w).^%yA%w = Monter  %yB%w = "
      "Descendre&%yL%w = Sprint&1 minute de recharge après utilisation.",
      "Du hast %pHylias Gnade%w erhalten!&Ein göttlicher Segen der dich&für 10 Sekunden in eine %cFee%w "
      "verwandelt.^Drücke %y\xA1%w zum Aktivieren&(benötigt eine %rFee in einer Flasche%w).^%yA%w = Aufsteigen  %yB%w = "
      "Absteigen&%yL%w = Sprinten&1 Minute Abklingzeit nach Nutzung." },
    { ITEM_ZONAI_PERMAFROST,         PLAYER_IA_ZONAI_PERMAFROST,     PLAYER_MODELGROUP_DEFAULT,  SLOT_ZONAI_PERMAFROST,     AGE_REQ_NONE,  (void*)gItemIconZonaiPermafrostTex,        func_8083485C,                  Player_InitZonaiPermafrostIA, Randomizer_DrawZonaiPermafrost, RG_ZONAI_PERMAFROST,
      "You got %cZonai Permafrost%w!&Ancient Zonai technology that&freezes the flow of time itself.^Press %y\xA1%w to "
      "cast the spell.&%rAll enemies%w, %ypuzzle elements%w,&and even the %cday/night cycle%w&freeze for %g10 "
      "seconds%w!^Costs %g12 Magic%w per use.&Move freely while time is stopped.",
      "Vous obtenez %cPermafrost Soneau%w!&Technologie ancienne des Soneau&qui gèle le flux du temps.^Appuyez sur "
      "%y\xA1%w pour lancer&le sort. %rTous les ennemis%w,&%yéléments de puzzle%w, et même&le %ccycle jour/nuit%w "
      "gèlent&pendant %g10 secondes%w!^Coûte %g12 Magie%w par utilisation.&Bougez librement pendant que&le temps est "
      "arrêté.",
      "Du hast %cSonau Permafrost%w!&Uralte Sonau-Technologie die&den Fluss der Zeit einfriert.^Drücke %y\xA1%w um den "
      "Zauber&zu wirken. %rAlle Feinde%w,&%yRätsel-Elemente%w, und sogar&der %cTag/Nacht-Zyklus%w frieren&für %g10 "
      "Sekunden%w ein!^Kostet %g12 Magie%w pro Nutzung.&Bewege dich frei während die&Zeit angehalten ist." },
    { ITEM_DEMISE_DESTRUCTION,       PLAYER_IA_DEMISE_DESTRUCTION,   PLAYER_MODELGROUP_DEFAULT,  SLOT_DEMISE_DESTRUCTION,   AGE_REQ_NONE,  (void*)gItemIconDemiseDestructionTex,      func_8083485C,                  Player_InitDemiseDestructionIA, Randomizer_DrawDemiseDestruction, RG_DEMISE_DESTRUCTION,
      "You got %rDemise Destruction%w!&The dark power of the Demon King&Demise, sealed in this artifact.^Press %y\xA1%w "
      "to unleash a&devastating %rlightning explosion%w&that damages all enemies in&a %glarge radius%w around "
      "you.^%rHigh Magic cost%w.&Best saved for emergencies!&The ground itself trembles...",
      "Vous obtenez %rDestruction de l'Avatar%w!&Le pouvoir sombre du Roi Démon&Avatar, scellé dans cet "
      "artefact.^Appuyez sur %y\xA1%w pour déchaîner&une %rexplosion de foudre%w&dévastatrice qui blesse tous "
      "les&ennemis dans un %glarge rayon%w.^%rCoût élevé en Magie%w.&À garder pour les urgences!&La terre elle-même "
      "tremble...",
      "Du hast %rTodbringer Zerstörung%w!&Die dunkle Macht des Dämonenkönigs&Todbringer, versiegelt in "
      "diesem&Artefakt.^Drücke %y\xA1%w um eine verheerende&%rBlitz-Explosion%w zu entfesseln&die alle Feinde in "
      "einem&%ggroßen Radius%w um dich trifft.^%rHohe Magiekosten%w.&Am besten für Notfälle aufheben!&Der Boden selbst "
      "bebt..." },
    { ITEM_DEKU_LEAF,                PLAYER_IA_DEKU_LEAF,            PLAYER_MODELGROUP_DEFAULT,  SLOT_DEKU_LEAF,            AGE_REQ_CHILD, (void*)gItemIconDekuLeafTex,               Player_UpperAction_DekuLeaf,    Player_InitDefaultIA,      Randomizer_DrawDekuLeaf,          RG_DEKU_LEAF,
      "You got the %gDeku Leaf%w!&A giant leaf with powers&of the wind.^%yIn the air%w: Use it to glide&slowly and "
      "cover great&distances. Consumes magic.^%yOn the ground%w: Creates a gust&of wind that pushes objects&and "
      "enemies forward.",
      "Vous obtenez la %gFeuille Mojo%w!&Une feuille géante dotée&des pouvoirs du vent.^%yDans les airs%w: "
      "Planez&lentement sur de grandes&distances. Consomme de la magie.^%yAu sol%w: Crée une rafale&qui pousse les "
      "objets&et ennemis vers l'avant.",
      "Du hast das %gDeku-Blatt%w erhalten!&Ein Riesenblatt mit der&Kraft des Windes.^%yIn der Luft%w: Gleite "
      "langsam&und überbrücke große&Distanzen. Verbraucht Magie.^%yAm Boden%w: Erzeugt einen&Windstoß der Objekte "
      "und&Feinde nach vorne schiebt." },
    { ITEM_SWITCH_HOOK,              PLAYER_IA_SWITCH_HOOK,          PLAYER_MODELGROUP_HOOKSHOT, SLOT_SWITCH_HOOK,          AGE_REQ_CHILD, (void*)gItemIconSwitchHookTex,             Player_UpperAction_SwitchHook,  Player_InitSwitchHookIA,   Randomizer_DrawSwitchHook,        RG_SWITCH_HOOK,
      "You got the %cSwitch Hook%w!&A magical hook that swaps&your position with targets.^Hold %y\xA1%w to aim,&release "
      "to fire.&%c\xA5%w = First-person mode^Swap places with pots, crates,&and certain enemies!&Non-swappable targets "
      "take damage.",
      "Vous obtenez le %cCrochet Échange%w!&Un crochet magique qui échange&votre position avec les cibles.^Maintenez "
      "%y\xA1%w pour viser,&relâchez pour tirer.&%c\xA5%w = Première personne^Échangez avec des pots, caisses,&et "
      "certains ennemis!&Les cibles non-échangeables subissent des dégâts.",
      "Du hast den %cWechselhaken%w!&Ein magischer Haken der deine&Position mit Zielen tauscht.^Halte %y\xA1%w zum "
      "Zielen,&lass los zum Feuern.&%c\xA5%w = Erste-Person^Tausche Plätze mit Töpfen, Kisten&und bestimmten "
      "Feinden!&Nicht-tauschbare Ziele nehmen Schaden." },
    { ITEM_MOGMA_MITTS,              PLAYER_IA_MOGMA_MITTS,          PLAYER_MODELGROUP_DEFAULT,  SLOT_MOGMA_MITTS,          AGE_REQ_NONE,  (void*)gItemIconMogmaMittsTex,             func_8083485C,                  Player_InitMogmaMittsIA,   Randomizer_DrawMogmaMitts,        RG_MOGMA_MITTS,
      "You got the %yMogma Mitts%w!&Claws of the underground.&Climb any wall! Uses %gMagic%w.",
      "Vous obtenez les %yGants Mogma%w!&Griffes souterraines.&Grimpez partout! Utilise de la %gMagie%w.",
      "Du hast die %yMogma-Klauen%w erhalten!&Klauen aus dem Untergrund.&Klettere überall! Verbraucht %gMagie%w." },
    { ITEM_GUST_JAR,                 PLAYER_IA_GUST_JAR,             PLAYER_MODELGROUP_DEFAULT,  SLOT_GUST_JAR,             AGE_REQ_CHILD, (void*)gItemIconGustJarTex,                func_8083485C,                  Player_InitGustJarIA,      Randomizer_DrawGustJar,           RG_GUST_JAR,
      "You got the %gGust Jar%w!&A vessel containing&ancient winds.^%ySuction mode%w: Hold %y\xA1%w&to absorb objects, "
      "enemies&and environmental elements.^%yCapture mode%w: Absorb fire,&ice or electricity to store&special "
      "ammunition.^%yShoot mode%w: Release %y\xA1%w to&fire what you captured.&%c\xA5%w = First-person mode",
      "Vous obtenez le %gPot Magique%w!&Un récipient contenant&des vents anciens.^%yMode aspiration%w: Maintenez "
      "%y\xA1%w&pour absorber objets, ennemis&et éléments environnementaux.^%yMode capture%w: Absorbez feu,&glace ou "
      "électricité comme&munition spéciale.^%yMode tir%w: Relâchez %y\xA1%w pour&tirer ce que vous avez "
      "capturé.&%c\xA5%w = Première personne",
      "Du hast den %gMagischen Krug%w!&Ein Gefäß mit uralten&Winden.^%yAnsaugmodus%w: Halte %y\xA1%w&um Objekte, Feinde "
      "und&Umgebungselemente anzusaugen.^%yFangmodus%w: Sauge Feuer,&Eis oder Elektrizität auf&als spezielle "
      "Munition.^%ySchussmodus%w: Lass %y\xA1%w los&um das Gefangene zu feuern.&%c\xA5%w = Erste-Person" },
    { ITEM_BALL_AND_CHAIN,           PLAYER_IA_BALL_AND_CHAIN,       PLAYER_MODELGROUP_DEFAULT,  SLOT_BALL_AND_CHAIN,       AGE_REQ_ADULT, (void*)gItemIconBallAndChainTex,           func_8083485C,                  Player_InitBallAndChainIA, Randomizer_DrawBallAndChain,      RG_BALL_AND_CHAIN,
      "You got the %yBall and Chain%w!&A heavy weapon from the&snow palace.^Hold %y\xA1%w to charge,&release to "
      "throw.&Crush ice and enemies!^With %g\xA4%w it homes in&on the enemy automatically.&Breaks %rRed "
      "Ice%w!^%rNote%w: Your speed is reduced&while it's equipped.",
      "Vous obtenez le %yBoulet%w!&Une arme lourde du palais&des neiges.^Maintenez %y\xA1%w pour charger,&relâchez pour "
      "lancer.&Écrasez glace et ennemis!^Avec %g\xA4%w il suit&automatiquement l'ennemi.&Brise la %rGlace "
      "Rouge%w!^%rNote%w: Votre vitesse est réduite&tant qu'il est équipé.",
      "Du hast die %yKettenkugel%w!&Eine schwere Waffe aus dem&Schneepalast.^Halte %y\xA1%w zum Aufladen,&lass los zum "
      "Werfen.&Zerschmettere Eis und Feinde!^Mit %g\xA4%w verfolgt sie&automatisch den Feind.&Zerbricht %rRotes "
      "Eis%w!^%rHinweis%w: Deine Geschwindigkeit&ist reduziert während sie&ausgerüstet ist." },
    { ITEM_WHIP,                     PLAYER_IA_WHIP,                 PLAYER_MODELGROUP_DEFAULT,  SLOT_WHIP,                 AGE_REQ_NONE,  (void*)gItemIconWhipTex,                   func_8083485C,                  Player_InitWhipIA,         Randomizer_DrawWhip,              RG_WHIP,
      "You got the %yWhip%w!&A versatile tool for combat&and exploration.^Press %y\xA1%w to lash forward.&It latches "
      "onto beams and bars&for pendulum swinging.^%ySwinging%w: Use the stick to&control the pendulum.&Release to "
      "launch with momentum!^%yCombat%w: Paralyze enemies,&pull shields, and disarm.&Also grabs items!",
      "Vous obtenez le %yFouet%w!&Un outil polyvalent pour le combat&et l'exploration.^Appuyez sur %y\xA1%w pour "
      "fouetter.&S'accroche aux poutres et barres&pour se balancer en pendule.^%yBalancement%w: Utilisez le stick&pour "
      "contrôler le pendule.&Relâchez pour vous lancer!^%yCombat%w: Paralysez les ennemis,&tirez les boucliers et "
      "désarmez.&Attrape aussi des objets!",
      "Du hast die %yPeitsche%w!&Ein vielseitiges Werkzeug für&Kampf und Erkundung.^Drücke %y\xA1%w zum Schlagen.&Hakt "
      "sich an Balken und Stangen&zum Pendelschwingen ein.^%ySchwingen%w: Nutze den Stick um&das Pendel zu "
      "steuern.&Lass los für Schwung-Start!^%yKampf%w: Lähme Feinde,&ziehe Schilde weg und entwaffne.&Greift auch "
      "Items!" },
    { ITEM_SPINNER,                  PLAYER_IA_SPINNER,              PLAYER_MODELGROUP_DEFAULT,  SLOT_SPINNER,              AGE_REQ_NONE,  (void*)gItemIconSpinnerTex,                func_8083485C,                  Player_InitSpinnerIA,      Randomizer_DrawSpinner,           RG_SPINNER,
      "You got the %ySpinner%w!&Ancient technology from the&desert sands.^Press %y\xA1%w to ride it&and glide around. "
      "Use it to&cross great distances.^With %g\xA4%w you perform&a homing attack towards&the enemy. Breaks rocks!",
      "Vous obtenez la %yToupie%w!&Technologie ancienne des&sables du désert.^Appuyez sur %y\xA1%w pour monter&et "
      "glisser. Utilisez-la pour&traverser de grandes distances.^Avec %g\xA4%w vous effectuez&une attaque guidée "
      "vers&l'ennemi. Brise les rochers!",
      "Du hast den %yKreisel%w!&Uralte Technologie aus dem&Wüstensand.^Drücke %y\xA1%w um aufzusteigen&und zu gleiten. "
      "Überbrücke&große Distanzen damit.^Mit %g\xA4%w führst du einen&Verfolgungs-Angriff auf&den Feind aus. "
      "Zerbricht Felsen!" },
    { ITEM_CANE_OF_SOMARIA,          PLAYER_IA_CANE_OF_SOMARIA,      PLAYER_MODELGROUP_DEFAULT,  SLOT_CANE_OF_SOMARIA,      AGE_REQ_NONE,  (void*)gItemIconCaneOfSomariaTex,          Player_UpperAction_CaneOfSomaria, Player_InitCaneOfSomariaIA, Randomizer_DrawCaneOfSomaria,   RG_CANE_OF_SOMARIA,
      "You got the %rCane of Somaria%w!&A wand that creates magical&blocks out of thin air.^Press %y\xA1%w to swing and "
      "create&a %rmagical block%w. Up to %g3&blocks%w can exist at once.^The %roldest block%w is destroyed&when you "
      "create a 4th.^Use them to activate switches,&block enemies, or as&platforms to reach heights.",
      "Vous obtenez la %rCanne de Somaria%w!&Une baguette qui crée des&blocs magiques de nulle part.^Appuyez sur "
      "%y\xA1%w pour brandir&et créer un %rbloc magique%w.&Jusqu'à %g3 blocs%w peuvent exister.^Le %rbloc le plus "
      "ancien%w est&détruit quand vous en créez un 4e.^Utilisez-les pour activer des&interrupteurs, bloquer des "
      "ennemis,&ou comme plateformes.",
      "Du hast den %rStab von Somaria%w!&Ein Stab der magische Blöcke&aus dem Nichts erschafft.^Drücke %y\xA1%w zum "
      "Schwingen&und erschaffe einen %rmagischen&Block%w. Bis zu %g3 Blöcke%w können&gleichzeitig existieren.^Der "
      "%rälteste Block%w wird zerstört&wenn du einen 4. erschaffst.^Nutze sie für Schalter, um Feinde&zu blockieren, "
      "oder als Plattform." },
    { ITEM_DOMINION_ROD,             PLAYER_IA_DOMINION_ROD,         PLAYER_MODELGROUP_DEFAULT,  SLOT_DOMINION_ROD,         AGE_REQ_NONE,  (void*)gItemIconDominionRodTex,            func_8083485C,                  Player_InitDominionRodIA,  Randomizer_DrawDominionRod,       RG_DOMINION_ROD,
      "You got the %pDominion Rod%w!&An ancient artifact that can&possess and control enemies.^Press %y\xA1%w to fire a "
      "golden orb.&It can possess: %rBeamos%w,&%yArmos%w, and %cAnubis%w.^Once possessed, the enemy will&%gmimic your "
      "movements%w!&Walk to make it walk,&attack to make it attack.^Uses %gMagic%w while controlling.",
      "Vous obtenez la %pBaguette des Animes%w!&Un artefact ancien qui peut&posséder et contrôler les ennemis.^Appuyez "
      "sur %y\xA1%w pour tirer un&orbe doré. Il peut posséder:&%rBeamos%w, %yArmos%w et %cAnubis%w.^Une fois possédé, "
      "l'ennemi va&%gimiter vos mouvements%w!&Marchez pour le faire marcher,&attaquez pour le faire attaquer.^Utilise "
      "de la %gMagie%w pendant&le contrôle.",
      "Du hast den %pKopierstab%w!&Ein uraltes Artefakt das Feinde&besitzen und kontrollieren kann.^Drücke %y\xA1%w um "
      "einen goldenen Orb&zu feuern. Er kann besitzen:&%rBeamos%w, %yArmos%w und %cAnubis%w.^Einmal besessen, wird der "
      "Feind&%gdeine Bewegungen imitieren%w!&Laufe um ihn laufen zu lassen,&greife an um ihn angreifen zu "
      "lassen.^Verbraucht %gMagie%w beim Kontrollieren." },
    { ITEM_TIME_GATE,                PLAYER_IA_TIME_GATE,            PLAYER_MODELGROUP_DEFAULT,  SLOT_TIME_GATE,            AGE_REQ_NONE,  (void*)gItemIconTimeGateTex,               func_8083485C,                  Player_InitTimeGateIA,     Randomizer_DrawTimeGate,          RG_TIME_GATE,
      "You got the %cTime Gate%w!&A portable door through the ages,&the power of the Temple of Time&in your "
      "hands.^Press %y\xA1%w to activate.&A prompt will ask: %g\"Travel&through time?\"%w^Select %yYes%w to switch "
      "between&%rChild%w and %gAdult%w Link&anywhere in the world!^Costs %g48 Magic%w per use.",
      "Vous obtenez la %cPorte du Temps%w!&Une porte portable à travers les&âges, le pouvoir du Temple du Temps&dans "
      "vos mains.^Appuyez sur %y\xA1%w pour activer.&Une question apparaît: %g\"Voyager&dans le temps?\"%w^Sélectionnez "
      "%yOui%w pour passer&entre Link %rEnfant%w et %gAdulte%w&n'importe où!^Coûte %g48 Magie%w par utilisation.",
      "Du hast das %cZeittor%w!&Eine tragbare Tür durch die Zeit,&die Macht des Zeitturms in&deinen Händen.^Drücke "
      "%y\xA1%w zum Aktivieren.&Eine Frage erscheint: %g\"Durch&die Zeit reisen?\"%w^Wähle %yJa%w um zwischen&%rKind%w "
      "und %gErwachsenem%w Link&überall zu wechseln!^Kostet %g48 Magie%w pro Nutzung." },
    { ITEM_BOMB_ARROWS,              PLAYER_IA_BOMB_ARROWS,          PLAYER_MODELGROUP_DEFAULT,  SLOT_BOMB_ARROWS,          AGE_REQ_ADULT, (void*)gItemIconBombArrowsTex,             Player_UpperAction_BombArrows,  Player_InitBombArrowsIA,   Randomizer_DrawBombArrows,        RG_BOMB_ARROWS,
      "You got %rBomb Arrows%w!&An explosive combination.^Requires %yArrows%w and %rBombs%w.&Use %y\xA1%w to enter "
      "first-person&mode and aim.^The arrow explodes on impact.&Consumes %y1 arrow%w + %r1 bomb%w&per shot.",
      "Vous obtenez les %rFlèches-Bombes%w!&Une combinaison explosive.^Nécessite des %yFlèches%w et "
      "%rBombes%w.&Utilisez %y\xA1%w pour entrer en&première personne et viser.^La flèche explose à l'impact.&Consomme "
      "%y1 flèche%w + %r1 bombe%w&par tir.",
      "Du hast %rBombenpfeile%w!&Eine explosive Kombination.^Benötigt %yPfeile%w und %rBomben%w.&Benutze %y\xA1%w für "
      "Erste-Person&Modus und zielen.^Der Pfeil explodiert beim&Aufprall. Verbraucht %y1 Pfeil%w&+ %r1 Bombe%w pro "
      "Schuss." },
    // Rods use the BGS (two-handed) model group + sword mechanics for charge attacks.
    { ITEM_ROD_FIRE,                 PLAYER_IA_ROD_FIRE,             PLAYER_MODELGROUP_BGS,      SLOT_FIRE_ROD,             AGE_REQ_NONE,  (void*)gItemIconFireRodTex,                Player_UpperAction_1,           Player_InitFireRodIA,      Randomizer_DrawFireRod,           RG_FIRE_ROD,
      "You got the %rFire Rod%w!&A magical weapon that channels&the power of fire.^%yBasic attacks%w:&Slash = 3 "
      "fireballs&Stab = 1 fireball&Jump = Flamethrower down^%ySpecial attacks%w:&Spin = Expanding fire wave&Hold "
      "%y\xA1%w = Charge attack&%c\xA5%w = First-person mode^%rWarning%w: Without magic, the&fire will burn YOU. Make "
      "sure&you have enough magic!",
      "Vous obtenez la %rBaguette de Feu%w!&Une arme magique qui canalise&le pouvoir du feu.^%yAttaques de "
      "base%w:&Taille = 3 boules de feu&Estoc = 1 boule de feu&Saut = Lance-flammes^%yAttaques spéciales%w:&Tourbillon "
      "= Vague de feu&Maintenez %y\xA1%w = Charge&%c\xA5%w = Première personne^%rAttention%w: Sans magie, le feu&VOUS "
      "brûlera. Assurez-vous&d'avoir assez de magie!",
      "Du hast den %rFeuerstab%w!&Eine magische Waffe mit der&Kraft des Feuers.^%yBasisangriffe%w:&Hieb = 3 "
      "Feuerbälle&Stoß = 1 Feuerball&Sprung = Flammenwerfer^%ySpezialangriffe%w:&Wirbelattacke = Feuerwelle&Halte "
      "%y\xA1%w = Aufladen&%c\xA5%w = Erste-Person^%rWarnung%w: Ohne Magie verbrennt&das Feuer DICH. Achte auf&genug "
      "Magie!" },
    { ITEM_ROD_ICE,                  PLAYER_IA_ROD_ICE,              PLAYER_MODELGROUP_BGS,      SLOT_ICE_ROD,              AGE_REQ_NONE,  (void*)gItemIconIceRodTex,                 Player_UpperAction_1,           Player_InitIceRodIA,       Randomizer_DrawIceRod,            RG_ICE_ROD,
      "You got the %bIce Rod%w!&A magical weapon that channels&the power of ice.^%yBasic attacks%w:&Slash = 3 ice "
      "projectiles&Stab = 1 ice projectile&Jump = Freezing blast down^%ySpecial attacks%w:&Spin = Expanding ice "
      "wave&Hold %y\xA1%w = Charge attack&%c\xA5%w = First-person mode^%rWarning%w: Without magic, the&ice will freeze "
      "YOU. Make sure&you have enough magic!",
      "Vous obtenez la %bBaguette de Glace%w!&Une arme magique qui canalise&le pouvoir de la glace.^%yAttaques de "
      "base%w:&Taille = 3 projectiles de glace&Estoc = 1 projectile de glace&Saut = Souffle glacial^%yAttaques "
      "spéciales%w:&Tourbillon = Vague de glace&Maintenez %y\xA1%w = Charge&%c\xA5%w = Première "
      "personne^%rAttention%w: Sans magie, la glace&VOUS gèlera. Assurez-vous&d'avoir assez de magie!",
      "Du hast den %bEisstab%w!&Eine magische Waffe mit der&Kraft des Eises.^%yBasisangriffe%w:&Hieb = 3 "
      "Eisprojektile&Stoß = 1 Eisprojektil&Sprung = Eisstrahl^%ySpezialangriffe%w:&Wirbelattacke = Eiswelle&Halte "
      "%y\xA1%w = Aufladen&%c\xA5%w = Erste-Person^%rWarnung%w: Ohne Magie friert&das Eis DICH ein. Achte auf&genug "
      "Magie!" },
    { ITEM_ROD_LIGHT,                PLAYER_IA_ROD_LIGHT,            PLAYER_MODELGROUP_BGS,      SLOT_LIGHT_ROD,            AGE_REQ_NONE,  (void*)gItemIconLightRodTex,               Player_UpperAction_1,           Player_InitLightRodIA,     Randomizer_DrawLightRod,          RG_LIGHT_ROD,
      "You got the %yLight Rod%w!&A magical weapon that channels&the power of lightning.^%yBasic attacks%w:&Slash = 3 "
      "lightning bolts&Stab = 1 lightning bolt&Jump = Electric discharge^%ySpecial attacks%w:&Spin = Expanding "
      "electric wave&Hold %y\xA1%w = Charge attack&%c\xA5%w = First-person mode^%rWarning%w: Without magic, "
      "the&lightning will shock YOU.&Make sure you have enough magic!",
      "Vous obtenez la %yBaguette de Lumière%w!&Une arme magique qui canalise&le pouvoir de la foudre.^%yAttaques de "
      "base%w:&Taille = 3 éclairs en éventail&Estoc = 1 éclair direct&Saut = Décharge électrique^%yAttaques "
      "spéciales%w:&Tourbillon = Vague électrique&Maintenez %y\xA1%w = Charge&%c\xA5%w = Première "
      "personne^%rAttention%w: Sans magie, la foudre&VOUS électrocutera. Assurez-vous&d'avoir assez de magie!",
      "Du hast den %yLichtstab%w!&Eine magische Waffe mit der&Kraft des Blitzes.^%yBasisangriffe%w:&Hieb = 3 Blitze im "
      "Bogen&Stoß = 1 direkter Blitz&Sprung = Elektrische Entladung^%ySpezialangriffe%w:&Wirbelattacke = "
      "Elektrowelle&Halte %y\xA1%w = Aufladen&%c\xA5%w = Erste-Person^%rWarnung%w: Ohne Magie trifft&der Blitz DICH. "
      "Achte auf&genug Magie!" },
    { ITEM_BEETLE,                   PLAYER_IA_BEETLE,               PLAYER_MODELGROUP_DEFAULT,  SLOT_BEETLE,               AGE_REQ_ADULT, (void*)gItemIconBeetleTex,                 Player_UpperAction_Beetle,      Player_InitBeetleIA,       Randomizer_DrawBeetle,            RG_BEETLE,
      "You got the %gBeetle%w!&A remote-controlled mechanical&insect from ancient times.^%y\xA1%w = Launch "
      "beetle&%yAnalog Stick%w = Steer flight&%y\xA1%w again = Recall beetle&%y\xA0%w = Speed boost^The camera follows "
      "the beetle.&Use it to grab distant items,&hit switches, and scout ahead!",
      "Vous obtenez le %gScarabée%w!&Un insecte mécanique télécommandé&des temps anciens.^%y\xA1%w = Lancer le "
      "scarabée&%yStick Analogique%w = Diriger le vol&%y\xA1%w à nouveau = Rappeler&%y\xA0%w = Accélération^La caméra "
      "suit le scarabée.&Utilisez-le pour attraper des objets,&activer des interrupteurs et explorer!",
      "Du hast den %gKäfer%w erhalten!&Ein ferngesteuertes mechanisches&Insekt aus alter Zeit.^%y\xA1%w = Käfer "
      "starten&%yAnalog-Stick%w = Flug steuern&%y\xA1%w erneut = Käfer zurückrufen&%y\xA0%w = Geschwindigkeitsschub^Die "
      "Kamera folgt dem Käfer.&Nutze ihn um Items zu holen,&Schalter zu treffen und voraus zu spähen!" },
    { ITEM_SHOVEL,                   PLAYER_IA_SHOVEL,               PLAYER_MODELGROUP_DEFAULT,  SLOT_SHOVEL,               AGE_REQ_NONE,  (void*)gItemIconShovelTex,                 Player_UpperAction_Shovel,      Player_InitDefaultIA,      Randomizer_DrawShovel,            RG_SHOVEL,
      "You got the %yShovel%w!&A reliable tool for&excavation.^Use %y\xA1%w on soft soil&to dig and find "
      "hidden&treasures.^It can also reveal secret&%gGrottos%w and damage&buried enemies!",
      "Vous obtenez la %yPelle%w!&Un outil fiable pour&l'excavation.^Utilisez %y\xA1%w sur terre&meuble pour creuser "
      "et&trouver des trésors cachés.^Elle peut aussi révéler des&%gGrottes secrètes%w et blesser&les ennemis "
      "enterrés!",
      "Du hast die %ySchaufel%w!&Ein zuverlässiges Werkzeug&zum Graben.^Benutze %y\xA1%w auf weichem&Boden um zu graben "
      "und&verborgene Schätze zu finden.^Sie kann auch geheime&%gGrotten%w aufdecken und&vergrabene Feinde verletzen!" },
    { ITEM_MINISH_CAP,               PLAYER_IA_MINISH_CAP,           PLAYER_MODELGROUP_DEFAULT,  SLOT_MINISH_CAP,           AGE_REQ_CHILD, (void*)gItemIconMinishCapTex,              func_8083485C,                  Player_InitMinishCapIA,    Randomizer_DrawMinishCap,         RG_MINISH_CAP,
      "You got %pThe Minish Cap%w!&Fast travel between pod soils.",
      "Vous obtenez %pPending Item 1%w!&Cet objet n'est pas encore implémenté.",
      "Du hast %pThe Minish Cap%w!&Schnellreise zwischen Pod Soils." },
    // Lantern: icon is dynamic (chosen by fire type) -> NULL, getter handles it. Skijer's NEI
    { ITEM_LANTERN,                  PLAYER_IA_LANTERN,              PLAYER_MODELGROUP_DEFAULT,  SLOT_LANTERN,              AGE_REQ_NONE,  NULL,                                      func_8083485C,                  Player_InitLanternIA,      Randomizer_DrawLantern,           RG_LANTERN,
      "You got the %yLantern%w!&Catch fire from torches and&use it to light your way!",
      "Vous obtenez la %yLanterne%w!&Capturez le feu des torches et&utilisez-le pour éclairer votre chemin!",
      "Du hast die %yLaterne%w erhalten!&Fang Feuer von Fackeln und&nutze es um deinen Weg zu erleuchten!" },
    { ITEM_POKEBALL,                 PLAYER_IA_POKEBALL,             PLAYER_MODELGROUP_DEFAULT,  SLOT_POKEBALL,             AGE_REQ_NONE,  (void*)gItemIconPokeballTex,               func_8083485C,                  Player_InitPokeballIA,     Randomizer_DrawPokeball,          RG_POKEBALL,
      "You got the %yPoké Ball%w!&Use it to give orders to&a transformed Pikachu."
      "^%y\x9F%w combo  %y\xA0%w Thunder Jolt&Stick+%y\x9F%w/%y\xA0%w: smash / special&%y\xA2%w crouch  %y\xA3%w bubble shield&%y\xA1%w-buttons: special items",
      "Vous obtenez la %yPoké Ball%w!&Donnez des ordres à un&Pikachu transformé."
      "^%y\x9F%w combo  %y\xA0%w Tonnerre&Stick+%y\x9F%w/%y\xA0%w: smash / spécial&%y\xA2%w accroupi  %y\xA3%w bouclier&%y\xA1%w: objets spéciaux",
      "Du hast den %yPokéball%w erhalten!&Damit gibst du einem&verwandelten Pikachu Befehle."
      "^%y\x9F%w Combo  %y\xA0%w Donner-Schock&Stick+%y\x9F%w/%y\xA0%w: Smash / Special&%y\xA2%w Hocken  %y\xA3%w Blasen-Schild&%y\xA1%w-Tasten: Special-Items" },
    // IA-only: reserved action with no inventory item (default behavior).
    { NEI_NO_ITEM,                   PLAYER_IA_UNUSED_5B,            PLAYER_MODELGROUP_DEFAULT,  NEI_NO_SLOT,               AGE_REQ_NONE,  NULL,                                      func_8083485C,                  Player_InitDefaultIA,      NULL,                             NEI_NO_RG,               NULL, NULL, NULL },
    // Bottle with Magic Mushroom — bottle behavior (drop on B-swing via vanilla path). Give stays on old path (bottle-loop before the switch).
    { ITEM_BOTTLE_WITH_MAGIC_MUSHROOM, PLAYER_IA_BOTTLE_MAGIC_MUSHROOM, PLAYER_MODELGROUP_DEFAULT, NEI_NO_SLOT,            AGE_REQ_NONE,  NULL,                                      func_8083485C,                  Player_InitDefaultIA,      Randomizer_DrawBottleWithMagicMushroom, RG_BOTTLE_WITH_MAGIC_MUSHROOM,
      "You got a %gBottle with Magic Mushroom%w!&A fragrant Termina mushroom plucked&by the keen nose of the Mask of Scents.^Stored in an empty bottle.&Drop it later for unknown effects -&or simply admire the catch.",
      "Vous obtenez une %gFiole avec Champignon Magique%w!&Un champignon parfumé de Termina,&flairé par le Masque des Odeurs.^Stocké dans une fiole vide.&À déposer plus tard pour des effets&inconnus - ou à contempler.",
      "Du hast eine %gFlasche mit Zauberpilz%w!&Ein duftender Termina-Pilz, geschnüffelt&von der Geruchsmaske.^In einer leeren Flasche aufbewahrt.&Lass ihn später fallen für unbekannte&Effekte - oder bewundere ihn." },

    // MM bottle-content custom items (Bottle Randomizer, Skijer's NEI). Standalone custom items —
    // icon is dynamic (mm.o2r, resolved in ExtInv_GetItemIcon), behavior dispatched from
    // mm_bottles_behavior when used. Generic no-op IA + no get-item model yet (placeholder),
    // not a rando item yet (NEI_NO_RG). Stored directly in SLOT_BOTTLE_* by the wheel. (Chateau
    // Romani 0xB6 + Magic Mushroom 0xDD already exist and keep their own rows.)

    // Bottle Randomizer extra items: Net + Bottomless Bottle (occupy SLOT_BOTTLE_3/4). Icons from
    // soh.otr (icon_item_custom). The empty Bottomless Bottle behaves as a bottle via the IA alias
    // in ExtPlayer_GetItemAction (PLAYER_IA_BOTTLE); when filled, the slot holds the content id.
    { ITEM_NET,                  PLAYER_IA_NET,                     PLAYER_MODELGROUP_DEFAULT, NEI_NO_SLOT, AGE_REQ_NONE, (void*)gItemIconNetTex, func_8083485C, Player_InitDefaultIA, NULL, NEI_NO_RG,
      "You got the %gNet%w!&Spin to scoop things in a wider radius.", NULL, NULL },
    { ITEM_BOTTOMLESS_BOTTLE,    PLAYER_IA_BOTTOMLESS_BOTTLE,       PLAYER_MODELGROUP_DEFAULT, NEI_NO_SLOT, AGE_REQ_NONE, (void*)gItemIconBottomlessBottleTex, func_8083485C, Player_InitDefaultIA, NULL, NEI_NO_RG,
      "You got the %gBottomless Bottle%w!&Its contents multiply with use.", NULL, NULL },

};

#define NEI_ITEMS_COUNT (sizeof(sNeiItems) / sizeof(sNeiItems[0]))

// Skijer's NEI
const NeiItem* Nei_FindByItem(int32_t item) {
    for (size_t i = 0; i < NEI_ITEMS_COUNT; i++) {
        if (sNeiItems[i].item != NEI_NO_ITEM && sNeiItems[i].item == item) {
            return &sNeiItems[i];
        }
    }
    return NULL;
}

// NEI debug: grant every custom item to its inventory slot so the extended-inventory kaleido
// pages have something to show. Gated by gMods.CustomItems.GiveAll (see CustomItems_Update).
void ExtInv_DebugGiveAll(void) {
    extern void Nei_SetOwnedItem(unsigned char slot, unsigned char v);
    for (size_t i = 0; i < NEI_ITEMS_COUNT; i++) {
        u8 slot = sNeiItems[i].slot;
        if (sNeiItems[i].item != NEI_NO_ITEM && slot >= 24 && slot < 72) {
            Nei_SetOwnedItem(slot, (unsigned char)sNeiItems[i].item);
        }
    }
}

// Skijer's NEI
const NeiItem* Nei_FindBySlot(uint8_t slot) {
    if (slot == NEI_NO_SLOT) {
        return NULL;
    }
    for (size_t i = 0; i < NEI_ITEMS_COUNT; i++) {
        if (sNeiItems[i].slot == slot) {
            return &sNeiItems[i];
        }
    }
    return NULL;
}

// Skijer's NEI
const NeiItem* Nei_FindByRg(int16_t rg) {
    if (rg == NEI_NO_RG) {
        return NULL;
    }
    for (size_t i = 0; i < NEI_ITEMS_COUNT; i++) {
        if (sNeiItems[i].rg == rg) {
            return &sNeiItems[i];
        }
    }
    return NULL;
}

static const NeiItem* ExtPlayer_FindByIA(int32_t itemAction) {
    for (size_t i = 0; i < NEI_ITEMS_COUNT; i++) {
        if (sNeiItems[i].ia == itemAction) {
            return &sNeiItems[i];
        }
    }
    return NULL;
}

/**
 * Get the PLAYER_IA_xxx value for a given ITEM_xxx value.
 */
int8_t ExtPlayer_GetItemAction(int32_t item) {
    // Handle special cases first
    if (item >= ITEM_NONE_FE) {
        return PLAYER_IA_NONE;
    }
    if (item == ITEM_LAST_USED) {
        return PLAYER_IA_SWORD_CS;
    }
    if (item == ITEM_FISHING_POLE) {
        return PLAYER_IA_FISHING_POLE;
    }

    // Vanilla-IA aliases: custom items that behave as an existing vanilla action.
    // (Their model group / update / init come from the vanilla arrays, so they are
    // intentionally NOT table rows.)
    switch (item) {
        // Bow combos and swords (originally in the expanded vanilla array).
        case ITEM_BOW_ARROW_FIRE:
            return PLAYER_IA_BOW_FIRE;
        case ITEM_BOW_ARROW_ICE:
            return PLAYER_IA_BOW_ICE;
        case ITEM_BOW_ARROW_LIGHT:
            return PLAYER_IA_BOW_LIGHT;
        case ITEM_SWORD_KOKIRI:
            return PLAYER_IA_SWORD_KOKIRI;
        case ITEM_SWORD_MASTER:
            return PLAYER_IA_SWORD_MASTER;
        case ITEM_SWORD_BGS:
            return PLAYER_IA_SWORD_BIGGORON;

        // Chateau Romani (bottle item - drink to activate infinite magic)
        case ITEM_CHATEAU_ROMANI:
            return PLAYER_IA_BOTTLE_POTION_BLUE;

        // Bottle Randomizer: the EMPTY Bottomless Bottle behaves as an empty bottle (so the vanilla
        // catch action triggers); when filled, SLOT_BOTTLE_4 holds the real content id instead, so
        // this alias only applies to the empty state. Kept as an alias (not a table row's ia) so
        // ExtPlayer_FindByIA(PLAYER_IA_BOTTLE) does NOT shadow normal bottles. The row still supplies
        // the icon/name. Skijer's NEI
        case ITEM_BOTTOMLESS_BOTTLE:
            return PLAYER_IA_BOTTLE;

        // Net: wields 1:1 like the Kokiri Sword (all MM sword melee via the native IA — normal
        // slashes AND the spin attack). Net identity is kept via heldItemId == ITEM_NET (NOT this
        // IA), so z_player_lib special-cases it to draw the net model instead of the sword (closed
        // fist + net DL on the hand bone) and capture at the DL instead of dealing damage. Alias
        // (not a row ia) so it doesn't shadow the real sword; the row supplies icon/name. Skijer's NEI
        case ITEM_NET:
            return PLAYER_IA_SWORD_KOKIRI;

        // SW97 Medallion spells (quest medallions → spell IAs)
        case ITEM_MEDALLION_FOREST:
            return PLAYER_IA_MAGIC_SPELL_15;
        case ITEM_MEDALLION_SPIRIT:
            return PLAYER_IA_MAGIC_SPELL_16;
        case ITEM_MEDALLION_SHADOW:
            return PLAYER_IA_MAGIC_SPELL_17;
        case ITEM_MEDALLION_WATER:
            return PLAYER_IA_FARORES_WIND;
        case ITEM_MEDALLION_LIGHT:
            return PLAYER_IA_NAYRUS_LOVE;
        case ITEM_MEDALLION_FIRE:
            return PLAYER_IA_DINS_FIRE;

        // OoT page-0 items (Skijer's NEI MM port). Behavior quickfixes: reuse FUNCTIONAL MM IAs
        // where they map cleanly; spells share the medallion routes (same casters).
        case ITEM_DINS_FIRE:
            return PLAYER_IA_DINS_FIRE; // same route as ITEM_MEDALLION_FIRE
        case ITEM_FARORES_WIND:
            return PLAYER_IA_FARORES_WIND; // same route as ITEM_MEDALLION_WATER
        case ITEM_NAYRUS_LOVE:
            return PLAYER_IA_NAYRUS_LOVE; // same route as ITEM_MEDALLION_LIGHT
        case ITEM_HOOKSHOT_OOT:
        case ITEM_LONGSHOT_OOT:
            return PLAYER_IA_HOOKSHOT; // MM's native hookshot — fully functional today
        // Skijer's NEI switchhook rework: the Switch Hook now IS the hookshot (real arms_hook aim/
        // anim/model). It's differentiated only by heldItemId in z_arms_hook.c (swap-on-hit) and by a
        // dist-1 reach. Routing it here avoids the janky custom PLAYER_IA_SWITCH_HOOK action (which had
        // no player action of its own, so it fell back to a boomerang throw pose and never aimed). Its
        // inventory icon/name/slot still come from its sNeiItems row (looked up by itemId, not IA).
        case ITEM_SWITCH_HOOK:
            return PLAYER_IA_HOOKSHOT;
        case ITEM_FAIRY_SLINGSHOT:
            return PLAYER_IA_SLINGSHOT; // real OoT slingshot IA: fires ARROW_TYPE_SLINGSHOT on NEI seed ammo
        case ITEM_BOOMERANG:
            return PLAYER_IA_BOOMERANG; // real OoT boomerang IA (0x62): 1:1 human throw/catch chain
        case ITEM_ROCS_FEATHER:
            return PLAYER_IA_ROCS_FEATHER_SKIJER; // ship-vanilla feather: same functional jump route
        case ITEM_HAMMER:
            return PLAYER_IA_HAMMER; // real Megaton Hammer IA — 2H melee weapon w/ ground-pound floor smash

        // SW97 Arrow items: ALWAYS the bow (user decision 2026-07-02: elemental arrows and
        // elemental bullets coexist — arrows live on the bow wheel, bullets on the slingshot
        // wheel). fire/ice/light keep their elemental bow IA (fork parity: same magic-state
        // error gate); dark/soul/wind use the plain bow IA (MM has no PLAYER_IA_BOW_0C..0E).
        // The SW97 ARROW_TYPE override happens in func_808305BC via heldItemId.
        case ITEM_SW97_ARROW_FIRE:
            return PLAYER_IA_BOW_FIRE;
        case ITEM_SW97_ARROW_ICE:
            return PLAYER_IA_BOW_ICE;
        case ITEM_SW97_ARROW_LIGHT:
            return PLAYER_IA_BOW_LIGHT;
        case ITEM_SW97_ARROW_DARK:
        case ITEM_SW97_ARROW_SOUL:
        case ITEM_SW97_ARROW_WIND:
            return PLAYER_IA_BOW;

        // SW97 Bullet items (slingshot wheel): the Fairy Slingshot loaded with an element.
        // Same IA as the plain slingshot; the ARROW_TYPE_SEED_* override happens in
        // func_808305BC via heldItemId.
        case ITEM_SW97_BULLET_FIRE:
        case ITEM_SW97_BULLET_ICE:
        case ITEM_SW97_BULLET_LIGHT:
        case ITEM_SW97_BULLET_DARK:
        case ITEM_SW97_BULLET_SOUL:
        case ITEM_SW97_BULLET_WIND:
            return PLAYER_IA_SLINGSHOT;

        default:
            break;
    }

    // Custom items: unified NEI registry. Skijer's NEI
    const NeiItem* desc = Nei_FindByItem(item);
    if (desc != NULL) {
        return (int8_t)desc->ia;
    }

    // For vanilla items, use the original array if within bounds
    if (item < VANILLA_SITEMACTIONS_SIZE) {
        return sItemItemActions[item];
    }

    // For items in the gap (equipment, songs, quest items, etc.), return NONE
    return PLAYER_IA_NONE;
}

/**
 * Get the model group for a given PLAYER_IA_xxx value.
 */
uint8_t ExtPlayer_GetActionModelGroup(int32_t itemAction) {
    // Megaton Hammer: hold two-handed (Stage 1 uses the 2H-sword hold; Stage 2 swaps to the
    // companion held-hammer DL gLinkAdultLeftHandHoldingHammer*).
    if (itemAction == PLAYER_IA_HAMMER) {
        return PLAYER_MODELGROUP_TWO_HAND_SWORD;
    }

    // Fairy Slingshot: OoT puts PLAYER_IA_SLINGSHOT in PLAYER_MODELGROUP_BOW_SLINGSHOT (shared
    // with the bow); MM's surviving equivalent group is PLAYER_MODELGROUP_BOW. The right-hand
    // DL swap to the companion child-slingshot model happens in z_player_lib.c by heldItemAction.
    if (itemAction == PLAYER_IA_SLINGSHOT) {
        return PLAYER_MODELGROUP_BOW;
    }

    // OoT Boomerang: OoT's PLAYER_MODELGROUP_BOOMERANG = anim-type 0 + left fist holding the
    // boomerang. MM's surviving equivalent is PLAYER_MODELGROUP_ZORA_BOOMERANG (default anims,
    // LH_CLOSED fist); the boomerang mesh itself is drawn on the fist from the companion in
    // z_player_lib.c (hidden while PLAYER_STATE1_ZORA_BOOMERANG_THROWN, like OoT's DL swap).
    if (itemAction == PLAYER_IA_BOOMERANG) {
        return PLAYER_MODELGROUP_ZORA_BOOMERANG;
    }

    // Skijer's NEI: OoT Master Sword — mechanically the Gilded one-handed sword. Reuse the Gilded
    // model group (hold pose / anim-type) so the swing/hold is identical; only the drawn blade DL
    // differs (overridden by heldItemId in z_player_lib.c).
    if (itemAction == PLAYER_IA_SWORD_MASTER) {
        return sActionModelGroups[PLAYER_IA_SWORD_GILDED];
    }

    const NeiItem* desc = ExtPlayer_FindByIA(itemAction);
    if (desc != NULL) {
        return desc->modelGroup;
    }

    // For vanilla item actions, use the original array if within bounds
    if (itemAction < VANILLA_PLAYER_IA_COUNT) {
        return sActionModelGroups[itemAction];
    }

    return PLAYER_MODELGROUP_DEFAULT;
}

/**
 * Get the update function for a given PLAYER_IA_xxx value.
 */
ItemActionUpdateFunc ExtPlayer_GetItemActionUpdateFunc(int32_t itemAction) {
    // Fairy Slingshot: same upper action as the bow (OoT: sItemActionUpdateFuncs[PLAYER_IA_SLINGSHOT]
    // == func_8083501C == MM's Player_UpperAction_6).
    if (itemAction == PLAYER_IA_SLINGSHOT) {
        return Player_UpperAction_6;
    }

    // OoT Boomerang: 1:1 port of OoT's func_80835800 dispatcher (item_oot_boomerang.c).
    if (itemAction == PLAYER_IA_BOOMERANG) {
        return Player_UpperAction_OotBoomerang;
    }

    // Skijer's NEI: OoT Master Sword — reuse the Gilded sword's upper action (identical swing).
    if (itemAction == PLAYER_IA_SWORD_MASTER) {
        return sItemActionUpdateFuncs[PLAYER_IA_SWORD_GILDED];
    }

    const NeiItem* desc = ExtPlayer_FindByIA(itemAction);
    if (desc != NULL) {
        return desc->updateFn;
    }

    // For vanilla item actions, use the original array if within bounds
    if (itemAction < VANILLA_PLAYER_IA_COUNT) {
        return sItemActionUpdateFuncs[itemAction];
    }

    return func_8083485C;
}

/**
 * Get the init function for a given PLAYER_IA_xxx value.
 */
ItemActionInitFunc ExtPlayer_GetItemActionInitFunc(int32_t itemAction) {
    // Fairy Slingshot: OoT Player_InitBowOrSlingshotIA slingshot arm (unk_860 = -2 → MM unk_B28 = -4,
    // see Player_InitSlingshotIA in z_player.c).
    if (itemAction == PLAYER_IA_SLINGSHOT) {
        return Player_InitSlingshotIA;
    }

    // OoT Boomerang: OoT Player_InitBoomerangIA — raises USING_BOOMERANG (MM bit-identical
    // USING_ZORA_BOOMERANG), which also feeds MM's boom-wait stance hook func_8082EF20.
    if (itemAction == PLAYER_IA_BOOMERANG) {
        return Player_InitOotBoomerangIA;
    }

    // Skijer's NEI: OoT Master Sword — reuse the Gilded sword's init.
    if (itemAction == PLAYER_IA_SWORD_MASTER) {
        return sItemActionInitFuncs[PLAYER_IA_SWORD_GILDED];
    }

    const NeiItem* desc = ExtPlayer_FindByIA(itemAction);
    if (desc != NULL) {
        return desc->initFn;
    }

    // For vanilla item actions, use the original array if within bounds
    if (itemAction < VANILLA_PLAYER_IA_COUNT) {
        return sItemActionInitFuncs[itemAction];
    }

    return Player_InitDefaultIA;
}
