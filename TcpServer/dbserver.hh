#ifndef DB_SERVER
#define DB_SERVER

#include <errno.h>
#include <thread>
#include <strings.h>
#include <poll.h>
#include "EventLoop.hh"
#include "Channel.hh"
#include "Poller.hh"
#include "Logger.hh"
#include "AsyncLogging.hh"
#include "TcpServer.hh"

#include "Acceptor.hh"
#include "SocketHelp.hh"
#include "InetAddress.hh"
#include "highdb.hh"

#include <sstream>
#include <vector>

class highdb;

class dbserver {
    private:
    InetAddress listenAddr;
    EventLoop loop;
    highdb *highdb_;
    TcpServer *Tserver;
    void onConnection(const TcpConnectionPtr& conn){}
    void onMessage(const TcpConnectionPtr& conn, Buffer* interBuffer, ssize_t len);
    public:
    dbserver(std::string filepack, int port);
    void start();
    TcpConnectionPtr getconn(int socketfd);
    
};


#endif