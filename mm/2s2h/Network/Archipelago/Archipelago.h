#pragma once

// Must be defined BEFORE any websocketpp / wswrap / apclientpp includes.
#ifdef _WIN32
// Prevent windows.h from dragging in winsock.h (which conflicts with WinSock2.h)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Make sure Winsock2 is used, not winsock.h
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

// apclientpp/wswrap note this sometimes matters for Asio on Windows
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#endif

// Force standalone Asio mode (no Boost.Asio)
#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

// Force websocketpp to use std::type_traits instead of Boost type_traits
#ifndef _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#endif

// Force websocketpp to use std::random (avoids Boost random)
#ifndef _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#endif
#ifndef _WEBSOCKETPP_CPP11_RANDOM_
#define _WEBSOCKETPP_CPP11_RANDOM_
#endif

#include "variables.h" // for gSaveContext + SAVETYPE_*

#define IS_ARCHI (IS_RANDO && gSaveContext.save.shipSaveInfo.rando.isArchiSave)

#ifdef __cplusplus
#include <string>
class Archipelago {
  public:
    static void Init();
    static void Shutdown();
    static void Update();

    static void RegisterMenu();

    static void ConnectFromCvars();
    static void Disconnect();
    static void SetDeathLinkTag();
    static bool IsConnected();
    static bool IsConnecting();
    static const char* GetStatusText();
    static void SendChat(const char* msg);
    static std::string GetPlayerAlias(int playerId);
    static int GetPlayerNumber();                                            // Get local player's slot number
    static std::string GetItemName(int64_t itemId, const std::string& game); // Get item name from AP
    static std::string GetPlayerGame(int playerId);                          // Get game name for a player
    static void ResyncItems(); // Re-enqueue all cached items for current save file

    // Send one location check to the AP server (no persistence here; call sites/bridge handle that).
    static void SendLocationCheck(uint64_t locationId);

    // New: called when a save is loaded (mirrors Rando's pattern)
    static void OnFileLoad(s16 fileNum);
};
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

// C-callable wrapper for Archipelago connection status
int Archipelago_IsConnected(void);
void Archipelago_ConnectFromCvars(void);

#ifdef __cplusplus
}
#endif
