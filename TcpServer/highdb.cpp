#include<vector>
#include "highdb.hh"
#include "json.hpp"
#include "decoder.hh"

// for convenience
using json = nlohmann::json;

highdb::highdb(std::string filepack_, dbserver *server):
   dbserver_(server),
   filepack(filepack_),
   encoder_(new encoder(filepack_, dbserver_, this)),
   map_(new ConcurrentHashMap<std::string, site>(1024)) {

}

void highdb::add(const TcpConnectionPtr& conn, std::string &key, std::string &&value) {
    pb::Record record;
    record.set_key(key);
    record.set_value(std::move(value));
    record.set_type(1);
    site s;
    encoder_ -> write_record(conn, std::move(record), s);
  //  map_-> put(record.key(), std::move(s));
}

void highdb::get(const TcpConnectionPtr& conn, std::string &key) {
    site site_;
    site_.file = -1;
    map_ -> for_one(key, site_);
    if(site_.file == -1) {
        conn -> send ("key not found\n");
        return;
    }
    encoder_ -> get_record(conn, std::move(site_));
}
void highdb::close() {
    close_ = true;
    encoder_ -> close();
}
// 合并
void highdb::merge() {
   /* highdb mergedb("data2/", dbserver_);
    int len = map_ -> table_length();
    int current = encoder_.get_current();
    encoder coder("hit/", dbserver_);
    for(int i = 0; i< len; i++) {
        std::vector<std::string> keys = map_ -> getkeys(i);
        for(int j = 0; j < keys.size(); j++) {
           site s;
           map_ -> for_one(keys[j], s);
           if(s.file < current) {
              std::string value = this -> get(keys[j]);
              mergedb.add(keys[j], std::move(value));
              pb::Record record;
              record.set_key(keys[j]);
              record.set_value(std::move(s.to_string()));
              record.set_type(1);
              site tmps;
              coder.write_record(std::move(record), tmps);
           } 
        }
    }
    
    decoder decoder_("data2/", current);
    if(decoder_.hashnext()) {
        pb::Record record = decoder_.getnext();
        std::string key = record.key();
        std::string value = record.value();
        mergedb.add(key, std::move(value));
    } 

    this -> filepack = mergedb.filepack;
    this -> map_ = mergedb.map_;
    this -> encoder_.reset(mergedb.encoder_.get_current(), filepack) ;
    */
}