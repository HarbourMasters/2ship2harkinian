#include "MessageViewer.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/BenGui/BenGui.hpp"
#include "2s2h/CustomMessage/CustomMessage.h"
#include "BenPort.h"

#include <message_data_static.h>

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
}

using namespace UIWidgets;

// Language names for the UI selector (prepared for future support)
static std::unordered_map<int32_t, const char*> languageNames = {
    { LANGUAGE_JPN, "Japanese" }, { LANGUAGE_ENG, "English" }, { LANGUAGE_GER, "German" },
    { LANGUAGE_FRE, "French" },   { LANGUAGE_SPA, "Spanish" },
};

void MessageViewerWindow::InitElement() {
    mTableIdBuf = static_cast<char*>(calloc(MAX_STRING_SIZE, sizeof(char)));
    mTextIdBuf = static_cast<char*>(calloc(MAX_STRING_SIZE, sizeof(char)));
    mCustomMessageBuf = static_cast<char*>(calloc(MAX_STRING_SIZE, sizeof(char)));
}

MessageViewerWindow::~MessageViewerWindow() {
    free(mTableIdBuf);
    free(mTextIdBuf);
    free(mCustomMessageBuf);
}

void MessageViewerWindow::DrawElement() {
    ImGui::BeginDisabled(true);
    ImGui::Text("Table ID");
    ImGui::SameLine();
    PushStyleInput(THEME_COLOR);
    ImGui::InputText("##TableID", mTableIdBuf, MAX_STRING_SIZE, ImGuiInputTextFlags_CallbackCharFilter,
                     TextFilters::FilterAlphaNum);
    Tooltip("Not yet implemented. Leave blank for vanilla message table.");
    PopStyleInput();
    ImGui::EndDisabled();

    ImGui::Text("Text ID");
    ImGui::SameLine();
    PushStyleInput(THEME_COLOR);
    switch (mTextIdBase) {
        case DECIMAL:
            ImGui::InputText("##TextID", mTextIdBuf, MAX_STRING_SIZE, ImGuiInputTextFlags_CharsDecimal);
            Tooltip("Decimal Text ID of the message to load. Decimal digits only (0-9).");
            break;
        case HEXADECIMAL:
        default:
            ImGui::InputText("##TextID", mTextIdBuf, MAX_STRING_SIZE, ImGuiInputTextFlags_CharsHexadecimal);
            Tooltip("Hexadecimal Text ID of the message to load. Hexadecimal digits only (0-9/A-F).");
            break;
    }
    PopStyleInput();

    PushStyleCheckbox(THEME_COLOR);
    if (ImGui::RadioButton("Hexadecimal", &mTextIdBase, HEXADECIMAL)) {
        memset(mTextIdBuf, 0, sizeof(char) * MAX_STRING_SIZE);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Decimal", &mTextIdBase, DECIMAL)) {
        memset(mTextIdBuf, 0, sizeof(char) * MAX_STRING_SIZE);
    }
    PopStyleCheckbox();

    // Language selector (prepared for future multi-language table support)
    UIWidgets::ComboboxOptions languageOptions = {};
    languageOptions.color = THEME_COLOR;
    languageOptions.disabled = true;
    languageOptions.tooltip = "Not yet implemented. Currently only English messages are supported.";
    UIWidgets::Combobox("Language", &mLanguage, &languageNames, languageOptions);

    PushStyleButton(THEME_COLOR);
    if (ImGui::Button("Display Message##ExistingMessage")) {
        mDisplayExistingMessageClicked = true;
    }
    PopStyleButton();

    ImGui::Text("Custom Message");
    Tooltip("Enter a string using Custom Message Syntax to preview it in-game. "
            "Supports color codes (%r, %w, %y, %g, %b, %p). "
            "Any newline (\\n) characters inserted by the Enter key will be stripped from the output.");

    PushStyleInput(THEME_COLOR);
    ImGui::InputTextMultiline("##CustomMessage", mCustomMessageBuf, MAX_STRING_SIZE);
    PopStyleInput();

    PushStyleButton(THEME_COLOR);
    if (ImGui::Button("Display Message##CustomMessage")) {
        mDisplayCustomMessageClicked = true;
    }
    PopStyleButton();
}

void MessageViewerWindow::UpdateElement() {
    if (mDisplayExistingMessageClicked) {
        mTableId = std::string(mTableIdBuf);

        // Check if text ID buffer is empty
        if (strlen(mTextIdBuf) == 0) {
            mDisplayExistingMessageClicked = false;
            return;
        }

        try {
            switch (mTextIdBase) {
                case DECIMAL:
                    mTextId = std::stoi(std::string(mTextIdBuf), nullptr, 10);
                    break;
                case HEXADECIMAL:
                default:
                    mTextId = std::stoi(std::string(mTextIdBuf), nullptr, 16);
                    break;
            }
            DisplayExistingMessage();
        } catch (const std::exception& e) {
            // Invalid text ID input, just ignore
        }
        mDisplayExistingMessageClicked = false;
    }
    if (mDisplayCustomMessageClicked) {
        mCustomMessageString = std::string(mCustomMessageBuf);

        // Check if custom message buffer is empty
        if (mCustomMessageString.empty()) {
            mDisplayCustomMessageClicked = false;
            return;
        }

        std::erase(mCustomMessageString, '\n');
        DisplayCustomMessage();
        mDisplayCustomMessageClicked = false;
    }
}

void MessageViewerWindow::DisplayExistingMessage() const {
    MessageDebug_StartTextBox(mTableId.c_str(), mTextId, mLanguage);
}

void MessageViewerWindow::DisplayCustomMessage() const {
    MessageDebug_DisplayCustomMessage(mCustomMessageString.c_str());
}

void MessageDebug_StartTextBox(const char* tableId, uint16_t textId, uint8_t language) {
    PlayState* play = gPlayState;
    if (play == nullptr) {
        return;
    }

    const auto player = GET_PLAYER(gPlayState);
    if (player == nullptr) {
        return;
    }

    MessageContext* msgCtx = &play->msgCtx;

    // Validate that the textId exists in the message table to prevent crashes
    MessageTableEntry* msgEntry = msgCtx->messageTableNES;
    if (msgEntry == nullptr) {
        return;
    }

    bool textIdExists = false;
    while (msgEntry->textId != 0xFFFF) {
        if (msgEntry->textId == textId) {
            textIdExists = true;
            break;
        }
        msgEntry++;
    }

    if (!textIdExists) {
        // Text ID doesn't exist, don't try to display it
        return;
    }

    // For vanilla messages (empty tableId), use the standard message system
    // For custom message tables (future implementation), we'd load from a custom table
    // Language parameter is prepared for future multi-language table support
    if (strlen(tableId) == 0) {
        // Use the built-in Message_StartTextbox - it handles everything:
        // - Calls Message_OpenText() which loads the message
        // - Sets up all message context state
        // - Starts the display
        Message_StartTextbox(play, textId, &player->actor);
    } else {
        // Custom message table (prepared for future implementation)
        // For now, fall back to vanilla
        Message_StartTextbox(play, textId, &player->actor);
    }
}

void MessageDebug_DisplayCustomMessage(const char* customMessage) {
    if (gPlayState == nullptr) {
        return;
    }

    const auto player = GET_PLAYER(gPlayState);
    if (player == nullptr) {
        return;
    }

    // Create a custom message entry
    CustomMessage::Entry entry;
    entry.textboxType = 0; // Default black textbox
    entry.textboxYPos = 3; // Bottom position
    entry.icon = 0xFE;     // No icon
    entry.nextMessageID = 0xFFFF;
    entry.firstItemCost = 0xFFFF;
    entry.secondItemCost = 0xFFFF;
    entry.msg = std::string(customMessage);
    entry.autoFormat = true;

    // Set the active custom message and display it
    CustomMessage::StartTextbox(entry.msg, entry);
}
