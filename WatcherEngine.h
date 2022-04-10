#ifndef WATCHERENGINE_H
#define WATCHERENGINE_H

#include <list>
#include <vector>
#include <mutex>
#include <unordered_map>
#include "HPPackServer.h"
#include "HPPackClient.h"
#include "YMLConfig.hpp"

class WatcherEngine
{
public:
    explicit WatcherEngine();
    virtual ~WatcherEngine();
    void LoadConfig(const char* yml);
    void Start();
protected:
    void RegisterServer(const char *ip, unsigned int port);
    void RegisterClient(const char *ip, unsigned int port);
    void HandleThread();
    void HandlePackMessage(const Message::PackMessage &msg);
    void HandleCommand(const Message::PackMessage &msg);
    void HandleOrderRequest(const Message::PackMessage &msg);
    void HandleActionRequest(const Message::PackMessage &msg);
    void ForwardToXServer(const Message::PackMessage &msg);

    void HandleRiskCommand(const Message::PackMessage &msg);
    void HandleTraderCommand(const Message::PackMessage &msg);
    
    bool IsTrading()const;
    void CheckTrading();
private:
    HPPackServer* m_HPPackServer;
    HPPackClient* m_HPPackClient;
    Message::PackMessage m_PackMessage;
    Utils::XWatcherConfig m_XWatcherConfig;
    bool m_Trading;
    unsigned long m_CurrentTimeStamp;
    int m_OpenTime;
    int m_CloseTime;
    std::thread* m_pWorkThread;
};

#endif // WATCHERENGINE_H