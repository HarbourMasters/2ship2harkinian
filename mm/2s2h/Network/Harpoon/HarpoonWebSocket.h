#ifndef HARPOON_WEBSOCKET_H
#define HARPOON_WEBSOCKET_H
#ifdef __cplusplus

// =============================================================================
// HarpoonWebSocket — WebSocket client (RFC 6455 plain ws://) used by Harpoon.
// =============================================================================
//
// Ported literally from Ship of Harkinian to 2ship2harkinian. The only change
// versus the SoH original is the compile guard: SoH uses ENABLE_REMOTE_CONTROL
// (gated on its BUILD_REMOTE_CONTROL flag); 2ship uses ENABLE_HARPOON gated on
// the BUILD_HARPOON CMake option.
//
// For TLS (wss://), front the server with a reverse proxy (Caddy / Nginx /
// AWS ALB) that terminates TLS — this client always speaks plain ws://.
// =============================================================================

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <cstdint>
#include <queue>
#ifdef ENABLE_HARPOON
#include <SDL2/SDL_net.h>
#endif

class HarpoonWebSocket {
  public:
    using TextHandler        = std::function<void(const std::string&)>;
    using ConnectHandler     = std::function<void()>;
    using DisconnectHandler  = std::function<void()>;

    HarpoonWebSocket();
    ~HarpoonWebSocket();

    void Connect(const std::string& host, uint16_t port);
    void Disconnect();

    void SendText(const std::string& payload);

    void SetOnText(TextHandler h)            { onText_ = std::move(h); }
    void SetOnConnected(ConnectHandler h)    { onConnected_ = std::move(h); }
    void SetOnDisconnected(DisconnectHandler h) { onDisconnected_ = std::move(h); }

    bool IsEnabled()   const { return enabled_.load(); }
    bool IsConnected() const { return connectedAndHandshakeDone_.load(); }

  private:
#ifdef ENABLE_HARPOON
    IPaddress address_{};
    TCPsocket socket_ = nullptr;
#endif
    std::string host_;
    uint16_t port_ = 0;

    std::thread thread_;
    std::atomic<bool> enabled_{ false };
    std::atomic<bool> connectedAndHandshakeDone_{ false };

    std::mutex outMutex_;
    std::queue<std::string> outQueue_;

    std::string rxBuffer_;
    std::string textAccum_;

    TextHandler        onText_;
    ConnectHandler     onConnected_;
    DisconnectHandler  onDisconnected_;

    void RunLoop();
    bool PerformHandshake();
    void ProcessOutbound();
    void ProcessInboundFrames();
    void Cleanup();
};

#endif // __cplusplus
#endif // HARPOON_WEBSOCKET_H
