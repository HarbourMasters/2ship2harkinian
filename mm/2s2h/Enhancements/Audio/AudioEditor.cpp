#include "AudioEditor.h"

#include <map>
#include <set>
#include <string>
#include <libultraship/bridge/consolevariablebridge.h>
#include <functions.h>
#include <sequence.h>
// #include "../randomizer/3drando/random.hpp"
#include "../../BenPort.h"
#include <ship/utils/StringHelper.h>
#include "../../BenGui/UIWidgets.hpp"
#include "../../BenGui/BenMenu.h"
#include "../../BenGui/BenGui.hpp"
#include "AudioCollection.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include <random>

extern "C" Vec3f gZeroVec3f;
extern "C" f32 gSfxDefaultFreqAndVolScale;
extern "C" s8 gSfxDefaultReverb;

using namespace UIWidgets;

static WidgetInfo lowHpAlarm;
static WidgetInfo muteCarpenterSfx;
static WidgetInfo childGoronCry;
static WidgetInfo tatlCall;
static WidgetInfo enemyProx;
static WidgetInfo displaySeqName;
static WidgetInfo ovlDuration;
static WidgetInfo voicePitch;
static WidgetInfo randoMusicOnSceneChange;
static WidgetInfo randomAudioOnSeedGen;

namespace BenGui {
extern std::shared_ptr<BenMenu> mBenMenu;
}

// Authentic sequence counts
// used to ensure we have enough to shuffle
#define SEQ_COUNT_BGM_WORLD 30
#define SEQ_COUNT_BGM_BATTLE 6
#define SEQ_COUNT_FANFARE 15
#define SEQ_COUNT_OCARINA 12
#define SEQ_COUNT_NOSHUFFLE 6
#define SEQ_COUNT_BGM_EVENT 17
#define SEQ_COUNT_INSTRUMENT 6
#define SEQ_COUNT_SFX 57
#define SEQ_COUNT_VOICE 108

size_t AuthenticCountBySequenceType(SeqType type) {
    return AudioCollection::Instance->CountSequencesByType(type);
}

#define CVAR_AUDIO(var) CVAR_PREFIX_AUDIO "." var

// Grabs the current BGM sequence ID and replays it
// which will lookup the proper override, or reset back to vanilla
void ReplayCurrentBGM() {
    u16 curSeqId = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
    // TODO: replace with Audio_StartSeq when the macro is shared
    // The fade time and audio player flags will always be 0 in the case of replaying the BGM, so they are not set here
    // AudioSeq_QueueSeqCmd(0x00000000 | curSeqId);
    SEQCMD_PLAY_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 0, curSeqId);
}

// Attempt to update the BGM if it matches the current sequence that is being played
// The seqKey that is passed in should be the vanilla ID, not the override ID
void UpdateCurrentBGM(u16 seqKey, SeqType seqType) {
    if (seqType != SEQ_BGM_WORLD) {
        return;
    }

    u16 curSeqId = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
    if (curSeqId == seqKey) {
        ReplayCurrentBGM();
    }
}

void RandomizeGroup(SeqType type) {
    std::vector<u16> values;

    // An empty IncludedSequences set means that the AudioEditor window has never been drawn
    if (AudioCollection::Instance->GetIncludedSequences().empty()) {
        AudioCollection::Instance->InitializeShufflePool();
    }

    // use a while loop to add duplicates if we don't have enough included sequences
    while (values.size() < AudioCollection::Instance->CountSequencesByType(type)) {
        size_t initialSize = values.size();
        for (const auto& seqData : AudioCollection::Instance->GetIncludedSequences()) {
            if (seqData->category & type && seqData->canBeUsedAsReplacement) {
                values.push_back(seqData->sequenceId);
            }
        }

        // if we didn't add any new values, return early to prevent an infinite loop
        if (values.size() == initialSize) {
            return;
        }
    }

    if (values.empty()) {
        return;
    }
    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(values.begin(), values.end(), g);
    for (const auto& [seqId, seqData] : AudioCollection::Instance->GetAllSequences()) {
        const std::string cvarKey = AudioCollection::Instance->GetCvarKey(seqData.sfxKey);
        const std::string cvarLockKey = AudioCollection::Instance->GetCvarLockKey(seqData.sfxKey);
        // don't randomize locked entries
        if ((seqData.category & type) && CVarGetInteger(cvarLockKey.c_str(), 0) == 0) {
            // Only save authentic sequence CVars
            if ((((seqData.category & SEQ_BGM_CUSTOM) || seqData.category == SEQ_FANFARE) &&
                 seqData.sequenceId >= MAX_AUTHENTIC_SEQID) ||
                seqData.canBeReplaced == false) {
                continue;
            }
            if (!values.empty()) {
                const int randomValue = values.back();
                CVarSetInteger(cvarKey.c_str(), randomValue);
                values.pop_back();
            }
        }
    }
}

void ResetGroup(const std::map<u16, SequenceInfo>& map, SeqType type) {
    for (const auto& [defaultValue, seqData] : map) {
        if (seqData.category == type) {
            // Only save authentic sequence CVars
            if (seqData.category == SEQ_FANFARE && defaultValue >= MAX_AUTHENTIC_SEQID) {
                continue;
            }
            const std::string cvarKey = AudioCollection::Instance->GetCvarKey(seqData.sfxKey);
            const std::string cvarLockKey = AudioCollection::Instance->GetCvarLockKey(seqData.sfxKey);
            if (CVarGetInteger(cvarLockKey.c_str(), 0) == 0) {
                CVarClear(cvarKey.c_str());
            }
        }
    }
}

void LockGroup(const std::map<u16, SequenceInfo>& map, SeqType type) {
    for (const auto& [defaultValue, seqData] : map) {
        if (seqData.category == type) {
            // Only save authentic sequence CVars
            if (seqData.category == SEQ_FANFARE && defaultValue >= MAX_AUTHENTIC_SEQID) {
                continue;
            }
            const std::string cvarKey = AudioCollection::Instance->GetCvarKey(seqData.sfxKey);
            const std::string cvarLockKey = AudioCollection::Instance->GetCvarLockKey(seqData.sfxKey);
            CVarSetInteger(cvarLockKey.c_str(), 1);
        }
    }
}

void UnlockGroup(const std::map<u16, SequenceInfo>& map, SeqType type) {
    for (const auto& [defaultValue, seqData] : map) {
        if (seqData.category == type) {
            // Only save authentic sequence CVars
            if (seqData.category == SEQ_FANFARE && defaultValue >= MAX_AUTHENTIC_SEQID) {
                continue;
            }
            const std::string cvarKey = AudioCollection::Instance->GetCvarKey(seqData.sfxKey);
            const std::string cvarLockKey = AudioCollection::Instance->GetCvarLockKey(seqData.sfxKey);
            CVarSetInteger(cvarLockKey.c_str(), 0);
        }
    }
}
extern "C" void Audio_ForceRestorePreviousBgm(void);
extern "C" void PreviewSequence(u16 seqId);
void DrawPreviewButton(uint16_t sequenceId, std::string sfxKey, SeqType sequenceType) {
    const std::string cvarKey = AudioCollection::Instance->GetCvarKey(sfxKey);
    const std::string hiddenKey = "##" + cvarKey;
    const std::string stopButton = ICON_FA_STOP + hiddenKey;
    const std::string previewButton = ICON_FA_PLAY + hiddenKey;

    if (CVarGetInteger(CVAR_AUDIO("Playing"), 0) == sequenceId) {
        if (ImGui::Button(stopButton.c_str())) {
            Audio_ForceRestorePreviousBgm();
            CVarSetInteger(CVAR_AUDIO("Playing"), 0);
        }
        UIWidgets::Tooltip("Stop Preview");
    } else {
        if (ImGui::Button(previewButton.c_str())) {
            if (CVarGetInteger(CVAR_AUDIO("Playing"), 0) != 0) {
                Audio_ForceRestorePreviousBgm();
                CVarSetInteger(CVAR_AUDIO("Playing"), 0);
            } else {
                if (sequenceType == SEQ_SFX || sequenceType == SEQ_VOICE) {
                    AudioSfx_PlaySfx(sequenceId, &gZeroVec3f, 4, &gSfxDefaultFreqAndVolScale,
                                     &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                } else if (sequenceType == SEQ_INSTRUMENT) {
                    AudioOcarina_SetInstrument(sequenceId - INSTRUMENT_OFFSET);
                    AudioOcarina_SetPlaybackSong(9, 1);
                } else {
                    // TODO: Cant do both here, so have to click preview button twice
                    PreviewSequence(sequenceId);
                    CVarSetInteger(CVAR_AUDIO("Playing"), sequenceId);
                }
            }
        }
        UIWidgets::Tooltip("Play Preview");
    }
}

void Draw_SfxTab(const std::string& tabId, SeqType type) {
    const std::map<u16, SequenceInfo>& map = AudioCollection::Instance->GetAllSequences();

    const std::string hiddenTabId = "##" + tabId;
    const std::string resetAllButton = "Reset All" + hiddenTabId;
    const std::string randomizeAllButton = "Randomize All" + hiddenTabId;
    const std::string lockAllButton = "Lock All" + hiddenTabId;
    const std::string unlockAllButton = "Unlock All" + hiddenTabId;
    if (ImGui::Button(resetAllButton.c_str())) {
        auto currentBGM = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
        auto prevReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        ResetGroup(map, type);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        auto curReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        if (type == SEQ_BGM_WORLD && prevReplacement != curReplacement) {
            ReplayCurrentBGM();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(randomizeAllButton.c_str())) {
        auto currentBGM = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
        auto prevReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        RandomizeGroup(type);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        auto curReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        if (type == SEQ_BGM_WORLD && prevReplacement != curReplacement) {
            ReplayCurrentBGM();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(lockAllButton.c_str())) {
        auto currentBGM = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
        auto prevReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        LockGroup(map, type);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        auto curReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        if (type == SEQ_BGM_WORLD && prevReplacement != curReplacement) {
            ReplayCurrentBGM();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(unlockAllButton.c_str())) {
        auto currentBGM = AudioSeq_GetActiveSeqId(SEQ_PLAYER_BGM_MAIN);
        auto prevReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        UnlockGroup(map, type);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        auto curReplacement = AudioCollection::Instance->GetReplacementSequence(currentBGM);
        if (type == SEQ_BGM_WORLD && prevReplacement != curReplacement) {
            ReplayCurrentBGM();
        }
    }

    ImGui::BeginTable(tabId.c_str(), 3, ImGuiTableFlags_SizingFixedFit);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    for (const auto& [defaultValue, seqData] : map) {
        if (~(seqData.category) & type) {
            continue;
        }
        // Do not display custom sequences in the list
        if ((((seqData.category & SEQ_BGM_CUSTOM) || seqData.category == SEQ_BGM_CUSTOM_FANFARE) &&
             defaultValue >= MAX_AUTHENTIC_SEQID) ||
            seqData.canBeReplaced == false) {
            continue;
        }

        const std::string initialSfxKey = seqData.sfxKey;
        const std::string cvarKey = AudioCollection::Instance->GetCvarKey(seqData.sfxKey);
        const std::string cvarLockKey = AudioCollection::Instance->GetCvarLockKey(seqData.sfxKey);
        const std::string hiddenKey = "##" + cvarKey;
        const std::string resetButton = ICON_FA_UNDO + hiddenKey;
        const std::string randomizeButton = ICON_FA_RANDOM + hiddenKey;
        const std::string lockedButton = ICON_FA_LOCK + hiddenKey;
        const std::string unlockedButton = ICON_FA_UNLOCK + hiddenKey;
        const int currentValue = CVarGetInteger(cvarKey.c_str(), defaultValue);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", seqData.label.c_str());
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        const int initialValue = map.contains(currentValue) ? currentValue : defaultValue;
        if (ImGui::BeginCombo(hiddenKey.c_str(), map.at(initialValue).label.c_str())) {
            for (const auto& [value, seqData] : map) {
                // If excluded as a replacement sequence, don't show in other dropdowns except the effect's own
                // dropdown.
                if (~(seqData.category) & type ||
                    (!seqData.canBeUsedAsReplacement && initialSfxKey != seqData.sfxKey)) {
                    continue;
                }

                if (ImGui::Selectable(seqData.label.c_str())) {
                    CVarSetInteger(cvarKey.c_str(), value);
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                    UpdateCurrentBGM(defaultValue, type);
                }

                if (currentValue == value) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        // BENTODO ship checks for some values and passes either the replaced value or the default value. For some
        // reason, likely an edge case, the original -> replacement lookup is done twice. Passing the original value
        // here seems to work.
        DrawPreviewButton(defaultValue, seqData.sfxKey, type);
        auto locked = CVarGetInteger(cvarLockKey.c_str(), 0) == 1;
        ImGui::SameLine();
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::Button(resetButton.c_str())) {
            CVarClear(cvarKey.c_str());
            CVarClear(cvarLockKey.c_str());
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
            UpdateCurrentBGM(defaultValue, seqData.category);
        }
        UIWidgets::Tooltip("Reset to default");
        ImGui::SameLine();
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::Button(randomizeButton.c_str())) {
            std::vector<SequenceInfo*> validSequences = {};
            for (const auto seqInfo : AudioCollection::Instance->GetIncludedSequences()) {
                if (seqInfo->category & type) {
                    validSequences.push_back(seqInfo);
                }
            }

            if (validSequences.size()) {
                auto it = validSequences.begin();
                const auto& seqData = *std::next(it, rand() % validSequences.size());
                CVarSetInteger(cvarKey.c_str(), seqData->sequenceId);
                if (locked) {
                    CVarClear(cvarLockKey.c_str());
                }
                Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
                UpdateCurrentBGM(defaultValue, type);
            }
        }
        UIWidgets::Tooltip("Randomize this sound");
        ImGui::SameLine();
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::Button(locked ? lockedButton.c_str() : unlockedButton.c_str())) {
            if (locked) {
                CVarClear(cvarLockKey.c_str());
            } else {
                CVarSetInteger(cvarLockKey.c_str(), 1);
            }
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
        }
        UIWidgets::Tooltip(locked ? "Sound locked" : "Sound unlocked");
    }
    ImGui::EndTable();
}

extern "C" u16 AudioEditor_GetReplacementSeq(u16 seqId) {
    return AudioCollection::Instance->GetReplacementSequence(seqId);
}

extern "C" u16 AudioEditor_GetOriginalSeq(u16 seqId) {
    return AudioCollection::Instance->GetOriginalSequence(seqId);
}

const char* GetSequenceTypeName(SeqType type) {
    switch (type) {
        case SEQ_NOSHUFFLE:
            return "No Shuffle";
        case SEQ_BGM_WORLD:
            return "World";
        case SEQ_BGM_EVENT:
            return "Event";
        case SEQ_BGM_BATTLE:
            return "Battle";
        case SEQ_OCARINA:
            return "Ocarina";
        case SEQ_FANFARE:
            return "Fanfare";
        case SEQ_BGM_ERROR:
            return "Error";
        case SEQ_SFX:
            return "SFX";
        case SEQ_VOICE:
            return "Voice";
        case SEQ_INSTRUMENT:
            return "Instrument";
        case SEQ_BGM_CUSTOM:
            return "Custom";
        default:
            return "No Sequence Type";
    }
}

ImVec4 GetSequenceTypeColor(SeqType type) {
    switch (type) {
        case SEQ_BGM_WORLD:
            return ImVec4(0.0f, 0.2f, 0.0f, 1.0f);
        case SEQ_BGM_EVENT:
            return ImVec4(0.3f, 0.0f, 0.15f, 1.0f);
        case SEQ_BGM_BATTLE:
            return ImVec4(0.2f, 0.07f, 0.0f, 1.0f);
        case SEQ_OCARINA:
            return ImVec4(0.0f, 0.0f, 0.4f, 1.0f);
        case SEQ_FANFARE:
            return ImVec4(0.3f, 0.0f, 0.3f, 1.0f);
        case SEQ_SFX:
            return ImVec4(0.4f, 0.33f, 0.0f, 1.0f);
        case SEQ_VOICE:
            return ImVec4(0.3f, 0.42f, 0.09f, 1.0f);
        case SEQ_INSTRUMENT:
            return ImVec4(0.0f, 0.25f, 0.5f, 1.0f);
        case SEQ_BGM_CUSTOM:
            return ImVec4(0.9f, 0.0f, 0.9f, 1.0f);
        default:
            return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
}

void DrawTypeChip(SeqType type) {
    ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_Button, GetSequenceTypeColor(type));
    ImGui::SmallButton(GetSequenceTypeName(type));
    ImGui::PopStyleColor();
    ImGui::EndDisabled();
}

void AudioEditorRegisterOnSceneChangeHook() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneChange>([](int16_t sceneNum) {
        if (CVarGetInteger("gAudioEditor.RandomizeAllOnNewScene", 0)) {
            AudioEditor_RandomizeAll();
        }
    });
}

void AudioEditorRegisterOnGenerationCompletionHook() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnRandoSeedGeneration>([]() {
        if (CVarGetInteger("gAudioEditor.RandomizeAllOnRandoGen", 0)) {
            AudioEditor_RandomizeAll();
        }
    });
}

void AudioEditor::InitElement() {
    AudioEditorRegisterOnSceneChangeHook();
    AudioEditorRegisterOnGenerationCompletionHook();
}

void AudioEditor::DrawElement() {
    AudioCollection::Instance->InitializeShufflePool();

    float buttonSegments = ImGui::GetContentRegionAvail().x / 4;
    if (ImGui::Button("Randomize All Groups", ImVec2(buttonSegments, 30.0f))) {
        AudioEditor_RandomizeAll();
    }
    UIWidgets::Tooltip("Randomizes all unlocked music and sound effects across tab groups");
    ImGui::SameLine();
    if (ImGui::Button("Reset All Groups", ImVec2(buttonSegments, 30.0f))) {
        AudioEditor_ResetAll();
    }
    UIWidgets::Tooltip("Resets all unlocked music and sound effects across tab groups");
    ImGui::SameLine();
    if (ImGui::Button("Lock All Groups", ImVec2(buttonSegments, 30.0f))) {
        AudioEditor_LockAll();
    }
    UIWidgets::Tooltip("Locks all music and sound effects across tab groups");
    ImGui::SameLine();
    if (ImGui::Button("Unlock All Groups", ImVec2(buttonSegments, 30.0f))) {
        AudioEditor_UnlockAll();
    }
    UIWidgets::Tooltip("Unlocks all music and sound effects across tab groups");

    if (ImGui::BeginTabBar("SfxContextTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
        static ImVec2 cellPadding(8.0f, 8.0f);
        if (ImGui::BeginTabItem("Audio Options")) {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
            ImGui::BeginTable("Audio Options", 1, ImGuiTableFlags_SizingStretchSame);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::BeginChild("SfxOptions", ImVec2(0, -8))) {
                BenGui::mBenMenu->MenuDrawItem(lowHpAlarm, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(muteCarpenterSfx, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(childGoronCry, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(tatlCall, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(enemyProx, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(randoMusicOnSceneChange, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(randomAudioOnSeedGen, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(displaySeqName, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(ovlDuration, ImGui::GetContentRegionAvail().x, THEME_COLOR);
                BenGui::mBenMenu->MenuDrawItem(voicePitch, ImGui::GetContentRegionAvail().x, THEME_COLOR);
            }
            ImGui::EndChild();
            ImGui::EndTable();
            ImGui::PopStyleVar(1);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Background Music")) {
            Draw_SfxTab("backgroundMusic", SEQ_BGM_WORLD);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Fanfares")) {
            Draw_SfxTab("fanfares", SEQ_FANFARE);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Events")) {
            Draw_SfxTab("event", SEQ_BGM_EVENT);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Battle Music")) {
            Draw_SfxTab("battleMusic", SEQ_BGM_BATTLE);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Ocarina")) {
            Draw_SfxTab("ocarina", SEQ_OCARINA);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Instrument")) {
            Draw_SfxTab("instrument", SEQ_INSTRUMENT);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Songs")) {
            Draw_SfxTab("songs", SEQ_BGM_SONGS);
            ImGui::EndTabItem();
        }
#if 0
        if (ImGui::BeginTabItem("Sound Effects")) {
            Draw_SfxTab("sfx", SEQ_SFX);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Voices")) {
            Draw_SfxTab("voice", SEQ_VOICE);
            ImGui::EndTabItem();
#endif

        static bool excludeTabOpen = false;
        if (ImGui::BeginTabItem("Audio Shuffle Pool Management")) {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
            if (!excludeTabOpen) {
                excludeTabOpen = true;
            }

            static std::map<SeqType, bool> showType{ { SEQ_BGM_WORLD, true },  { SEQ_BGM_EVENT, true },
                                                     { SEQ_BGM_BATTLE, true }, { SEQ_OCARINA, true },
                                                     { SEQ_FANFARE, true },    { SEQ_SFX, true },
                                                     { SEQ_VOICE, true },      { SEQ_INSTRUMENT, true },
                                                     { SEQ_BGM_CUSTOM, true } };

            // make temporary sets because removing from the set we're iterating through crashes ImGui
            std::set<SequenceInfo*> seqsToInclude = {};
            std::set<SequenceInfo*> seqsToExclude = {};

            static ImGuiTextFilter sequenceSearch;
            sequenceSearch.Draw("Filter (inc,-exc)", 490.0f);
            ImGui::SameLine();
            if (ImGui::Button("Exclude All")) {
                for (auto seqInfo : AudioCollection::Instance->GetIncludedSequences()) {
                    if (sequenceSearch.PassFilter(seqInfo->label.c_str()) && showType[seqInfo->category]) {
                        seqsToExclude.insert(seqInfo);
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Include All")) {
                for (auto seqInfo : AudioCollection::Instance->GetExcludedSequences()) {
                    if (sequenceSearch.PassFilter(seqInfo->label.c_str()) && showType[seqInfo->category]) {
                        seqsToInclude.insert(seqInfo);
                    }
                }
            }

            ImGui::BeginTable("sequenceTypes", 9,
                              ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_BGM_WORLD));
            ImGui::Selectable(GetSequenceTypeName(SEQ_BGM_WORLD), &showType[SEQ_BGM_WORLD]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_BGM_EVENT));
            ImGui::Selectable(GetSequenceTypeName(SEQ_BGM_EVENT), &showType[SEQ_BGM_EVENT]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_BGM_BATTLE));
            ImGui::Selectable(GetSequenceTypeName(SEQ_BGM_BATTLE), &showType[SEQ_BGM_BATTLE]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_OCARINA));
            ImGui::Selectable(GetSequenceTypeName(SEQ_OCARINA), &showType[SEQ_OCARINA]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_FANFARE));
            ImGui::Selectable(GetSequenceTypeName(SEQ_FANFARE), &showType[SEQ_FANFARE]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_SFX));
            ImGui::Selectable(GetSequenceTypeName(SEQ_SFX), &showType[SEQ_SFX]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_VOICE));
            ImGui::Selectable(GetSequenceTypeName(SEQ_VOICE), &showType[SEQ_VOICE]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_INSTRUMENT));
            ImGui::Selectable(GetSequenceTypeName(SEQ_INSTRUMENT), &showType[SEQ_INSTRUMENT]);
            ImGui::PopStyleColor(1);

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_Header, GetSequenceTypeColor(SEQ_BGM_CUSTOM));
            ImGui::Selectable(GetSequenceTypeName(SEQ_BGM_CUSTOM), &showType[SEQ_BGM_CUSTOM]);
            ImGui::PopStyleColor(1);

            ImGui::EndTable();

            if (ImGui::BeginTable("tableAllSequences", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV)) {
                ImGui::TableSetupColumn("Included", ImGuiTableColumnFlags_WidthStretch, 200.0f);
                ImGui::TableSetupColumn("Excluded", ImGuiTableColumnFlags_WidthStretch, 200.0f);
                ImGui::TableHeadersRow();
                ImGui::TableNextRow();

                // COLUMN 1 - INCLUDED SEQUENCES
                ImGui::TableNextColumn();

                ImGui::BeginChild("ChildIncludedSequences", ImVec2(0, -8));
                for (auto seqInfo : AudioCollection::Instance->GetIncludedSequences()) {
                    if (sequenceSearch.PassFilter(seqInfo->label.c_str()) && showType[seqInfo->category]) {
                        if (ImGui::Button(std::string(ICON_FA_TIMES "##" + seqInfo->sfxKey).c_str())) {
                            seqsToExclude.insert(seqInfo);
                        }
                        ImGui::SameLine();
                        DrawPreviewButton(seqInfo->sequenceId, seqInfo->sfxKey, seqInfo->category);
                        ImGui::SameLine();
                        DrawTypeChip(seqInfo->category);
                        ImGui::SameLine();
                        ImGui::Text("%s", seqInfo->label.c_str());
                    }
                }
                ImGui::EndChild();

                // remove the sequences we added to the temp set
                for (auto seqInfo : seqsToExclude) {
                    AudioCollection::Instance->RemoveFromShufflePool(seqInfo);
                }

                // COLUMN 2 - EXCLUDED SEQUENCES
                ImGui::TableNextColumn();

                ImGui::BeginChild("ChildExcludedSequences", ImVec2(0, -8));
                for (auto seqInfo : AudioCollection::Instance->GetExcludedSequences()) {
                    if (sequenceSearch.PassFilter(seqInfo->label.c_str()) && showType[seqInfo->category]) {
                        if (ImGui::Button(std::string(ICON_FA_PLUS "##" + seqInfo->sfxKey).c_str())) {
                            seqsToInclude.insert(seqInfo);
                        }
                        ImGui::SameLine();
                        DrawPreviewButton(seqInfo->sequenceId, seqInfo->sfxKey, seqInfo->category);
                        ImGui::SameLine();
                        DrawTypeChip(seqInfo->category);
                        ImGui::SameLine();
                        ImGui::Text("%s", seqInfo->label.c_str());
                    }
                }
                ImGui::EndChild();

                // add the sequences we added to the temp set
                for (auto seqInfo : seqsToInclude) {
                    AudioCollection::Instance->AddToShufflePool(seqInfo);
                }

                ImGui::EndTable();
            }
            ImGui::PopStyleVar(1);
            ImGui::EndTabItem();
        } else {
            excludeTabOpen = false;
        }

        ImGui::EndTabBar();
    }
}

static std::array<SeqType, 8> allTypes = { SEQ_BGM_WORLD, SEQ_BGM_EVENT,  SEQ_BGM_BATTLE, SEQ_OCARINA,
                                           SEQ_FANFARE,   SEQ_INSTRUMENT, SEQ_SFX,        SEQ_VOICE };

void AudioEditor_RandomizeAll() {
    for (auto type : allTypes) {
        RandomizeGroup(type);
    }

    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    ReplayCurrentBGM();
}

void AudioEditor_RandomizeGroup(SeqType group) {
    RandomizeGroup(group);

    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    ReplayCurrentBGM();
}

void AudioEditor_ResetAll() {
    for (auto type : allTypes) {
        ResetGroup(AudioCollection::Instance->GetAllSequences(), type);
    }

    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    ReplayCurrentBGM();
}

void AudioEditor_ResetGroup(SeqType group) {
    ResetGroup(AudioCollection::Instance->GetAllSequences(), group);

    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    ReplayCurrentBGM();
}

void AudioEditor_LockAll() {
    for (auto type : allTypes) {
        LockGroup(AudioCollection::Instance->GetAllSequences(), type);
    }

    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
}

void AudioEditor_UnlockAll() {
    for (auto type : allTypes) {
        UnlockGroup(AudioCollection::Instance->GetAllSequences(), type);
    }

    Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
}

void RegisterAudioWidgets() {

    lowHpAlarm = { .name = "Mute Low HP Alarm", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    lowHpAlarm.CVar("gAudioEditor.LowHpAlarm")
        .Options(CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Mutes the beeping alarm when you are critically low on health."));
    BenGui::mBenMenu->AddSearchWidget({ lowHpAlarm, "Enhancements", "Audio Editor", "Audio Options" });

    muteCarpenterSfx = { .name = "Mute Carpenter Sounds", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    muteCarpenterSfx.CVar("gAudioEditor.MuteCarpenterSfx")
        .Options(CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Requires scene reload to take effect. Mutes the carpenter sounds coming "
                              "from the tower in South Clock Town."));
    BenGui::mBenMenu->AddSearchWidget({ muteCarpenterSfx, "Enhancements", "Audio Editor", "Audio Options" });

    childGoronCry = { .name = "Mute Crying Goron Child", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    childGoronCry.CVar("gAudioEditor.ChildGoronCry")
        .Options(CheckboxOptions().Color(THEME_COLOR).Tooltip("Mutes the crying Goron child inside Goron Shrine."));
    BenGui::mBenMenu->AddSearchWidget({ childGoronCry, "Enhancements", "Audio Editor", "Audio Options" });

    tatlCall = { .name = "Disable Tatl Call Audio", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    tatlCall.CVar("gAudioEditor.DisableTatlCallAudio")
        .Options(CheckboxOptions().Color(THEME_COLOR).Tooltip("Disables the bell audio when Tatl calls you."));
    BenGui::mBenMenu->AddSearchWidget({ tatlCall, "Enhancements", "Audio Editor", "Audio Options" });

    enemyProx = { .name = "Disable Enemy Proximity Music", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    enemyProx.CVar("gAudioEditor.EnemyBGMDisable")
        .Options(CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Disables the music change when getting close to enemies. Useful for hearing "
                              "your custom music for each scene more often."));
    BenGui::mBenMenu->AddSearchWidget({ enemyProx, "Enhancements", "Audio Editor", "Audio Options" });

    randoMusicOnSceneChange = { .name = "Randomize All Music and Sound Effects on New Scene",
                                .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    randoMusicOnSceneChange.CVar("gAudioEditor.RandomizeAllOnNewScene")
        .Options(CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Enables randomizing all unlocked music and sound effects when you enter a new scene."));
    BenGui::mBenMenu->AddSearchWidget({ randoMusicOnSceneChange, "Enhancements", "Audio Editor", "Audio Options" });

    randomAudioOnSeedGen = { .name = "Randomize All Music and Sound Effects on Randomizer Generation",
                             .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    randomAudioOnSeedGen.CVar("gAudioEditor.RandomizeAllOnRandoGen")
        .Options(CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Enables randomizing all unlocked music and sound effects when you generate a new "
                              "randomizer. Respects locks already in place."));
    BenGui::mBenMenu->AddSearchWidget({ randomAudioOnSeedGen, "Enhancements", "Audio Editor", "Audio Options" });

    displaySeqName = { .name = "Display Sequence Name on Overlay", .type = WidgetType::WIDGET_CVAR_CHECKBOX };
    displaySeqName.CVar("gAudioEditor.SeqNameNotification")
        .Options(CheckboxOptions()
                     .Color(THEME_COLOR)
                     .Tooltip("Displays the name of the current sequence in the corner of the screen whenever a new "
                              "sequence "
                              "is loaded to the main sequence player (does not apply to fanfares or enemy BGM)."));
    BenGui::mBenMenu->AddSearchWidget({ displaySeqName, "Enhancements", "Audio Editor", "Audio Options" });

    ovlDuration = { .name = "Overlay Duration: %d seconds", .type = WidgetType::WIDGET_CVAR_SLIDER_INT };
    ovlDuration.CVar("gAudioEditor.SeqNameNotificationDuration")
        .Options(IntSliderOptions().Color(THEME_COLOR).Min(1).Max(10).DefaultValue(5).Size(ImVec2(300.0f, 0.0f)));
    BenGui::mBenMenu->AddSearchWidget({ ovlDuration, "Enhancements", "Audio Editor", "Audio Options" });

    voicePitch = { .name = "Link's Voice Pitch Multiplier", .type = WidgetType::WIDGET_CVAR_SLIDER_FLOAT };
    voicePitch.CVar("gAudioEditor.LinkVoiceFreqMultiplier")
        .Options(FloatSliderOptions()
                     .Color(THEME_COLOR)
                     .IsPercentage()
                     .Min(0.4f)
                     .Max(2.5f)
                     .DefaultValue(1.0f)
                     .Size(ImVec2(300.0f, 0.0f)));
    BenGui::mBenMenu->AddSearchWidget({ voicePitch, "Enhancements", "Audio Editor", "Audio Options" });
}

static RegisterMenuInitFunc initAudioWidgets(RegisterAudioWidgets);