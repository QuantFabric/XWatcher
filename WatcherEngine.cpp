#include "WatcherEngine.h"

extern Utils::Logger *gLogger;

WatcherEngine::WatcherEngine()
{
    m_HPPackServer = NULL;
    m_HPPackClient = NULL;
    m_pWorkThread = NULL;
}

WatcherEngine::~WatcherEngine()
{
    m_HPPackClient->Stop();
    delete m_HPPackClient;
    m_HPPackClient = NULL;
    m_HPPackServer->Stop();
    delete m_HPPackServer;
    m_HPPackServer = NULL;
    delete m_pWorkThread;
    m_pWorkThread = NULL;
}

void WatcherEngine::LoadConfig(const char* yml)
{
    Utils::gLogger->Log->info("WatcherEngine::LoadConfig {} start", yml);
    std::string errorBuffer;
    if(Utils::LoadXWatcherConfig(yml, m_XWatcherConfig, errorBuffer))
    {
        Utils::gLogger->Log->info("WatcherEngine::LoadXWatcherConfig {} successed", yml);
        m_OpenTime = Utils::getTimeStampMs(m_XWatcherConfig.OpenTime.c_str());
        m_CloseTime = Utils::getTimeStampMs(m_XWatcherConfig.CloseTime.c_str());
    }
    else
    {
        Utils::gLogger->Log->error("WatcherEngine::LoadXWatcherConfig {} failed, {}", yml, errorBuffer.c_str());
    }
}

void WatcherEngine::Start()
{
    RegisterServer(m_XWatcherConfig.ServerIP.c_str(), m_XWatcherConfig.Port);
    RegisterClient(m_XWatcherConfig.XServerIP.c_str(), m_XWatcherConfig.XServerPort);
    m_pWorkThread = new std::thread(&WatcherEngine::HandleThread, this);
    m_pWorkThread->join();
}

void WatcherEngine::RegisterServer(const char *ip, unsigned int port)
{
    m_HPPackServer = new HPPackServer(ip, port);
    m_HPPackServer->Start();
}

void WatcherEngine::RegisterClient(const char *ip, unsigned int port)
{
    m_HPPackClient = new HPPackClient(ip, port);
    m_HPPackClient->Start();
    Message::TLoginRequest login;
    login.ClientType = Message::EClientType::EXWATCHER;
    strncpy(login.Colo, m_XWatcherConfig.Colo.c_str(), sizeof(login.Colo));
    strncpy(login.Account, m_XWatcherConfig.Colo.c_str(), sizeof(login.Account));
    m_HPPackClient->Login(login);
}

void WatcherEngine::HandleThread()
{
    Utils::gLogger->Log->info("WatcherEngine::HandleThread to handle message");
    while (true)
    {
        CheckTrading();
        if(m_CurrentTimeStamp < m_CloseTime)
        {
            // Handle Server Received Message from APP
            memset(&m_PackMessage, 0, sizeof(m_PackMessage));
            bool ret = m_HPPackServer->m_PackMessageQueue.pop(m_PackMessage);
            if (ret)
            {
                HandlePackMessage(m_PackMessage);
            }
            // Handle Client Received Message from XServer
            memset(&m_PackMessage, 0, sizeof(m_PackMessage));
            ret = m_HPPackClient->m_PackMessageQueue.pop(m_PackMessage);
            if (ret)
            {
                HandlePackMessage(m_PackMessage);
            }
        }
        if(m_CurrentTimeStamp % 2000 == 0)
        {
            // Client ReConnect
            m_HPPackClient->ReConnect();
        }
    }
}

void WatcherEngine::HandlePackMessage(const Message::PackMessage &msg)
{
    unsigned int type = msg.MessageType;
    switch (type)
    {
    case Message::EMessageType::ELoginRequest:
    case Message::EMessageType::ECommand:
    case Message::EMessageType::EOrderRequest:
    case Message::EMessageType::EActionRequest:
    case Message::EMessageType::EEventLog:
    case Message::EMessageType::EAccountFund:
    case Message::EMessageType::EAccountPosition:
    case Message::EMessageType::EOrderStatus:
    case Message::EMessageType::ERiskReport:
    case Message::EMessageType::EAppStatus:
    case Message::EMessageType::EFutureMarketData:
    case Message::EMessageType::EStockMarketData:
    default:
        char buffer[128] = {0};
        sprintf(buffer, "UnKown Message type:0X%X", msg.MessageType);
        Utils::gLogger->Log->info("WatcherEngine::HandlePackMessage {}", buffer);
        break;
    }
}

bool WatcherEngine::IsTrading()const
{
    return m_Trading;
}

void WatcherEngine::CheckTrading()
{
    std::string buffer = Utils::getCurrentTimeMs() + 11;
    m_CurrentTimeStamp = Utils::getTimeStampMs(buffer.c_str());
    m_Trading  = (m_CurrentTimeStamp >= m_OpenTime && m_CurrentTimeStamp <= m_CloseTime);
}