#include "ActorBehavior.h"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipUtils.h"

extern "C" {
#include "variables.h"
}

void ApplyHookshotHint(u16* textId, bool* loadFromMessageTable) {
    std::string msg = LOCALIZED(
        "I overheard those Pirates talk about treasure in %y{{location}}%w that can help you reach the unreachable!",
        "J'ai entendu des Pirates parler d'un trésor à %y{{location}}%w qui pourrait t'aider à atteindre "
        "l'inaccessible!",
        "Ich habe die weiblichen Piraten über einen Schatz %y{{location}}%w reden hören, mit dem du das Unerreichbare "
        "erreichen kannst!"
        "\x12"
        "Wie?\n"
        "Ich habe nur gelauscht! Sieh mich nicht so an!",
        "TODO_JAPANESE", "TODO_SPANISH");

    RandoCheckId randoCheckId = Rando::FindItemPlacement(RI_HOOKSHOT);
    // TODO HATO: SCENE LOCALIZATION
    CustomMessage::Replace(&msg, "{{location}}", Ship_GetSceneName(Rando::StaticData::Checks[randoCheckId].sceneId));

    CustomMessage::Entry entry = {
        .nextMessageID = (u16)0xFFFF,
        .msg = msg,
    };
    CustomMessage::ReplaceColorChars(&entry.msg);
    CustomMessage::ReplaceSpecialChars(&entry.msg);
    CustomMessage::LoadCustomMessageIntoFont(entry);
    *loadFromMessageTable = false;
}

void Rando::ActorBehavior::InitEnZowBehavior() {
    bool shouldRegister = IS_RANDO && RANDO_SAVE_OPTIONS[RO_HINTS_HOOKSHOT];

    COND_ID_HOOK(OnOpenText, 0x12FD, shouldRegister, ApplyHookshotHint);
    COND_ID_HOOK(OnOpenText, 0x12FA, shouldRegister, ApplyHookshotHint);
    COND_ID_HOOK(OnOpenText, 0x1301, shouldRegister, ApplyHookshotHint);
    COND_ID_HOOK(OnOpenText, 0x12FF, shouldRegister, ApplyHookshotHint);
    COND_ID_HOOK(OnOpenText, 0x12F8, shouldRegister, ApplyHookshotHint);
    COND_ID_HOOK(OnOpenText, 0x12F3, shouldRegister, ApplyHookshotHint);
}
