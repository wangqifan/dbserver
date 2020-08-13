#ifndef SITE
#define SITE

#include "json.hpp"
#include <string>
#include <iostream>

using json = nlohmann::json;

struct site
{
    int size;
    int file;
    int offset;
    std::string to_string() {
        json j;
        j.push_back(size);
        j.push_back(file);
        j.push_back(offset);
        return j.dump();
    }
};


#endif