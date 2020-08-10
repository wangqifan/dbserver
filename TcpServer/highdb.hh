#ifndef HIGH_DB
#define HIGH_DB


#include "encoder.hh"
#include "record.pb.h"
#include "ConcurrentHashMap.hh"
#include "site.h"
#include "dbserver.hh"
#include <string>

class encoder;
class dbserver;

class highdb
{
private:
    encoder *encoder_;
    std::shared_ptr<ConcurrentHashMap<std::string, site>> map_;
    std::string filepack;
    bool close_;
    dbserver *dbserver_;

public:
    highdb(std::string filepack_,dbserver* server);
    void add(const TcpConnectionPtr& conn, std::string &key, std::string &&value);
    std::string get(std::string &key);
    void merge();
    void close();
     std::shared_ptr<ConcurrentHashMap<std::string, site>> getmap() {
         return map_;
     }
};

#endif