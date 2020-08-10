#include "dbserver.hh"



void split(const std::string& s,
    std::vector<std::string>& sv,
                   const char delim = ' ') {
    sv.clear();
    std::istringstream iss(s);
    std::string temp;

    while (std::getline(iss, temp, delim)) {
        sv.emplace_back(std::move(temp));
    }
    return;
}


dbserver::dbserver(std::string filepack, int port):
  highdb_(new highdb(filepack, this)),
  listenAddr(port),
  loop(),
  Tserver(new TcpServer(&loop, listenAddr, "TcpServer")) {
}


void dbserver::onMessage(const TcpConnectionPtr& conn, Buffer* interBuffer, ssize_t len) {
    //printf("onMessage : received %d Bytes from connection [%s]\n", interBuffer->readableBytes(), conn->name());
  std::string request = interBuffer->retrieveAsString(len - 2);
  LOG_TRACE << "message :" <<  request ;
  interBuffer -> retrieve(2);
    std::vector<std::string> terms;
    split(request, terms);
    if(terms.size() <= 1) {
      conn->send("bad request\n");
      return;
    } 
     // std::cout << terms[0] << std::endl;
    if(terms[0] == "get") {
      std::string res = highdb_ -> get(terms[1]);
      //LOG_DEBUG << "************************** " + res;
      // std::cout << "get " <<  terms[1] << res << std::endl;
      res += '\r\n';
      conn->send(res);
      return; 
    } else if(terms[0] == "put") {
       highdb_ -> add(conn, terms[1], std::move(terms[2]));
   //    conn->send("put success \n");
       return;
    }
}



TcpConnectionPtr dbserver::getconn(int socketfd) {
  return Tserver -> getconn(socketfd);
}


void dbserver::start() {
    Tserver->setConnectionCallBack(std::bind(&dbserver::onConnection, this, std::placeholders::_1));
    Tserver->setMessageCallBack(std::bind(&dbserver::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    Tserver->start();
    loop.loop();
}