#include "encoder.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

encoder::encoder(std::string filepack_):
   mutex_(),
   filepack(filepack_) {
    DIR *dir = opendir(filepack.c_str());
    if(dir == NULL) {
        mkdir(filepack.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    }
    current_file = getFileCout();
    if(current_file == 0) current_file++;
    outfile.open(filepack + std::to_string(current_file) + ".dat", std::ios::out | std::ios::binary | std::ios::app);
}

int encoder::getFileCout() {
    int result = 0;
    DIR *dir = opendir(filepack.c_str());
    struct dirent *ptr;
    while((ptr=readdir(dir))!=NULL)
    {
        //跳过'.'和'..'两个目录
        if(ptr->d_name[0] == '.')
            continue;

        result++;
    }
    closedir(dir);
    return result;
}

void encoder::write_record(pb::Record &&record, site &site_) {
    std::lock_guard<std::mutex> lock(mutex_);
    int pos = outfile.tellp();
    if(pos > maxsize) {
        // 创建新文件
        outfile.close();
        current_file++;
        outfile.open(filepack + std::to_string(current_file) + ".dat", std::ios::out | std::ios::binary | std::ios::app);
        pos = outfile.tellp();
    }
    std::string str = record.SerializeAsString();
    int len = str.size();
    outfile.write((char*)&len, sizeof(int));
    record.SerializePartialToOstream(&outfile);
    outfile << std::endl;
    site_.file = current_file;
    site_.offset = pos;
}

pb::Record encoder::get_record(site &&site_) {
    std::string filepath = filepack + std::to_string(site_.file) + ".dat";
    std::cout << filepath << " " << site_.file << site_.offset << std::endl;
    std::ifstream inFile(filepath, std::ios::in | std::ios::binary);
    inFile.seekg(site_.offset);
    int len;
    inFile.read((char*)&len, sizeof(int));
    char buffer[len];
    inFile.read(buffer, sizeof(char) * len);
    pb::Record record;
    record.ParseFromArray(buffer, len);
    return record;
}
int encoder::get_current() {
    return current_file;
}

void encoder::close() {
    outfile.close();
}

void encoder::reset(int current, std::string package) {
    current_file = current;
    filepack = package;
}