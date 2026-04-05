#include "2s2h/BenGui/UIWidgets.hpp"
#include "CosmeticEditor.h"
#include "2s2h/ShipInit.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>
#include <fast/resource/type/DisplayList.h>
#include <ship/resource/archive/Archive.h>

extern "C" {
#include "macros.h"
}

static constexpr const char* DYNAMIC_CVAR_PREFIX = "gCosmetic.Custom.";

struct DynamicCosmeticBinding {
    std::string materialPath;
    size_t commandIndex = 0;
    bool isPrimColor = true;
    uint8_t defaultA = 255;
    uint8_t primM = 0;
    uint8_t primL = 0;
};

struct DynamicCosmeticEntry {
    std::string displayName;
    std::string category;
    std::string colorCvar;
    std::string changedCvar;
    Color_RGBA8 defaultColor = { 255, 255, 255, 255 };
    std::vector<DynamicCosmeticBinding> bindings;
};

static std::vector<DynamicCosmeticEntry> dynamicCosmeticEntries;

void ApplyDynamicCosmetics();
static bool customHumanModelActive = false;
static bool customDekuModelActive = false;
static bool customGoronModelActive = false;
static bool customZoraModelActive = false;
static bool customFierceDeityModelActive = false;
static bool customKafeiModelActive = false;
static bool customHumanCosmeticsAvailable = false;
static bool customDekuCosmeticsAvailable = false;
static bool customGoronCosmeticsAvailable = false;
static bool customZoraCosmeticsAvailable = false;
static bool customFierceDeityCosmeticsAvailable = false;
static bool customKafeiCosmeticsAvailable = false;

static bool IsCustomArchive(const std::shared_ptr<Ship::Archive>& archive) {
    if (archive == nullptr) {
        return false;
    }

    const auto& archivePath = archive->GetPath();
    return archivePath.find("\\mods\\") != std::string::npos || archivePath.find("/mods/") != std::string::npos;
}

static void SanitizeDynamicCosmeticKey(std::string& value) {
    for (auto it = value.begin(); it != value.end();) {
        if (!std::isalnum(static_cast<unsigned char>(*it))) {
            it = value.erase(it);
        } else {
            ++it;
        }
    }
}

static bool IsSkeletonOverriddenByCustomArchive(Ship::ArchiveManager* archiveManager, const char* path) {
    if (archiveManager == nullptr) {
        return false;
    }

    return IsCustomArchive(archiveManager->GetArchiveFromFile(path));
}

static void RefreshCustomModelActiveFlags(Ship::ArchiveManager* archiveManager) {
    customHumanModelActive =
        IsSkeletonOverriddenByCustomArchive(archiveManager, "objects/object_link_child/gLinkHumanSkel");
    customDekuModelActive =
        IsSkeletonOverriddenByCustomArchive(archiveManager, "objects/object_link_nuts/gLinkDekuSkel");
    customGoronModelActive =
        IsSkeletonOverriddenByCustomArchive(archiveManager, "objects/object_link_goron/gLinkGoronSkel");
    customZoraModelActive =
        IsSkeletonOverriddenByCustomArchive(archiveManager, "objects/object_link_zora/gLinkZoraSkel");
    customFierceDeityModelActive =
        IsSkeletonOverriddenByCustomArchive(archiveManager, "objects/object_link_boy/gLinkFierceDeitySkel");
    customKafeiModelActive = IsSkeletonOverriddenByCustomArchive(archiveManager, "objects/object_test3/gKafeiSkel");
}

enum class DynamicCosmeticForm {
    Human = 0,
    Deku,
    Goron,
    Zora,
    FierceDeity,
    Kafei,
    Other,
};

static DynamicCosmeticForm GetDynamicMaterialForm(const std::string& materialPath) {
    if (materialPath.starts_with("objects/object_link_child/")) {
        return DynamicCosmeticForm::Human;
    }
    if (materialPath.starts_with("objects/object_link_nuts/")) {
        return DynamicCosmeticForm::Deku;
    }
    if (materialPath.starts_with("objects/object_link_goron/")) {
        return DynamicCosmeticForm::Goron;
    }
    if (materialPath.starts_with("objects/object_link_zora/")) {
        return DynamicCosmeticForm::Zora;
    }
    if (materialPath.starts_with("objects/object_link_boy/")) {
        return DynamicCosmeticForm::FierceDeity;
    }
    if (materialPath.starts_with("objects/object_test3/")) {
        return DynamicCosmeticForm::Kafei;
    }

    return DynamicCosmeticForm::Other;
}

static int GetDynamicMaterialFormSortOrder(DynamicCosmeticForm form) {
    return static_cast<int>(form);
}

static void MarkDynamicCosmeticsAvailable(DynamicCosmeticForm form) {
    switch (form) {
        case DynamicCosmeticForm::Human:
            customHumanCosmeticsAvailable = true;
            break;
        case DynamicCosmeticForm::Deku:
            customDekuCosmeticsAvailable = true;
            break;
        case DynamicCosmeticForm::Goron:
            customGoronCosmeticsAvailable = true;
            break;
        case DynamicCosmeticForm::Zora:
            customZoraCosmeticsAvailable = true;
            break;
        case DynamicCosmeticForm::FierceDeity:
            customFierceDeityCosmeticsAvailable = true;
            break;
        case DynamicCosmeticForm::Kafei:
            customKafeiCosmeticsAvailable = true;
            break;
        case DynamicCosmeticForm::Other:
            break;
    }
}

bool IsCustomModelActiveForCosmeticElement(const CosmeticEditorElement& element) {
    switch (element.id) {
        case COSMETIC_ELEMENT_HUMAN_TUNIC:
        case COSMETIC_ELEMENT_HUMAN_HAIR:
            return customHumanModelActive && customHumanCosmeticsAvailable;
        case COSMETIC_ELEMENT_DEKU_TUNIC:
        case COSMETIC_ELEMENT_DEKU_HAIR:
            return customDekuModelActive && customDekuCosmeticsAvailable;
        case COSMETIC_ELEMENT_GORON_TUNIC:
            return customGoronModelActive && customGoronCosmeticsAvailable;
        case COSMETIC_ELEMENT_ZORA_TUNIC:
            return customZoraModelActive && customZoraCosmeticsAvailable;
        case COSMETIC_ELEMENT_KAFEI_HAIR:
            return customKafeiModelActive && customKafeiCosmeticsAvailable;
        default:
            return false;
    }
}

bool IsCustomHumanModelActive() {
    return customHumanModelActive;
}

bool IsCustomDekuModelActive() {
    return customDekuModelActive;
}

bool IsCustomGoronModelActive() {
    return customGoronModelActive;
}

bool IsCustomZoraModelActive() {
    return customZoraModelActive;
}

bool IsCustomKafeiModelActive() {
    return customKafeiModelActive;
}

static bool TryLoadDynamicDisplayListXml(Ship::ArchiveManager* archiveManager, Ship::ResourceManager* resourceManager,
                                         const std::string& materialPath, tinyxml2::XMLDocument& document,
                                         std::shared_ptr<Fast::DisplayList>& material, tinyxml2::XMLElement*& root) {
    auto file = archiveManager->LoadFile(materialPath);
    if (file == nullptr || !file->IsLoaded || file->Buffer == nullptr) {
        return false;
    }

    document.Parse(file->Buffer->data(), file->Buffer->size());
    if (document.Error()) {
        return false;
    }

    root = document.FirstChildElement();
    if (root == nullptr || std::string(root->Name()) != "DisplayList") {
        return false;
    }

    material = std::dynamic_pointer_cast<Fast::DisplayList>(resourceManager->LoadResource(materialPath));
    return material != nullptr;
}

static size_t FindDynamicInstructionIndex(const Fast::DisplayList& displayList, const Gfx& expected,
                                          size_t searchStart) {
    for (size_t i = searchStart; i < displayList.Instructions.size(); i++) {
        const Gfx& current = displayList.Instructions[i];
        if (current.words.w0 == expected.words.w0 && current.words.w1 == expected.words.w1) {
            return i;
        }
    }

    return SIZE_MAX;
}

static Color_RGBA8 GetDynamicCosmeticColor(const DynamicCosmeticEntry& entry) {
    if (CVarGetInteger(entry.changedCvar.c_str(), 0)) {
        return CVarGetColor(entry.colorCvar.c_str(), entry.defaultColor);
    }

    return entry.defaultColor;
}

static void SetDynamicCosmeticChanged(const DynamicCosmeticEntry& entry, Color_RGBA8 color) {
    CVarSetColor(entry.colorCvar.c_str(), color);
    CVarSetInteger(entry.changedCvar.c_str(), 1);
    ShipInit::Init(entry.colorCvar.c_str());
    ShipInit::Init(entry.changedCvar.c_str());
}

static void ResetDynamicCosmeticChanged(const DynamicCosmeticEntry& entry) {
    CVarClear(entry.colorCvar.c_str());
    CVarClear(entry.changedCvar.c_str());
    ShipInit::Init(entry.colorCvar.c_str());
    ShipInit::Init(entry.changedCvar.c_str());
}

static void CopyDynamicColorArray(const DynamicCosmeticEntry& entry, float currentColor[4]) {
    Color_RGBA8 color = GetDynamicCosmeticColor(entry);
    currentColor[0] = color.r / 255.0f;
    currentColor[1] = color.g / 255.0f;
    currentColor[2] = color.b / 255.0f;
    currentColor[3] = color.a / 255.0f;
}

static void RandomizeDynamicCosmetic(const DynamicCosmeticEntry& entry) {
    Color_RGBA8 color = { static_cast<uint8_t>(rand() % 256), static_cast<uint8_t>(rand() % 256),
                          static_cast<uint8_t>(rand() % 256), 255 };
    SetDynamicCosmeticChanged(entry, color);
}

static void CommitDynamicCosmeticChanges(bool save) {
    ApplyDynamicCosmetics();
    if (save) {
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
    }
}

static void SetDynamicCosmeticColorFromEditor(const DynamicCosmeticEntry& entry, const float currentColor[4]) {
    Color_RGBA8 colorSelected = { static_cast<uint8_t>(currentColor[0] * 255.0f),
                                  static_cast<uint8_t>(currentColor[1] * 255.0f),
                                  static_cast<uint8_t>(currentColor[2] * 255.0f), 255 };
    SetDynamicCosmeticChanged(entry, colorSelected);
}

static void DrawDynamicCosmeticEntryOptions(const DynamicCosmeticEntry& entry) {
    float currentColor[4];
    CopyDynamicColorArray(entry, currentColor);

    bool colorChanged =
        ImGui::ColorEdit3("Color", currentColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
    if (colorChanged) {
        SetDynamicCosmeticColorFromEditor(entry, currentColor);
        CommitDynamicCosmeticChanges(true);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_UNDO, ImVec2(27.0f, 27.0f))) {
        ResetDynamicCosmeticChanged(entry);
        CommitDynamicCosmeticChanges(true);
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_RECYCLE, ImVec2(27.0f, 27.0f))) {
        RandomizeDynamicCosmetic(entry);
        CommitDynamicCosmeticChanges(true);
    }

    ImGui::SameLine();
    ImGui::TextColored(CVarGetInteger(entry.changedCvar.c_str(), 0)
                           ? UIWidgets::ColorValues.at(UIWidgets::Colors::Green)
                           : UIWidgets::ColorValues.at(UIWidgets::Colors::Gray),
                       CVarGetInteger(entry.changedCvar.c_str(), 0) ? "Modified" : "Default");
}

void ApplyDynamicCosmetics() {
    auto resourceManager = Ship::Context::GetInstance()->GetResourceManager();
    auto archiveManager = resourceManager->GetArchiveManager();

    for (const auto& entry : dynamicCosmeticEntries) {
        Color_RGBA8 color = GetDynamicCosmeticColor(entry);

        for (const auto& binding : entry.bindings) {
            if (!IsCustomArchive(archiveManager->GetArchiveFromFile(binding.materialPath))) {
                continue;
            }

            auto material =
                std::dynamic_pointer_cast<Fast::DisplayList>(resourceManager->LoadResource(binding.materialPath));
            if (material == nullptr || binding.commandIndex >= material->Instructions.size()) {
                continue;
            }

            if (binding.isPrimColor) {
                material->Instructions[binding.commandIndex] =
                    gsDPSetPrimColor(binding.primM, binding.primL, color.r, color.g, color.b, binding.defaultA);
            } else {
                material->Instructions[binding.commandIndex] =
                    gsDPSetEnvColor(color.r, color.g, color.b, binding.defaultA);
            }
        }
    }
}

void RandomizeAllDynamicCosmetics() {
    for (const auto& entry : dynamicCosmeticEntries) {
        RandomizeDynamicCosmetic(entry);
    }

    CommitDynamicCosmeticChanges(true);
}

void ResetAllDynamicCosmetics() {
    for (const auto& entry : dynamicCosmeticEntries) {
        ResetDynamicCosmeticChanged(entry);
    }

    CommitDynamicCosmeticChanges(false);
}

void ScanDynamicCosmetics() {
    dynamicCosmeticEntries.clear();
    customHumanCosmeticsAvailable = false;
    customDekuCosmeticsAvailable = false;
    customGoronCosmeticsAvailable = false;
    customZoraCosmeticsAvailable = false;
    customFierceDeityCosmeticsAvailable = false;
    customKafeiCosmeticsAvailable = false;

    auto resourceManager = Ship::Context::GetInstance()->GetResourceManager();
    auto archiveManager = resourceManager->GetArchiveManager();
    RefreshCustomModelActiveFlags(archiveManager.get());
    auto materialPaths = archiveManager->ListFiles("*");
    std::unordered_map<std::string, size_t> entryIndicesByKey;

    for (const auto& materialPath : *materialPaths) {
        if (!IsCustomArchive(archiveManager->GetArchiveFromFile(materialPath))) {
            continue;
        }

        tinyxml2::XMLDocument document;
        std::shared_ptr<Fast::DisplayList> material;
        tinyxml2::XMLElement* root = nullptr;
        if (!TryLoadDynamicDisplayListXml(archiveManager.get(), resourceManager.get(), materialPath, document, material,
                                          root)) {
            continue;
        }

        size_t searchStart = 0;
        for (auto* child = root->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
            std::string childName = child->Name();
            bool isPrimColor = childName == "SetPrimColor";
            if (!isPrimColor && childName != "SetEnvColor") {
                continue;
            }

            const char* cosmeticEntry = child->Attribute("CosmeticEntry");
            const char* cosmeticCategory = child->Attribute("CosmeticCategory");
            if (cosmeticEntry == nullptr || cosmeticEntry[0] == '\0') {
                continue;
            }

            std::string key = cosmeticEntry;
            SanitizeDynamicCosmeticKey(key);
            if (key.empty()) {
                continue;
            }

            Gfx expectedInstruction;
            if (isPrimColor) {
                expectedInstruction =
                    gsDPSetPrimColor(child->IntAttribute("M"), child->IntAttribute("L"), child->IntAttribute("R"),
                                     child->IntAttribute("G"), child->IntAttribute("B"), child->IntAttribute("A"));
            } else {
                expectedInstruction = gsDPSetEnvColor(child->IntAttribute("R"), child->IntAttribute("G"),
                                                      child->IntAttribute("B"), child->IntAttribute("A"));
            }

            size_t commandIndex = FindDynamicInstructionIndex(*material, expectedInstruction, searchStart);
            if (commandIndex == SIZE_MAX) {
                continue;
            }
            searchStart = commandIndex + 1;

            MarkDynamicCosmeticsAvailable(GetDynamicMaterialForm(materialPath));

            size_t entryIndex = 0;
            if (auto it = entryIndicesByKey.find(key); it != entryIndicesByKey.end()) {
                entryIndex = it->second;
            } else {
                entryIndex = dynamicCosmeticEntries.size();
                entryIndicesByKey[key] = entryIndex;

                DynamicCosmeticEntry entry;
                entry.displayName = cosmeticEntry;
                entry.category = (cosmeticCategory != nullptr) ? cosmeticCategory : "";
                entry.colorCvar = std::string(DYNAMIC_CVAR_PREFIX) + key + ".Color";
                entry.changedCvar = std::string(DYNAMIC_CVAR_PREFIX) + key + ".Changed";
                entry.defaultColor = { static_cast<uint8_t>(child->IntAttribute("R")),
                                       static_cast<uint8_t>(child->IntAttribute("G")),
                                       static_cast<uint8_t>(child->IntAttribute("B")),
                                       static_cast<uint8_t>(child->IntAttribute("A")) };
                dynamicCosmeticEntries.push_back(std::move(entry));
            }

            DynamicCosmeticBinding binding;
            binding.materialPath = materialPath;
            binding.commandIndex = commandIndex;
            binding.isPrimColor = isPrimColor;
            binding.defaultA = static_cast<uint8_t>(child->IntAttribute("A"));
            binding.primM = static_cast<uint8_t>(child->IntAttribute("M"));
            binding.primL = static_cast<uint8_t>(child->IntAttribute("L"));
            dynamicCosmeticEntries[entryIndex].bindings.push_back(std::move(binding));
        }
    }

    std::stable_sort(
        dynamicCosmeticEntries.begin(), dynamicCosmeticEntries.end(),
        [](const DynamicCosmeticEntry& lhs, const DynamicCosmeticEntry& rhs) {
            int lhsOrder = 6;
            int rhsOrder = 6;

            for (const auto& binding : lhs.bindings) {
                lhsOrder =
                    std::min(lhsOrder, GetDynamicMaterialFormSortOrder(GetDynamicMaterialForm(binding.materialPath)));
            }
            for (const auto& binding : rhs.bindings) {
                rhsOrder =
                    std::min(rhsOrder, GetDynamicMaterialFormSortOrder(GetDynamicMaterialForm(binding.materialPath)));
            }

            if (lhsOrder != rhsOrder) {
                return lhsOrder < rhsOrder;
            }

            if (lhs.category.empty() != rhs.category.empty()) {
                return !lhs.category.empty();
            }

            if (lhs.category != rhs.category) {
                return lhs.category < rhs.category;
            }

            return lhs.displayName < rhs.displayName;
        });

    ApplyDynamicCosmetics();
}

static void DrawDynamicCosmeticEntriesTable(const char* tableId,
                                            const std::vector<const DynamicCosmeticEntry*>& entries) {
    if (!ImGui::BeginTable(tableId, 2)) {
        return;
    }

    ImGui::TableSetupColumn("Element Name", ImGuiTableColumnFlags_WidthStretch, 1.4f);
    ImGui::TableSetupColumn("Options", ImGuiTableColumnFlags_WidthStretch, 2.0f);

    for (const auto* entry : entries) {
        ImGui::PushID(entry->colorCvar.c_str());
        ImGui::TableNextColumn();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                             ((ImGui::GetFrameHeight() - ImGui::CalcTextSize(entry->displayName.c_str()).y) / 2.0f));
        ImGui::Text("%s", entry->displayName.c_str());
        ImGui::TableNextColumn();

        DrawDynamicCosmeticEntryOptions(*entry);
        ImGui::PopID();
    }

    ImGui::EndTable();
}

void DrawDynamicCosmetics() {
    if (dynamicCosmeticEntries.empty()) {
        return;
    }

    ImGui::SeparatorText("Custom");

    std::vector<const DynamicCosmeticEntry*> currentEntries;
    std::string currentCategory;
    int tableIndex = 0;

    auto flushTable = [&]() {
        if (!currentEntries.empty()) {
            std::string tableId = "DynamicCosmetics##" + std::to_string(tableIndex++);
            DrawDynamicCosmeticEntriesTable(tableId.c_str(), currentEntries);
            currentEntries.clear();
        }
    };

    for (const auto& entry : dynamicCosmeticEntries) {
        if (entry.category != currentCategory) {
            flushTable();
            currentCategory = entry.category;
            if (!currentCategory.empty()) {
                ImGui::Text("%s", currentCategory.c_str());
            }
        }
        currentEntries.push_back(&entry);
    }

    flushTable();
}
