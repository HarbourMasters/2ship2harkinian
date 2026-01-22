#ifndef NETWORK_H
#define NETWORK_H
#ifdef __cplusplus

#include <thread>
#ifdef ENABLE_NETWORKING
#include <SDL2/SDL_net.h>
#endif
#include <nlohmann/json.hpp>

class Network {
  private:
#ifdef ENABLE_NETWORKING
    IPaddress networkAddress;
    TCPsocket networkSocket;
#endif
    std::thread receiveThread;
    std::string receivedData;

    void ReceiveFromServer();
    void HandleCompletePacket(std::string payload);
    void ProcessOutgoingPackets();

    std::mutex incomingPacketQueueMutex;
    std::queue<nlohmann::json> incomingPacketQueue;
    std::mutex outgoingPacketQueueMutex;
    std::queue<nlohmann::json> outgoingPacketQueue;

  public:
    bool isEnabled;
    bool isConnected;

    void Enable(const char* host, uint16_t port);
    void Disable();

    virtual void OnConnected();
    virtual void OnDisconnected();

    // To be called from the Game thread
    void SwapIncomingPacketQueue(std::queue<nlohmann::json>& emptyQueue);
    void QueueOutgoingPacket(nlohmann::json packet);
};

#endif // __cplusplus
#endif // NETWORK_H
