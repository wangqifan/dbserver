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
#include "reporter.h"

const off_t kRollSize = 2048*1000;

AsyncLogging* g_asynclog = NULL;

void asyncOutput(const char* logline, int len){
  g_asynclog->append(logline, len);
}

void AsyncFlush()
{
  g_asynclog->stop();
}

#include "Acceptor.hh"
#include "SocketHelp.hh"
#include "InetAddress.hh"
#include "highdb.h"

#include <sstream>
#include <vector>

highdb highdb_("data/");


void split(const std::string& s,
    std::vector<std::string>& sv,
                   const char delim = ' ') {
    sv.clear();
    std::istringstream iss(s);
    std::string temp;

    while (std::getline(iss, temp, delim)) {
        sv.emplace_back(std::move(temp));
    }
    return;
}

void onConnection(const TcpConnectionPtr& conn)
{
  printf("onConnection\n");
  //conn->send("123456789");
}

void onMessage(const TcpConnectionPtr& conn, Buffer* interBuffer, ssize_t len)
{
  printf("onMessage : received %d Bytes from connection [%s]\n", interBuffer->readableBytes(), conn->name());
  std::string request = interBuffer->retrieveAsString(len - 2);
  interBuffer -> retrieve(2);
    std::vector<std::string> terms;
    split(request, terms);
    std::cout << "size: " << terms.size() << std::endl;
    if(terms.size() <= 1) {
      conn->send("bad request\n");
      return;
    } 
    std::cout << terms[0] << std::endl;
    if(terms[0] == "get") {
      std::string res = highdb_.get(terms[1]);
      std::cout << "get " <<  terms[1] << res << std::endl;
      conn->send(res);
      return; 
    } else if(terms[0] == "put") {
       highdb_.add(terms[1], std::move(terms[2]));
       conn->send("put success \n");
       return;
    }
  conn -> send("hello I am a server 8080\r\n");
}


void newConnetion(int sockfd, const InetAddress& peeraddr)
{
  LOG_DEBUG << "newConnetion() : accepted a new connection from";
  ::write(sockfd, "How are you?\n", 13);
}

int main()
{
  AsyncLogging log("/dev/stdout", kRollSize, 0.1);
  g_asynclog = &log;
  Logger::setOutput(asyncOutput);
  Logger::setFlush(AsyncFlush);
  g_asynclog->start();


  reporter report;
  report.start();

  InetAddress listenAddr(8080);
  EventLoop loop;
  TcpServer Tserver(&loop, listenAddr, "TcpServer");
  Tserver.setConnectionCallBack(onConnection);
  Tserver.setMessageCallBack(onMessage);
  Tserver.start();

  loop.loop();

}