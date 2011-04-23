#ifndef DEF_LOCAL_FRONTEND_H
#define DEF_LOCAL_FRONTEND_H

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>

#include "config.h"

using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;

class ClipboardManager;

class LocalManager: public Poco::Runnable
{
public:
    LocalManager(Config *conf, Poco::ThreadPool *pool,
                 ClipboardManager *manager);
    void start();
    void run();

private:
    Config *conf;
    ServerSocket ssock;
    Poco::ThreadPool *pool;
    ClipboardManager *manager;
    bool verbose;
};

class LocalHandler: public Poco::Runnable
{
public:
    LocalHandler(Config *conf, ClipboardManager *manager, StreamSocket &sock);
    void run();
    void sendClipboard();
    void sendClipboard(std::string clipboard);

private:
    void sendMsg(std::string msg);
    void close();
    void treatMsg(std::string msg);
    void treatData(int length, std::string data);

    Config *conf;
    ClipboardManager *manager;
    StreamSocket sock;
    bool isRunning;
    bool verbose;
};

#endif
