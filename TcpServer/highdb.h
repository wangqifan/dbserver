#ifndef HIGH_DB
#define HIGH_DB


#include "encoder.h"
#include "record.pb.h"
#include "ConcurrentHashMap.h"
#include "site.h"
#include <string>

class highdb
{
private:
    encoder encoder_;
    std::shared_ptr<ConcurrentHashMap<std::string, site>> map_;
    std::string filepack;
    bool close_;
public:
    highdb(std::string filepack_);
    void add(std::string &key, std::string &&value);
    std::string get(std::string &key);
    void merge();
    void close();
};

#endif