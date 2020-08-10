#ifndef DECODER
#define DECODER


#include "ConcurrentHashMap.hh"
#include "record.pb.h"
#include "site.h"
#include <memory>
#include <string>
#include <fstream>

class decoder
{
private:
    std::string filepack;
    std::ifstream inFile;
    int current_;
    int nextlen;

public:
    decoder(std::string pack, int current);
    pb::Record getnext();
    bool hashnext();
};

#endif