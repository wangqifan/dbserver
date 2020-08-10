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
#include "dbserver.hh"

const off_t kRollSize = 4096*1000;

AsyncLogging* g_asynclog = NULL;

void asyncOutput(const char* logline, int len){
  g_asynclog->append(logline, len);
}

void AsyncFlush()
{
  g_asynclog->stop();
}

int main()
{
  AsyncLogging log("log.txt", kRollSize, 0.1);
  g_asynclog = &log;
  Logger::setOutput(asyncOutput);
  Logger::setFlush(AsyncFlush);
  g_asynclog->start();
  dbserver server("data/", 8080);
  server.start();
}
