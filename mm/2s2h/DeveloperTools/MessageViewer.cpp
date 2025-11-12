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

int AutoLineBreak(ImGuiInputTextCallbackData* data) {
    if (!data || !data->Buf) {
        return 0;
    }

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (!window) {
        return 0;
    }

    float maxWidth = ImGui::GetContentRegionAvail().x;
    float lineWidth = 0.0f;
    size_t i = 0;

    while (data->Buf[i] != '\0' && i < data->BufSize - 1) {
        if (data->Buf[i] == '\n') {
            lineWidth = 0.0f;
            i++;
            continue;
        }
        char chStr[2] = { data->Buf[i], '\0' };
        float charWidth = ImGui::CalcTextSize(chStr).x;
        lineWidth += charWidth;
        if (lineWidth > maxWidth) {
            if (data->BufTextLen + 1 < data->BufSize) {
                memmove(data->Buf + i + 1, data->Buf + i, data->BufTextLen - i + 1);
                data->Buf[i] = '\n';
                data->BufTextLen++;
                lineWidth = 0.0f;
                i++;
            } else {
                break;
            }
        }
        i++;
    }

    return 0;
}

static std::string ParseEscapeSequences(const std::string& input) {
    std::string output;
    for (size_t i = 0; i < input.length(); ++i) {
        if (i + 3 < input.length() && input[i] == '\\' && input[i + 1] == 'x') {
            char hex[3] = { input[i + 2], input[i + 3], '\0' };
            char* endptr;
            unsigned char byte = (unsigned char)strtol(hex, &endptr, 16);
            if (endptr == hex + 2) { // Successfully parsed 2 hex digits
                output += (char)byte;
                i += 3; // Skip \xXX
            } else {
                output += input[i]; // Not valid hex, keep as-is
            }
        } else {
            output += input[i];
        }
    }
    return output;
}

static bool ValidateTextIdExists(uint16_t textId) {
    if (gPlayState == nullptr) {
        return false;
    }

    MessageTableEntry* msgEntry = gPlayState->msgCtx.messageTableNES;
    if (msgEntry == nullptr) {
        return false;
    }

    while (msgEntry->textId != 0xFFFF) {
        if (msgEntry->textId == textId) {
            return true;
        }
        msgEntry++;
    }
    return false;
}

void MessageViewerWindow::InitElement() {
    mTextIdBuf = static_cast<char*>(calloc(MAX_STRING_SIZE, sizeof(char)));
    mCustomMessageBuf = static_cast<char*>(calloc(MAX_STRING_SIZE, sizeof(char)));
    mcustomMessageRaw = static_cast<char*>(calloc(MAX_STRING_SIZE, sizeof(char)));
}

MessageViewerWindow::~MessageViewerWindow() {
    free(mTextIdBuf);
    free(mCustomMessageBuf);
    free(mcustomMessageRaw);
}

void MessageViewerWindow::DrawElement() {
    // Vanilla Message Viewer Section
    ImGui::SeparatorText("Vanilla Message Viewer");

    PushStyleInput(THEME_COLOR);
    switch (mTextIdBase) {
        case DECIMAL:
            ImGui::InputText("##TextID", mTextIdBuf, MAX_STRING_SIZE, ImGuiInputTextFlags_CharsDecimal);
            break;
        case HEXADECIMAL:
        default:
            ImGui::InputText("##TextID", mTextIdBuf, MAX_STRING_SIZE, ImGuiInputTextFlags_CharsHexadecimal);
            break;
    }

    // Draw placeholder overlay immediately if empty
    if (strlen(mTextIdBuf) == 0) {
        ImVec2 inputMin = ImGui::GetItemRectMin();
        ImVec2 textPos =
            ImVec2(inputMin.x + ImGui::GetStyle().FramePadding.x + 4.0f, inputMin.y + ImGui::GetStyle().FramePadding.y);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddText(textPos, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.4f)), "TextID");
    }
    PopStyleInput();

    // Draw radio buttons on same line as input
    ImGui::SameLine();
    PushStyleCheckbox(THEME_COLOR);
    if (ImGui::RadioButton("Hex", &mTextIdBase, HEXADECIMAL)) {
        memset(mTextIdBuf, 0, sizeof(char) * MAX_STRING_SIZE);
    }
    Tooltip("Hexadecimal Text ID of the message to load. Hexadecimal digits only (0-9/A-F).");
    ImGui::SameLine();
    if (ImGui::RadioButton("Dec", &mTextIdBase, DECIMAL)) {
        memset(mTextIdBuf, 0, sizeof(char) * MAX_STRING_SIZE);
    }
    Tooltip("Decimal Text ID of the message to load. Decimal digits only (0-9).");
    PopStyleCheckbox();

    PushStyleButton(THEME_COLOR);
    if (ImGui::Button("Display Message##ExistingMessage")) {
        mDisplayExistingMessageClicked = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Message##ExistingMessage")) {
        mLoadMessageClicked = true;
    }
    PopStyleButton();

    // Custom Message Builder Section
    ImGui::SeparatorText("Custom Message Builder");

    float currentMessageBoxWidth = ImGui::GetContentRegionAvail().x;

    PushStyleInput(THEME_COLOR);
    if (ImGui::InputTextMultiline("##CustomMessage", mCustomMessageBuf, MAX_STRING_SIZE,
                                  ImVec2(ImGui::GetContentRegionAvail().x, 100), ImGuiInputTextFlags_CallbackEdit,
                                  AutoLineBreak)) {
        if (previousMessageBoxWidth != currentMessageBoxWidth) {
            previousMessageBoxWidth = currentMessageBoxWidth;
            mcustomMessageRaw[0] = '\0';
            for (size_t i = 0; i < sizeof(mCustomMessageBuf); i++) {
                if (mCustomMessageBuf[i] == '\n') {
                    continue;
                }
                mcustomMessageRaw[i] = mCustomMessageBuf[i];
            }
            mCustomMessageBuf = mcustomMessageRaw;
        }
    }
    PopStyleInput();

    Tooltip(
        "Enter text to preview in-game. Supports color codes (%r, %w, %y, %g, %b, %p) and escape sequences (\\xXX).\n"
        "Use 'Load Message' to inspect vanilla message format. Newlines are stripped from simple text.");

    PushStyleButton(THEME_COLOR);
    if (ImGui::Button("Display Message##CustomMessage")) {
        mDisplayCustomMessageClicked = true;
    }
    PopStyleButton();
}

void MessageViewerWindow::UpdateElement() {
    if (mDisplayExistingMessageClicked) {
        if (ParseTextIdFromBuffer(mTextId)) {
            DisplayExistingMessage();
        }
        mDisplayExistingMessageClicked = false;
    }
    if (mLoadMessageClicked) {
        if (ParseTextIdFromBuffer(mTextId)) {
            LoadMessageToEditor();
        }
        mLoadMessageClicked = false;
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
    MessageDebug_StartTextBox("", mTextId, LANGUAGE_ENG);
}

void MessageViewerWindow::DisplayCustomMessage() const {
    MessageDebug_DisplayCustomMessage(mCustomMessageString.c_str());
}

void MessageViewerWindow::LoadMessageToEditor() {
    if (!ValidateTextIdExists(mTextId)) {
        return;
    }

    // Load the vanilla message entry
    CustomMessage::Entry entry = CustomMessage::LoadVanillaMessageTableEntry(mTextId);

    // Message header format (11 bytes) - see: https://wiki.cloudmodding.com/mm/Text_Format
    // [align|boxType][yPos|skip][icon][nextID][cost1][cost2][0xFF][0xFF]

    uint8_t textAlignment = 0;
    uint8_t textUnskippable = 0;

    std::ostringstream rawMessage;

    // Format header bytes
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0')
               << (int)((textAlignment << 4) | (entry.textboxType & 0x0F));
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0')
               << (int)((entry.textboxYPos << 4) | (textUnskippable & 0x0F));
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)entry.icon;
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)((entry.nextMessageID & 0xFF00) >> 8);
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)(entry.nextMessageID & 0x00FF);
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)((entry.firstItemCost & 0xFF00) >> 8);
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)(entry.firstItemCost & 0x00FF);
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)((entry.secondItemCost & 0xFF00) >> 8);
    rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)(entry.secondItemCost & 0x00FF);
    rawMessage << "\\xff\\xff"; // Padding bytes

    // Format message content bytes
    for (size_t i = 0; i < entry.msg.length(); ++i) {
        unsigned char byte = entry.msg[i];
        if (byte >= 0x20 && byte <= 0x7E) {
            // Printable ASCII - add as-is
            rawMessage << (char)byte;
        } else {
            // Control code - format as \xXX
            rawMessage << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
        }
    }

    // Copy the formatted string to the custom message buffer
    std::string formattedMessage = rawMessage.str();
    strncpy(mCustomMessageBuf, formattedMessage.c_str(), MAX_STRING_SIZE - 1);
    mCustomMessageBuf[MAX_STRING_SIZE - 1] = '\0';
}

bool MessageViewerWindow::ParseTextIdFromBuffer(uint16_t& outTextId) {
    if (strlen(mTextIdBuf) == 0) {
        return false;
    }
    try {
        switch (mTextIdBase) {
            case DECIMAL:
                outTextId = std::stoi(std::string(mTextIdBuf), nullptr, 10);
                break;
            case HEXADECIMAL:
            default:
                outTextId = std::stoi(std::string(mTextIdBuf), nullptr, 16);
                break;
        }
        return true;
    } catch (const std::exception&) { return false; }
}

bool MessageViewerWindow::ValidateTextIdExists(uint16_t textId) {
    if (gPlayState == nullptr) {
        return false;
    }

    MessageTableEntry* msgEntry = gPlayState->msgCtx.messageTableNES;
    if (msgEntry == nullptr) {
        return false;
    }

    while (msgEntry->textId != 0xFFFF) {
        if (msgEntry->textId == textId) {
            return true;
        }
        msgEntry++;
    }
    return false;
}

void MessageDebug_StartTextBox(const char* tableId, uint16_t textId, uint8_t language) {
    if (!ValidateTextIdExists(textId)) {
        return;
    }

    const auto player = GET_PLAYER(gPlayState);
    if (player == nullptr) {
        return;
    }

    Message_StartTextbox(gPlayState, textId, &player->actor);
}

void MessageDebug_DisplayCustomMessage(const char* customMessage) {
    if (gPlayState == nullptr) {
        return;
    }

    const auto player = GET_PLAYER(gPlayState);
    if (player == nullptr) {
        return;
    }

    // Parse escape sequences in the input
    std::string processedMessage = ParseEscapeSequences(customMessage);

    // Create a custom message entry
    CustomMessage::Entry entry;

    // Check if message starts with header (11+ bytes starting with escape sequences)
    if (processedMessage.length() >= MESSAGE_HEADER_SIZE && strlen(customMessage) >= 2 && customMessage[0] == '\\' &&
        customMessage[1] == 'x') {

        // Parse header bytes
        entry.textboxType = (processedMessage[0] & 0x0F);
        entry.textboxYPos = (processedMessage[1] & 0xF0) >> 4;
        entry.icon = (unsigned char)processedMessage[2];
        entry.nextMessageID = ((unsigned char)processedMessage[3] << 8) | (unsigned char)processedMessage[4];
        entry.firstItemCost = ((unsigned char)processedMessage[5] << 8) | (unsigned char)processedMessage[6];
        entry.secondItemCost = ((unsigned char)processedMessage[7] << 8) | (unsigned char)processedMessage[8];

        // Skip header, use remaining as message content
        entry.msg = processedMessage.substr(MESSAGE_HEADER_SIZE);
        entry.autoFormat = false; // Already formatted

    } else {
        // No header - use defaults for user-written messages
        entry.textboxType = 0;
        entry.textboxYPos = 3;
        entry.icon = 0xFE;
        entry.nextMessageID = 0xFFFF;
        entry.firstItemCost = 0xFFFF;
        entry.secondItemCost = 0xFFFF;
        entry.msg = processedMessage;
        entry.autoFormat = true;
    }

    // Set the active custom message and display it
    CustomMessage::StartTextbox(entry.msg, entry);
}
