#include "encoder.hh"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "Logger.hh"
#include "highdb.hh"

encoder::encoder(std::string filepack_,  dbserver *server, highdb *high):
   mutex_(),
   dbserver_(server),
   ring(),
   key_map(),
   high_(high),
   m_thread(std::bind(&encoder::fixevent, this)),
   filepack(filepack_) {
    int ret = io_uring_queue_init(ENTRIES, &ring, 0);
    if (ret < 0) {
      printf("io_uring_queue_init: %s\n", strerror(-ret));
    } 
    DIR *dir = opendir(filepack.c_str());
    if(dir == NULL) {
        mkdir(filepack.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    }
    current_file = getFileCout();
    if(current_file == 0) current_file++;
    std::string filepath = filepack + std::to_string(current_file) + ".dat";
    current_fd = open(filepath.c_str(), O_WRONLY | O_CREAT);
    struct stat statbuf;
    stat(filepath.c_str(), &statbuf);
    offset = statbuf.st_size;
    m_thread.start();
}

void encoder::fixevent() {
    for(;;) {
        
        struct io_uring_cqe *cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe);
        LOG_TRACE <<  cqe -> user_data;
        int fd = cqe -> user_data;
        if(key_map.find(fd) != key_map.end()) {
            node n = key_map[fd];
            high_ -> getmap() -> put(n.key, std::move(n.site_));
            // key_map.erase(fd);
        }
        if(conns.find(fd) != conns.end()) {
            conns[fd] -> send("put success\n");
           // conns.erase(fd);
        }
        io_uring_cqe_seen(&ring, cqe);
    }
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

void encoder::write_record(const TcpConnectionPtr& conn, pb::Record &&record, site &site_) {

    std::string str = record.SerializeAsString();

    std::lock_guard<std::mutex> lock(mutex_);
    if(offset > maxsize) {
        current_file++;
        std::string filepath = filepack + std::to_string(current_file) + ".dat";
        current_fd = open(filepath.c_str(), O_WRONLY | O_CREAT);
        offset = 0;
    }

    int len = str.size();
  
    char data[sizeof(len) + str.size()] = {0};
    memcpy(data, &len, sizeof(len));
    memcpy(data + sizeof(len) , str.c_str(), str.size());
   
    struct iovec iov = {
     .iov_base = data,
     .iov_len = sizeof(len) + str.size(),
    };
    struct io_uring_sqe *sqe;
    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_writev(sqe, current_fd, &iov, 1, offset);
    sqe->user_data = conn->getfd();
    site_.file = current_file;
    site_.offset = offset;
    node n;
    n.key = record.key();
    n.site_ = site_;
    key_map[conn->getfd()] = n;
    conns[conn->getfd()] = conn;
    
    offset += iov.iov_len;
    io_uring_submit(&ring);
}

pb::Record encoder::get_record(site &&site_) {
    std::string filepath = filepack + std::to_string(site_.file) + ".dat";
    std::ifstream inFile(filepath, std::ios::in | std::ios::binary);
    inFile.seekg(site_.offset);
    char tmp[sizeof(int)];
    inFile.read(tmp, sizeof(int));
    int len = 0;
    memcpy((char*)&len, &tmp, sizeof(len));
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
   // outfile.close();
}

void encoder::reset(int current, std::string package) {
    current_file = current;
    filepack = package;
}