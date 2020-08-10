#ifndef ENCODER
#define ENCODER


#include <string>
#include <mutex>
#include <liburing.h>
#include <sys/stat.h>
#include <atomic>
#include <fstream>
#include <map>
#include "dbserver.hh"

#include "record.pb.h"
#include "site.h"
#include "Thread.hh"

#define ENTRIES   128

struct node {
    std::string key;
    site site_;
};

class highdb;
class dbserver;

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
    std::map<int, node>  key_map;
    std::map<int, TcpConnectionPtr> conns;
    dbserver *dbserver_;
    highdb *high_;
public:
    encoder(std::string filepack_, dbserver *server, highdb *high);
    int getFileCout();
    void reset(int current, std::string package);
    void write_record(const TcpConnectionPtr& conn, pb::Record &&record, site &site_);
    pb::Record get_record(site &&site_);
    int get_current();
    void close();
    void fixevent();
};


#endif