#include<etcd/Client.hpp>
#include<string>
#include<unistd.h>
#include<iostream>

#include "Thread.hh"


class reporter {
    private:
          Thread m_thread;
   public: 
    reporter():
        m_thread(std::bind(&reporter::report, this)) {
    }
    void report() {
          etcd::Client etcd("http://192.168.134.128:2379");
          while(true){
               etcd.set("/node/127.0.0.1:8080","12").get();
               std::cout << "report onec" << std::endl;
               sleep(1000);
            }
      }
      void start() {
          m_thread.start();
      } 
      
};