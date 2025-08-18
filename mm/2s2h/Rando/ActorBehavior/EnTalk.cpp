#include "ActorBehavior.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
}

void ApplyRemainsHint(u16* textId, bool* loadFromMessageTable) {
    static int remainsHintIndex = 0;

    if (remainsHintIndex > 4) {
        remainsHintIndex = 0;
    }

    u8 icon = 0xFE;
    std::string msg;
    RandoItemId randoItemId = RI_NONE;

    if (remainsHintIndex == 0) {
        msg = LOCALIZED("        Witcher Wanted:\nThese monsters are tormenting the local townfolk, will pay good "
                        "money for their remains.",
                        "        Chasseur recherché:\nCes monstres tourmentent les habitants du village, bonne "
                        "récompense pour leurs restes.",
                        "TODO_GERMAN", "TODO_JAPANESE", "TODO_SPANISH");
    } else {
        msg = LOCALIZED("         %g{{boss}}%w:\nLast seen in near %y{{location}}%w.",
                        "         %g{{boss}}%w:\nAperçu dernièrement près de %y{{location}}%w.", "TODO_GERMAN",
                        "TODO_JAPANESE", "TODO_SPANISH");

        switch (remainsHintIndex) {
            case 1:
                CustomMessage::Replace(&msg, "{{boss}}", " Odolwa");
                randoItemId = RI_REMAINS_ODOLWA;
                break;
            case 2:
                CustomMessage::Replace(&msg, "{{boss}}", "  Goht");
                randoItemId = RI_REMAINS_GOHT;
                break;
            case 3:
                CustomMessage::Replace(&msg, "{{boss}}", "  Gyorg");
                randoItemId = RI_REMAINS_GYORG;
                break;
            case 4:
                CustomMessage::Replace(&msg, "{{boss}}", "Twinmold");
                randoItemId = RI_REMAINS_TWINMOLD;
                break;
        }

        icon = Rando::StaticData::GetIconForZMessage(randoItemId);
        RandoCheckId randoCheckId = Rando::FindItemPlacement(randoItemId);
        // TODO HATO: SCENE LOCALIZATION
        CustomMessage::Replace(&msg, "{{location}}",
                               Ship_GetSceneName(Rando::StaticData::Checks[randoCheckId].sceneId));
    }

    CustomMessage::Entry entry = {
        .icon = icon,
        .nextMessageID = remainsHintIndex >= 4 ? (u16)0xFFFF : (u16)0x1C06,
        .msg = msg,
    };

    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
    remainsHintIndex++;
}

void Rando::ActorBehavior::InitEnTalkBehavior() {
    // "Recruiting Soldiers..." Posters around Clock Town
    COND_ID_HOOK(OnOpenText, 0x1C06, IS_RANDO && RANDO_SAVE_OPTIONS[RO_HINTS_BOSS_REMAINS], ApplyRemainsHint);
}
