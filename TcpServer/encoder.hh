#ifndef ENCODER
#define ENCODER


#include <string>
#include <mutex>
#include <liburing.h>
#include <sys/stat.h>
#include <atomic>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "dbserver.hh"
#include <unistd.h>
#include <iostream>
#include "record.pb.h"
#include "site.h"
#include "Thread.hh"

#define ENTRIES   128


class highdb;
class dbserver;

class io_data {

public:
    int fd = -1;
    int type;
    std::string key;
    struct iovec iov;
    struct site site;
    TcpConnectionPtr conns;
};

class encoder
{
private:
    int current_file;
    std::string filepack = "data/";
    int current_fd;
    const int long maxsize = 1024 * 1024 * 10;
    std::atomic_int offset;
    std::mutex mutex_;
    struct io_uring ring;
    Thread m_thread;
    std::unordered_map<int, std::shared_ptr<io_data>> iodata_map;
    std::unordered_map<int, int> fds;
    dbserver *dbserver_;
    highdb *high_;
    int getfd(int file);
public:
    encoder(std::string filepack_, dbserver *server, highdb *high);
    int getFileCout();
    void reset(int current, std::string package);
    void write_record(const TcpConnectionPtr& conn, pb::Record &&record, site &site_);
    void get_record(const TcpConnectionPtr& conn, site &&site_);
    int get_current();
    void close();
    void fixevent();
};


#endif