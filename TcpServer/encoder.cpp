#include "encoder.hh"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "Logger.hh"
#include "highdb.hh"
#include <iostream>
#include <stdio.h>

encoder::encoder(std::string filepack_,  dbserver *server, highdb *high):
   mutex_(),
   dbserver_(server),
   ring(),
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


int encoder::getfd(int file) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(fds.find(file) != fds.end()) {
        return fds[file];
    }
    std::string filepath = filepack + std::to_string(file) + ".dat";
    std::cout << "file: " << filepath << std::endl;
    int fd = open(filepath.c_str(), O_RDONLY);
    std::cout << "fd: " << fd << std::endl; 
    fds[file] = fd;
    return  fd;
}


void encoder::fixevent() {
    for(;;) {
        
        struct io_uring_cqe *cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe);
        LOG_TRACE <<  cqe -> user_data;
        int fd = cqe -> user_data;
        if(iodata_map.find(fd) != iodata_map.end()) {
            std::shared_ptr<io_data>  data = iodata_map[fd];
             if(data->type == 0) { //write
                high_ -> getmap() ->put(data-> key, std::move(data->site));
                data->conns ->send("put success\n");
             } else {
            
               int len = data->iov.iov_len;
               char buffer[len];
               memcpy(buffer, data->iov.iov_base, len);
               pb::Record record;
               record.ParseFromArray(buffer, len);
               data->conns->send(record.value());
               data->conns -> send("\n");
              // close(data -> fd);
               if(cqe->res < 0) {
                    fprintf(stderr, "cqe failed: %s\n",
                        strerror(-cqe->res));
                    fprintf(stderr, "fd %d\n",
                            data->fd);
               }
             }

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


    std::shared_ptr<io_data> io_data_(new io_data());
    io_data_ -> conns = conn;
    io_data_ -> iov = iov;
    io_data_ -> type = 0;

    struct io_uring_sqe *sqe;
    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_writev(sqe, current_fd, &(io_data_ -> iov), 1, offset);
    sqe->user_data = conn->getfd();
    site_.file = current_file;
    site_.offset = offset;
    site_.size = str.size();
    io_data_ -> site = site_;
    io_data_ -> key = record.key();
    iodata_map[conn->getfd()] = io_data_;
    
    offset += iov.iov_len;
    io_uring_submit(&ring);
}

void encoder::get_record(const TcpConnectionPtr& conn, site &&site_) {

    int len = site_.size;
    struct io_uring_sqe *sqe;
    sqe = io_uring_get_sqe(&ring);

    std::shared_ptr<io_data> data(new io_data());
    data -> iov.iov_base  = new  char[len];
    data -> iov.iov_len = site_.size;
    data -> type = 1;
    int filed = getfd(site_.file);
    io_uring_prep_readv(sqe, filed, &data->iov, 1, site_.offset + sizeof(int));
    data->fd = filed;
    data->conns = conn;
    sqe -> user_data = conn->getfd();
    iodata_map[conn->getfd()] = data;
    io_uring_submit(&ring);

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