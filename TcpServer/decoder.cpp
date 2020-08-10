#include "decoder.hh"
#include "record.pb.h"
#include <dirent.h>

decoder::decoder(std::string pack, int current):
   inFile(pack + std::to_string(current) + ".dat", std::ios::in | std::ios::binary) {
   filepack = pack;
   current_ = current;
}

pb::Record decoder::getnext() {

    char buffer[nextlen];
    inFile.read(buffer, sizeof(char) * nextlen);
    pb::Record record;
    record.ParseFromArray(buffer, nextlen);
    return record;
}

bool decoder::hashnext() {
    int len;
    if(!inFile.read((char*)&len, sizeof(int))) {
        current_++;
        inFile.open(filepack + std::to_string(current_) + ".dat", std::ios::in | std::ios::binary);
        if(!inFile.read((char*)&len, sizeof(int))){
            return false;
        }
    }
    nextlen = len;
    return true;
}